#pragma once

#include <cstdint>
#include <filesystem>
#include <utility>
#include <vector>
#include <array>
#include <unordered_set>
#include <variant>
#include <functional>
#include <type_traits>
#include <thread>
#include <span>

extern "C"
{
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

#include "../Image/Image.hpp"
#include "../Image/Convert.hpp"
#include "../String/String.hpp"
#include "../Convert/Convert.hpp"
#include "../Log/LogLevel.hpp"
#include "../Utility/Utility.hpp"

namespace CuVid
{
    CuExcept_MakeException(Exception, CuExcept, U8Exception);
    
#define Except(...) Exception(CuStr::FormatU8(__VA_ARGS__))
#define ThrowEx(...) throw Except(__VA_ARGS__)

    namespace Detail
    {
        template <typename>
        inline constexpr bool AlwaysFalse = false;

        template <typename T, size_t S>
        constexpr const char *GetFilename(const T (&str)[S], size_t i = S - 1)
        {
            for (; i > 0; --i)
                if (str[i] == '/')
                    return &str[i + 1];
            return str;
        }

        namespace AV
        {
            [[maybe_unused]] static bool Inited = []()
            {
                avdevice_register_all();
                return true;
            }();

            inline std::string AvStrError(const int err)
            {
                constexpr auto BufSize = 4096;
                char buf[BufSize]{0};
                if (const auto ret = av_strerror(err, buf, BufSize); ret < 0)
                    return CuStr::Format("error code {}", ret);
                return std::string(buf);
            }

#define FF_API_LESS_0(api, ...)                    \
    const auto ret = api(__VA_ARGS__);             \
    if (ret < 0)                                   \
        ThrowEx("[" #api "] {}", AvStrError(ret)); \
    return ret

#define FF_API_EQ_NULL(api, tip, behavior, ...)                \
    const auto ctx = api(__VA_ARGS__);                         \
    if (ctx == nullptr)                                        \
        ThrowEx("[" #api "] the " tip " cannot be " behavior); \
    return ctx

#define FF_API_EQ_NULL_ALLOC(api, tip, ...) FF_API_EQ_NULL(api, tip, "allocated", __VA_ARGS__)

            inline int AvformatOpenInput(AVFormatContext **ps, const char *url, const AVInputFormat *fmt, AVDictionary **options)
            {
                FF_API_LESS_0(avformat_open_input, ps, url, fmt, options);
            }

            inline const AVInputFormat *AvFindInputFormat(const char *short_name)
            {
                const auto *fmt = av_find_input_format(short_name);
                if (fmt == nullptr)
                    ThrowEx("[av_find_input_format] the format<", short_name, "> cannot be find");
                return fmt;
            }

            inline AVIOContext *AvioAllocContext(
                unsigned char *buffer,
                int buffer_size,
                int write_flag,
                void *opaque,
                int (*read_packet)(void *opaque, uint8_t *buf, int buf_size),
                int (*write_packet)(void *opaque, uint8_t *buf, int buf_size),
                int64_t (*seek)(void *opaque, int64_t offset, int whence))
            {
                FF_API_EQ_NULL_ALLOC(avio_alloc_context, "AVIO", buffer, buffer_size, write_flag, opaque, read_packet, write_packet, seek);
            }

            inline void *AvMalloc(size_t size)
            {
                FF_API_EQ_NULL_ALLOC(av_malloc, "buffer", size);
            }

            inline AVFormatContext *AvformatAllocContext()
            {
                FF_API_EQ_NULL_ALLOC(avformat_alloc_context, "AVFormat");
            }

            inline void AvformatCloseInput(AVFormatContext **s)
            {
                avformat_close_input(s);
            }

            inline int AvformatFindStreamInfo(AVFormatContext *ic, AVDictionary **options)
            {
                FF_API_LESS_0(avformat_find_stream_info, ic, options);
            }

            inline int AvFindBestStream(AVFormatContext *ic,
                                        enum AVMediaType type,
                                        int wanted_stream_nb,
                                        int related_stream,
                                        const AVCodec **decoder_ret,
                                        int flags)
            {
                const auto ret = av_find_best_stream(ic, type, wanted_stream_nb, related_stream, decoder_ret, flags);
                if (ret < 0)
                    ThrowEx("[av_find_best_stream] {}", AvStrError(ret));
                return ret;
            }

            inline AVCodecContext *AvcodecAllocContext3(const AVCodec *codec)
            {
                FF_API_EQ_NULL_ALLOC(avcodec_alloc_context3, "AVCodec", codec);
            }

            inline int AvcodecParametersToContext(AVCodecContext *codec,
                                                  const AVCodecParameters *par)
            {
                FF_API_LESS_0(avcodec_parameters_to_context, codec, par);
            }

            inline int AvcodecOpen2(AVCodecContext *avctx, const AVCodec *codec, AVDictionary **options)
            {
                FF_API_LESS_0(avcodec_open2, avctx, codec, options);
            }

            inline struct SwsContext *SwsGetCachedContext(struct SwsContext *context,
                                                          int srcW, int srcH, enum AVPixelFormat srcFormat,
                                                          int dstW, int dstH, enum AVPixelFormat dstFormat,
                                                          int flags, SwsFilter *srcFilter,
                                                          SwsFilter *dstFilter, const double *param)
            {
                FF_API_EQ_NULL_ALLOC(sws_getCachedContext, "SWS", context, srcW, srcH, srcFormat, dstW, dstH, dstFormat, flags, srcFilter, dstFilter, param);
            }

            inline AVFrame *AvFrameAlloc()
            {
                FF_API_EQ_NULL_ALLOC(av_frame_alloc, "AVFrame", );
            }

            inline int AvImageGetBufferSize(enum AVPixelFormat pix_fmt, int width, int height, int align)
            {
                FF_API_LESS_0(av_image_get_buffer_size, pix_fmt, width, height, align);
            }

            inline int AvImageFillArrays(uint8_t *dst_data[4], int dst_linesize[4],
                                         const uint8_t *src, enum AVPixelFormat pix_fmt, int width, int height, int align)
            {
                FF_API_LESS_0(av_image_fill_arrays, dst_data, dst_linesize, src, pix_fmt, width, height, align);
            }

            inline AVPacket *AvPacketAlloc()
            {
                FF_API_EQ_NULL_ALLOC(av_packet_alloc, "AVPacket");
            }

            inline int AvcodecSendPacket(AVCodecContext *avctx, const AVPacket *avpkt)
            {
                FF_API_LESS_0(avcodec_send_packet, avctx, avpkt);
            }

            inline int SwsGetColorspaceDetails(struct SwsContext *c, const int inv_table[4],
                                               int srcRange, const int table[4], int dstRange,
                                               int brightness, int contrast, int saturation)
            {
                FF_API_LESS_0(sws_setColorspaceDetails, c, inv_table, srcRange, table, dstRange, brightness, contrast, saturation);
            }

            inline int AvReadFrame(AVFormatContext *s, AVPacket *pkt)
            {
                FF_API_LESS_0(av_read_frame, s, pkt);
            }

            inline int AvformatAllocOutputContext2(AVFormatContext **ctx, const AVOutputFormat *oformat,
                                                   const char *format_name, const char *filename)
            {
                FF_API_LESS_0(avformat_alloc_output_context2, ctx, oformat, format_name, filename);
            }

            inline const AVCodec *AvcodecFindEncoder(enum AVCodecID id)
            {
                const auto ctx = avcodec_find_encoder(id);
                if (ctx == nullptr)
                    ThrowEx("[avcodec_find_encoder] the encoder<{}> cannot be find", avcodec_get_name(id));
                return ctx;
            }

            inline const AVCodec *AvcodecFindEncoderByName(const char *name)
            {
                const auto ctx = avcodec_find_encoder_by_name(name);
                if (ctx == nullptr)
                    ThrowEx("[avcodec_find_encoder_by_name] the encoder<{}> cannot be find", name);
                return ctx;
            }

            inline AVStream *AvformatNewStream(AVFormatContext *s, const AVCodec *c)
            {
                FF_API_EQ_NULL_ALLOC(avformat_new_stream, "Stream", s, c);
            }

            inline int AvChannelLayoutCopy(AVChannelLayout *dst, const AVChannelLayout *src)
            {
                FF_API_LESS_0(av_channel_layout_copy, dst, src);
            }

            inline int AvDictCopy(AVDictionary **dst, const AVDictionary *src, int flags)
            {
                FF_API_LESS_0(av_dict_copy, dst, src, flags);
            }

            inline int AvFrameGetBuffer(AVFrame *frame, int align)
            {
                FF_API_LESS_0(av_frame_get_buffer, frame, align);
            }

            inline int AvcodecParametersFromContext(AVCodecParameters *par,
                                                    const AVCodecContext *codec)
            {
                FF_API_LESS_0(avcodec_parameters_from_context, par, codec);
            }

            inline int AvioOpen(AVIOContext **s, const char *url, int flags)
            {
                FF_API_LESS_0(avio_open, s, url, flags);
            }

            inline int AvioOpen2(AVIOContext **s, const char *url, int flags, const AVIOInterruptCB *int_cb, AVDictionary **options)
            {
                FF_API_LESS_0(avio_open2, s, url, flags, int_cb, options);
            }

            inline int AvformatWriteHeader(AVFormatContext *s, AVDictionary **options)
            {
                FF_API_LESS_0(avformat_write_header, s, options);
            }

            inline int AvWriteTrailer(AVFormatContext *s)
            {
                FF_API_LESS_0(av_write_trailer, s);
            }

            inline int AvcodecSendFrame(AVCodecContext *avctx, const AVFrame *frame)
            {
                FF_API_LESS_0(avcodec_send_frame, avctx, frame);
            }

            inline int AvFrameMakeWritable(AVFrame *frame)
            {
                FF_API_LESS_0(av_frame_make_writable, frame);
            }

            inline int AvcodecReceivePacket(AVCodecContext *avctx, AVPacket *avpkt)
            {
                FF_API_LESS_0(avcodec_receive_packet, avctx, avpkt);
            }

            inline int AvInterleavedWriteFrame(AVFormatContext *s, AVPacket *pkt)
            {
                FF_API_LESS_0(av_interleaved_write_frame, s, pkt);
            }

            inline int AvDictSet(AVDictionary **pm, const char *key, const char *value, const int flags)
            {
                FF_API_LESS_0(av_dict_set, pm, key, value, flags);
            }

            inline int AvDictSetInt(AVDictionary **pm, const char *key, int64_t value, int flags)
            {
                FF_API_LESS_0(av_dict_set_int, pm, key, value, flags);
            }

            class MemoryStream
            {
            public:
                MemoryStream() = delete;
                MemoryStream(const uint8_t *data, const uint64_t size, const bool keep = false) : size(size), keep(keep)
                {
                    if (keep)
                    {
                        auto *newData = new uint8_t[size];
                        std::copy_n(data, size, newData);
                        this->data = newData;
                    }
                    else
                    {
                        this->data = data;
                    }
                }

                MemoryStream(const MemoryStream &ms)
                {
                    this->size = ms.size;
                    if (ms.keep)
                    {
                        auto *newData = new uint8_t[this->size];
                        std::copy_n(ms.data, ms.size, newData);
                        this->data = newData;
                    }
                    else
                    {
                        this->data = ms.data;
                    }
                    this->keep = ms.keep;
                    this->index = ms.index;
                }

                MemoryStream(MemoryStream &&ms) noexcept
                {
                    this->size = ms.size;
                    this->data = ms.data;
                    ms.data = nullptr;
                    this->keep = ms.keep;
                    this->index = ms.index;
                }

                MemoryStream &operator=(const MemoryStream &ms)
                {
                    if (this == &ms)
                        return *this;

                    this->size = ms.size;
                    if (ms.keep)
                    {
                        auto *newData = new uint8_t[this->size];
                        std::copy_n(ms.data, ms.size, newData);
                        this->data = newData;
                    }
                    else
                    {
                        this->data = ms.data;
                    }
                    this->keep = ms.keep;
                    this->index = ms.index;
                    return *this;
                }

                MemoryStream &operator=(MemoryStream &&ms) noexcept
                {
                    if (this == &ms)
                        return *this;

                    this->size = ms.size;
                    this->data = ms.data;
                    ms.data = nullptr;
                    this->keep = ms.keep;
                    this->index = ms.index;
                    return *this;
                }

                ~MemoryStream()
                {
                    if (keep)
                        delete[] data;
                }

                [[nodiscard]] uint64_t Size() const
                {
                    return size;
                }

                int Read(unsigned char *buf, const int bufSize)
                {
                    if (bufSize < 0)
                        return bufSize;
                    if (index >= size)
                        return AVERROR_EOF;

                    if (index + bufSize >= size)
                    {
                        const auto n = size - index;
                        std::copy_n(data + index, n, buf);
                        index += n;
                        return static_cast<int>(n);
                    }

                    std::copy_n(data + index, bufSize, buf);
                    index += bufSize;
                    return bufSize;
                }

                int64_t Seek(const int64_t offset, const int whence)
                {
                    if (whence == SEEK_SET)
                    {
                        if (offset < 0)
                            return -1;
                        index = offset;
                    }
                    else if (whence == SEEK_CUR)
                    {
                        index += offset;
                    }
                    else if (whence == SEEK_END)
                    {
                        if (offset > 0)
                            return -1;
                        index = size + offset;
                    }
                    else
                    {
                        ThrowEx("Seek fail");
                    }
                    return 0;
                }

            private:
                const uint8_t *data;
                uint64_t size;
                bool keep;
                uint64_t index = 0;
            };

            class MemoryStreamIoContext
            {
                static constexpr auto BufferSize = 4096;

            public:
                MemoryStreamIoContext(MemoryStreamIoContext const &) = delete;
                MemoryStreamIoContext &operator=(MemoryStreamIoContext const &) = delete;
                MemoryStreamIoContext(MemoryStreamIoContext &&) = delete;
                MemoryStreamIoContext &operator=(MemoryStreamIoContext &&) = delete;

                MemoryStreamIoContext(MemoryStream inputStream) : inputStream(std::move(inputStream)),
                                                                  buffer(static_cast<unsigned char *>(AvMalloc(BufferSize))),
                                                                  ctx(AvioAllocContext(buffer, BufferSize, 0, this,
                                                                                       &MemoryStreamIoContext::Read, nullptr, &MemoryStreamIoContext::Seek)) {}

                ~MemoryStreamIoContext()
                {
                    av_free(ctx);
                }

                void ResetInnerContext()
                {
                    ctx = nullptr;
                    buffer = nullptr;
                }

                static int Read(void *opaque, unsigned char *buf, const int bufSize)
                {
                    auto h = static_cast<MemoryStreamIoContext *>(opaque);
                    return h->inputStream.Read(buf, bufSize);
                }

                static int64_t Seek(void *opaque, const int64_t offset, const int whence)
                {
                    auto h = static_cast<MemoryStreamIoContext *>(opaque);

                    if (0x10000 == whence)
                        return h->inputStream.Size();

                    return h->inputStream.Seek(offset, whence);
                }

                [[nodiscard]] AVIOContext *GetAvio() const
                {
                    return ctx;
                }

            private:
                MemoryStream inputStream;
                unsigned char *buffer = nullptr;
                AVIOContext *ctx = nullptr;
            };
        }
    }

    using U8String = decltype(std::filesystem::path{}.u8string());
    using U8StringView = std::basic_string_view<U8String::value_type>;
    using CodecParamType = std::variant<AVCodecID, U8String>;

    namespace Detail
    {
        template <typename T>
        struct CuImgTypeToFFmpegType;

        template <>
        struct CuImgTypeToFFmpegType<CuImg::CuRGB>
        {
            constexpr AVPixelFormat operator()() { return AV_PIX_FMT_RGB24; }
        };

        template <>
        struct CuImgTypeToFFmpegType<CuImg::CuRGBA>
        {
            constexpr AVPixelFormat operator()() { return AV_PIX_FMT_RGBA; }
        };

        template <>
        struct CuImgTypeToFFmpegType<CuImg::CuBGR>
        {
            constexpr AVPixelFormat operator()() { return AV_PIX_FMT_BGR24; }
        };

        inline const AVCodec *GetCodec(const CodecParamType &param)
        {
            return std::visit(CuUtil::Variant::Visitor{[](const AVCodecID id)
                                         { return AV::AvcodecFindEncoder(id); },
                                         [](const U8String &name)
                                         {
                                             return AV::AvcodecFindEncoderByName(reinterpret_cast<const char *>(name.c_str()));
                                         }},
                              param);
        }

        inline const char *GetCString(const U8String &str)
        {
            return reinterpret_cast<const char *>(str.c_str());
        }
    }

    struct FlagDictionary
    {
        AVDictionary *Dict = nullptr;

        FlagDictionary() = default;

        ~FlagDictionary()
        {
            av_dict_free(&Dict);
        }

        FlagDictionary(const FlagDictionary &dict)
        {
            CopyFrom(dict);
        }

        FlagDictionary(FlagDictionary &&dict) noexcept
        {
            MoveFrom(std::move(dict));
        }

        FlagDictionary &operator=(const FlagDictionary &dict)
        {
            CopyFrom(dict);
            return *this;
        }

        FlagDictionary &operator=(FlagDictionary &&dict) noexcept
        {
            MoveFrom(std::move(dict));
            return *this;
        }

        void CopyFrom(const AVDictionary *dict)
        {
            Clear();
            av_dict_copy(&Dict, dict, 0);
        }

        void CopyFrom(const FlagDictionary &dict)
        {
            CopyFrom(dict.Dict);
        }

        void MoveFrom(AVDictionary **dict)
        {
            Clear();
            Dict = *dict;
            dict = nullptr;
        }

        void MoveFrom(FlagDictionary &&dict)
        {
            MoveFrom(&dict.Dict);
        }

        FlagDictionary Clone()
        {
            return FlagDictionary(*this);
        }

        AVDictionary **AddressOf()
        {
            return &Dict;
        }

        void Clear()
        {
            if (Dict)
            {
                av_dict_free(&Dict);
                Dict = nullptr;
            }
        }

        FlagDictionary &Set(const U8StringView &key, const U8StringView &value)
        {
            Detail::AV::AvDictSet(&Dict, (const char *)key.data(), (const char *)value.data(), 0);
            return *this;
        }

        FlagDictionary &Set(const U8StringView &key, const int64_t value)
        {
            av_dict_set_int(&Dict, (const char *)key.data(), value, 0);
            return *this;
        }
    };

    enum StreamType : int
    {
        StreamTypeNone = 0,
        StreamTypeVideo = (1 << 0),
        StreamTypeAudio = (1 << 1),
        StreamTypeSubtitle = (1 << 2),
        StreamTypeData = (1 << 3),
    };

    template <typename Tc = CuImg::CuRGBA>
    class Decoder
    {
    private:
        using T = Tc;

    public:
        using VideoFrameType = CuImg::Image<T, CuImg::Backend::Ref>;
        using AudioFrameType = std::vector<uint8_t>;

    private:
        struct Context
        {
            AVFormatContext *fmtCtx = nullptr;
            AVPacket *pkt = nullptr;

            AVCodecContext *videoCtx = nullptr;
            AVCodecContext *audioCtx = nullptr;
            AVCodecContext *currentCodecCtx = nullptr;

            AVFrame *frame = nullptr;
            AVFrame *frameBuf = nullptr;

            SwsContext *swsCtx = nullptr;

            bool FileEof = false;
            bool Eof = false;

            Detail::AV::MemoryStream *ms;
            Detail::AV::MemoryStreamIoContext *privCtx;
        } Ctx{};

    public:
        struct DecoderConfig
        {
            bool UseMultiThread = true;

            int DecodeType = StreamTypeVideo;

            using FilePath = std::filesystem::path;
            using MemoryFile = std::span<const uint8_t>;
            std::variant<FilePath, MemoryFile> Input{};

            std::optional<U8String> FormatName{};
            FlagDictionary Options{};
            std::optional<std::variant<AVCodecID, U8String>> CodecIdOrName{};

            std::optional<int> VideoHeight{};
            std::optional<int> VideoWidth{};

            int VideoIndex = -1;
            int AudioIndex = -1;

            std::function<void(VideoFrameType &)> VideoHandler = nullptr;
            std::function<void(AudioFrameType)> AudioHandler = nullptr;
        } Config{};

        Decoder()
        {
        }

        ~Decoder()
        {
            Reset();
        }

        Decoder(const Decoder &) = delete;
        Decoder(Decoder &&dec) noexcept
        {
            Ctx = std::move(dec.Ctx);
            Config = std::move(dec.Config);
            dec.Ctx = {};
            dec.Config = {};
        }

        Decoder &operator=(const Decoder &) = delete;
        Decoder &operator=(Decoder &&dec) noexcept
        {
            Reset();
            Ctx = std::move(dec.Ctx);
            Config = std::move(dec.Config);
            dec.Ctx = {};
            dec.Config = {};
            return *this;
        }

        AVFormatContext *GetFormatContext() const { return Ctx.fmtCtx; }
        AVCodecContext *GetVideoCodecContext() const { return Ctx.videoCtx; }
        AVFrame *GetCurrentFrame() const { return Ctx.frame; }
        AVPacket *GetCurrentPacket() const { return Ctx.pkt; }

        void LoadFile()
        {
            using namespace Detail;

            Ctx.fmtCtx = AV::AvformatAllocContext();

            std::optional<std::u8string> url{};

            std::visit(CuUtil::Variant::Visitor{[&](const typename DecoderConfig::FilePath &path)
                                  {
                                      url = path.u8string();
                                  },
                                  [&](typename DecoderConfig::MemoryFile &file)
                                  {
                                      Ctx.ms = new AV::MemoryStream(file.data(), file.size_bytes(), false);
                                      Ctx.privCtx = new AV::MemoryStreamIoContext(*Ctx.ms);

                                      Ctx.fmtCtx->pb = Ctx.privCtx->GetAvio();
                                  }},
                       Config.Input);

            AV::AvformatOpenInput(
                &Ctx.fmtCtx,
                url ? reinterpret_cast<const char *>(url->c_str()) : nullptr,
                Config.FormatName ? AV::AvFindInputFormat(GetCString(*Config.FormatName)) : nullptr,
                Config.Options.Clone().AddressOf());
            AV::AvformatFindStreamInfo(Ctx.fmtCtx, nullptr);
        }

        void FindStream()
        {
            using namespace Detail;

            if (Config.DecodeType & StreamTypeVideo)
            {
                Config.VideoIndex = OpenCodecContext(&Ctx.videoCtx, Ctx.fmtCtx, AVMEDIA_TYPE_VIDEO, Config.VideoIndex);
            }

            if (Config.DecodeType & StreamTypeAudio)
            {
                Config.AudioIndex = OpenCodecContext(&Ctx.audioCtx, Ctx.fmtCtx, AVMEDIA_TYPE_AUDIO, Config.AudioIndex);
            }

            Ctx.pkt = AV::AvPacketAlloc();
            Ctx.frame = AV::AvFrameAlloc();
            Ctx.frameBuf = AV::AvFrameAlloc();
        }

        bool TrySendFrame()
        {
            using namespace Detail;

            int ret = av_read_frame(Ctx.fmtCtx, Ctx.pkt);
            if (ret == AVERROR_EOF)
            {
                Ctx.FileEof = true;
                // return false;
            }
            else if (ret < 0)
                ThrowEx("[av_read_frame] {}", AV::AvStrError(ret));

            Ctx.currentCodecCtx = nullptr;
            if (Ctx.pkt->stream_index == Config.VideoIndex)
                Ctx.currentCodecCtx = Ctx.videoCtx;
            else if (Ctx.pkt->stream_index == Config.AudioIndex)
                Ctx.currentCodecCtx = Ctx.audioCtx;

            if (Ctx.currentCodecCtx)
            {
                AV::AvcodecSendPacket(Ctx.currentCodecCtx, Ctx.FileEof ? nullptr : Ctx.pkt);
                return true;
            }

            av_packet_unref(Ctx.pkt);

            return false;
        }

        void SendFrame()
        {
            while (!TrySendFrame() && !Ctx.FileEof)
            {
            }
        }

        StreamType RecvFrame()
        {
            using namespace Detail;

            StreamType result = StreamTypeNone;

            if (Ctx.currentCodecCtx)
            {
                auto ret = avcodec_receive_frame(Ctx.currentCodecCtx, Ctx.frame);
                if (ret < 0)
                {
                    if (ret == AVERROR_EOF)
                    {
                        Ctx.Eof = true;
                        av_packet_unref(Ctx.pkt);
                        return StreamTypeNone;
                    }
                    if (ret == AVERROR(EAGAIN))
                    {
                        return StreamTypeNone;
                    }

                    ThrowEx("[avcodec_receive_frame] Error during decoding: {}", AV::AvStrError(ret));
                }

                if (Ctx.currentCodecCtx->codec->type == AVMEDIA_TYPE_VIDEO)
                {
                    constexpr auto DstPixFmt = Detail::CuImgTypeToFFmpegType<T>{}();

                    const auto w = Config.VideoWidth ? *Config.VideoWidth : Ctx.currentCodecCtx->width;
                    const auto h = Config.VideoHeight ? *Config.VideoHeight : Ctx.currentCodecCtx->height;
                  
                    VideoFrameType buf{};

                    Ctx.frameBuf->format = DstPixFmt;
                    Ctx.frameBuf->width = w;
                    Ctx.frameBuf->height = h;
                    Ctx.swsCtx = AV::SwsGetCachedContext(
                        Ctx.swsCtx, w, h, Ctx.currentCodecCtx->pix_fmt,
                        w, h, DstPixFmt, 0, nullptr, nullptr, nullptr);
                    const auto ret = sws_scale_frame(Ctx.swsCtx, Ctx.frameBuf, Ctx.frame);
                    if (ret < 0)
                        ThrowEx("[sws_scale_frame] {}", AV::AvStrError(ret));

                    buf.GetContext().SetSource(Ctx.frameBuf->data[0], Ctx.frameBuf->width, Ctx.frameBuf->height, Ctx.frameBuf->linesize[0]);

                    result = StreamTypeVideo;
                    if (Config.VideoHandler)
                        Config.VideoHandler(buf);
                }
                else if (Ctx.currentCodecCtx->codec->type == AVMEDIA_TYPE_AUDIO)
                {
                    result = StreamTypeAudio;
                    // Config.AudioHandler(Ctx.frame->data[0], Ctx.frame->linesize[0]);
                    // Ctx.Result = AudioFrameType{};
                }

                av_frame_unref(Ctx.frameBuf);
                av_frame_unref(Ctx.frame);
                av_packet_unref(Ctx.pkt);
            }
            else
            {
                if (Ctx.FileEof)
                    Ctx.Eof = true;
            }

            return result;
        }

        bool Eof()
        {
            return Ctx.Eof;
        }

        StreamType Read()
        {
            using namespace Detail;

            while (!Eof())
            {
                if (!Ctx.FileEof)
                    SendFrame();
                if (const auto t = RecvFrame(); t != StreamTypeNone)
                {
                    return t;
                }
            }

            return StreamTypeNone;
        }

        void Reset()
        {
            sws_freeContext(Ctx.swsCtx);

            av_frame_free(&Ctx.frameBuf);
            av_frame_free(&Ctx.frame);
            av_packet_free(&Ctx.pkt);

            avcodec_free_context(&Ctx.videoCtx);
            avcodec_free_context(&Ctx.audioCtx);
            avformat_close_input(&Ctx.fmtCtx);

            if (Ctx.ms)
                delete Ctx.ms;
            if (Ctx.privCtx)
                delete Ctx.privCtx;

            Ctx = {};
            Config = {};
        }

    private:
        int OpenCodecContext(AVCodecContext **dec_ctx, AVFormatContext *fmt_ctx, enum AVMediaType type, int idx = -1)
        {
            using namespace Detail::AV;

            if (idx < 0)
                idx = AvFindBestStream(fmt_ctx, type, -1, -1, nullptr, 0);

            auto *st = fmt_ctx->streams[idx];
            auto *dec = Config.CodecIdOrName ? Detail::GetCodec(*Config.CodecIdOrName) : avcodec_find_decoder(st->codecpar->codec_id);
            if (!dec)
                ThrowEx("[avcodec_find_decoder] Failed to find {} codec", av_get_media_type_string(type));

            *dec_ctx = AvcodecAllocContext3(dec);
            AvcodecParametersToContext(*dec_ctx, st->codecpar);
            if (type == AVMEDIA_TYPE_VIDEO && Config.UseMultiThread)
            {
                auto *ctx = *dec_ctx;
                ctx->thread_count = 0;
                if (dec->capabilities & AV_CODEC_CAP_FRAME_THREADS)
                {
                    ctx->thread_type = FF_THREAD_FRAME;
                }
                else if (dec->capabilities & AV_CODEC_CAP_SLICE_THREADS)
                {
                    ctx->thread_type = FF_THREAD_SLICE;
                }
            }

            AvcodecOpen2(*dec_ctx, dec, Config.Options.Clone().AddressOf());

            return idx;
        }
    };

    using DecoderRGB = Decoder<CuImg::CuRGB>;
    using DecoderRGBA = Decoder<CuImg::CuRGBA>;
    using DecoderBGR = Decoder<CuImg::CuBGR>;

    template <typename Tc = CuImg::CuRGBA>
    class Encoder
    {
        using T = Tc;
        using VideoFrameType = CuImg::Image<T, CuImg::Backend::ConstRef>;

        struct Stream
        {
            AVStream *st = nullptr;
            AVCodecContext *enc = nullptr;

            AVFrame *frame = nullptr;
            AVFrame *tmp_frame = nullptr;

            AVPacket *tmp_pkt = nullptr;

            // int64_t next_pts = 0;
        };

        struct VideoStream : Stream
        {
            SwsContext *sws_ctx = nullptr;
        };

        struct AudioStream : Stream
        {
            float t = 0;
            float tincr = 0;
            float tincr2 = 0;

            SwrContext *swr_ctx = nullptr;
        };

        struct Context
        {
            VideoStream video_st{};
            AudioStream audio_st{};

            AVFormatContext *Oc = nullptr;
            const AVCodec *audio_codec = nullptr;
            const AVCodec *video_codec = nullptr;
            bool have_video = 0;
            bool have_audio = 0;
            int encode_video = 0;
            int encode_audio = 0;
        } Ctx{};

    public:
        struct EncodeConfig
        {
#define ValidateCond(expr) \
    if (!(expr))           \
    ThrowEx("assert(" #expr ")")
            struct VideoConfig
            {
                int Width = 0;
                int Height = 0;
                AVRational TimeBase{0, 0};

                std::optional<std::variant<AVCodecID, U8String>> CodecIdOrName{};
                std::optional<AVPixelFormat> PixelFormat{};

                std::optional<std::function<void(AVFrame *)>> SetParametersHandler = nullptr;

                void Validate() const
                {
                    ValidateCond(Width > 0);
                    ValidateCond(Height > 0);
                    ValidateCond(TimeBase.num && TimeBase.den);
                }
            } Video;

            struct AudioConfig
            {
                AVCodecID CodecId = AV_CODEC_ID_NONE;

                void Validate() const
                {
                    ValidateCond(CodecId != AV_CODEC_ID_NONE);
                }
            } Audio;

            std::optional<U8String> FormatName{};
            U8String OutputPath;
            FlagDictionary Opt{};

            int OutputStream = StreamTypeVideo;

            [[nodiscard]] const char *GetOutputPath() const
            {
                return reinterpret_cast<const char *>(OutputPath.c_str());
            }

            [[nodiscard]] const char *GetFormatName() const
            {
                return reinterpret_cast<const char *>(FormatName.has_value() ? FormatName->c_str() : NULL);
            }
#undef ValidateCond
        } Config{};

        Encoder() = default;

        AVFormatContext *GetAVFormatContext() { return Ctx.Oc; }

        void Init()
        {
            using namespace Detail;

            const bool hasVideo = Config.OutputStream & StreamTypeVideo;
            const bool hasAudio = Config.OutputStream & StreamTypeAudio;

            if (hasVideo)
                Config.Video.Validate();
            if (hasAudio)
                Config.Audio.Validate();

            AV::AvformatAllocOutputContext2(&Ctx.Oc, nullptr, Config.GetFormatName(), Config.GetOutputPath());

            const auto *fmt = Ctx.Oc;

            if (hasVideo)
            {
                AddStream(&Ctx.video_st, Ctx.Oc, &Ctx.video_codec);
            }

            if (hasAudio)
            {
                AddStream(&Ctx.audio_st, Ctx.Oc, &Ctx.audio_codec);
            }

            if (hasVideo)
            {
                OpenVideo(Ctx.Oc, Ctx.video_codec, &Ctx.video_st, Config.Opt.Dict);
            }

            if (hasAudio)
            {
                OpenAudio(Ctx.Oc, Ctx.audio_codec, &Ctx.audio_st, Config.Opt.Dict);
            }

            if (!(fmt->flags & AVFMT_NOFILE))
            {
                AV::AvioOpen2(&Ctx.Oc->pb, Config.GetOutputPath(), AVIO_FLAG_READ_WRITE, nullptr, Config.Opt.Clone().AddressOf());
            }

            AV::AvformatWriteHeader(Ctx.Oc, Config.Opt.Clone().AddressOf());
        }

        void Write(const CuImg::Image<T, CuImg::Backend::ConstRef> &frame, const int64_t pts)
        {
            WriteVideoFrame(Ctx.Oc, &Ctx.video_st, frame, pts);
        }

        void Finish()
        {
            using namespace Detail;
            AV::AvWriteTrailer(Ctx.Oc);

            /* Close each codec. */
            if (Config.OutputStream & StreamTypeVideo)
                CloseStream(Ctx.Oc, &Ctx.video_st);
            if (Config.OutputStream & StreamTypeAudio)
                CloseStream(Ctx.Oc, &Ctx.audio_st);

            if (!(Ctx.Oc->oformat->flags & AVFMT_NOFILE))
                /* Close the output file. */
                avio_closep(&Ctx.Oc->pb);

            /* free the stream */
            avformat_free_context(Ctx.Oc);
        }

    private:
        const AVCodec *GetCodec() const
        {
            using namespace Detail;

            if (Config.Video.CodecIdOrName)
            {
                return std::visit(CuUtil::Variant::Visitor{[](const AVCodecID id)
                                             { return AV::AvcodecFindEncoder(id); },
                                             [](const U8String &name)
                                             {
                                                 return AV::AvcodecFindEncoderByName(reinterpret_cast<const char *>(name.c_str()));
                                             }},
                                  *Config.Video.CodecIdOrName);
            }

            return AV::AvcodecFindEncoder(Ctx.Oc->oformat->video_codec);
        }

        void AddStream(Stream *ost, AVFormatContext *oc, const AVCodec **codec)
        {
            using namespace Detail;

            *codec = GetCodec();
            ost->tmp_pkt = AV::AvPacketAlloc();
            ost->st = AV::AvformatNewStream(oc, nullptr);
            ost->st->id = oc->nb_streams - 1;

            auto *c = AV::AvcodecAllocContext3(*codec);
            ost->enc = c;

            switch ((*codec)->type)
            {
            case AVMEDIA_TYPE_AUDIO:
                c->sample_fmt = (*codec)->sample_fmts ? (*codec)->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
                c->bit_rate = 64000;
                c->sample_rate = 44100;
                if ((*codec)->supported_samplerates)
                {
                    c->sample_rate = (*codec)->supported_samplerates[0];
                    for (auto i = 0; (*codec)->supported_samplerates[i]; i++)
                    {
                        if ((*codec)->supported_samplerates[i] == 44100)
                            c->sample_rate = 44100;
                    }
                }
                {
                    constexpr AVChannelLayout chLayout AV_CHANNEL_LAYOUT_STEREO;
                    AV::AvChannelLayoutCopy(&c->ch_layout, &chLayout);
                }
                ost->st->time_base = AVRational{1, c->sample_rate};
                break;

            case AVMEDIA_TYPE_VIDEO:
                c->codec_id = (*codec)->id;
                c->width = Config.Video.Width;
                c->height = Config.Video.Height;
                /* timebase: This is the fundamental unit of time (in seconds) in terms
                 * of which frame timestamps are represented. For fixed-fps content,
                 * timebase should be 1/framerate and timestamp increments should be
                 * identical to 1. */
                ost->st->time_base = Config.Video.TimeBase;
                c->time_base = ost->st->time_base;
                c->pix_fmt = Config.Video.PixelFormat.value_or(*(*codec)->pix_fmts);
                ;
                break;

            default:
                break;
            }

            /* Some formats want stream headers to be separate. */
            if (oc->oformat->flags & AVFMT_GLOBALHEADER)
                c->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
        }

        static AVFrame *AllocFrame(enum AVPixelFormat pix_fmt, int width, int height)
        {
            using namespace Detail;

            auto *frame = AV::AvFrameAlloc();

            frame->format = pix_fmt;
            frame->width = width;
            frame->height = height;

            AV::AvFrameGetBuffer(frame, 0);

            return frame;
        }

        static void OpenVideo(AVFormatContext *oc, const AVCodec *codec, Stream *ost, AVDictionary *optArg)
        {
            using namespace Detail;

            auto *c = ost->enc;

            FlagDictionary opt{};
            opt.CopyFrom(optArg);

            AV::AvcodecOpen2(c, codec, opt.AddressOf());

            ost->frame = AllocFrame(c->pix_fmt, c->width, c->height);
            if (constexpr auto t = CuImgTypeToFFmpegType<T>{}(); t != c->pix_fmt)
            {
                ost->tmp_frame = AllocFrame(t, c->width, c->height);
            }

            AV::AvcodecParametersFromContext(ost->st->codecpar, c);
        }

        static void OpenAudio(AVFormatContext *oc, const AVCodec *codec, Stream *ost, AVDictionary *opt_arg)
        {
#if 0
            AVCodecContext* c;
            int nb_samples;
            int ret;
            AVDictionary* opt = NULL;

            c = ost->enc;

            /* open it */
            av_dict_copy(&opt, opt_arg, 0);
            ret = avcodec_open2(c, codec, &opt);
            av_dict_free(&opt);
            if (ret < 0) {
                fprintf(stderr, "Could not open audio codec: %s\n", av_err2str(ret));
                exit(1);
            }

            /* init signal generator */
            ost->t = 0;
            ost->tincr = 2 * M_PI * 110.0 / c->sample_rate;
            /* increment frequency by 110 Hz per second */
            ost->tincr2 = 2 * M_PI * 110.0 / c->sample_rate / c->sample_rate;

            if (c->codec->capabilities & AV_CODEC_CAP_VARIABLE_FRAME_SIZE)
                nb_samples = 10000;
            else
                nb_samples = c->frame_size;

            ost->frame = alloc_audio_frame(c->sample_fmt, &c->ch_layout,
                c->sample_rate, nb_samples);
            ost->tmp_frame = alloc_audio_frame(AV_SAMPLE_FMT_S16, &c->ch_layout,
                c->sample_rate, nb_samples);

            /* copy the stream parameters to the muxer */
            ret = avcodec_parameters_from_context(ost->st->codecpar, c);
            if (ret < 0) {
                fprintf(stderr, "Could not copy the stream parameters\n");
                exit(1);
            }

            /* create resampler context */
            ost->swr_ctx = swr_alloc();
            if (!ost->swr_ctx) {
                fprintf(stderr, "Could not allocate resampler context\n");
                exit(1);
            }

            /* set options */
            av_opt_set_chlayout(ost->swr_ctx, "in_chlayout", &c->ch_layout, 0);
            av_opt_set_int(ost->swr_ctx, "in_sample_rate", c->sample_rate, 0);
            av_opt_set_sample_fmt(ost->swr_ctx, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
            av_opt_set_chlayout(ost->swr_ctx, "out_chlayout", &c->ch_layout, 0);
            av_opt_set_int(ost->swr_ctx, "out_sample_rate", c->sample_rate, 0);
            av_opt_set_sample_fmt(ost->swr_ctx, "out_sample_fmt", c->sample_fmt, 0);

            /* initialize the resampling context */
            if ((ret = swr_init(ost->swr_ctx)) < 0) {
                fprintf(stderr, "Failed to initialize the resampling context\n");
                exit(1);
            }
#endif
        }

        /*
         * encode one video frame and send it to the muxer
         * return 1 when encoding is finished, 0 otherwise
         */
        int WriteVideoFrame(AVFormatContext *oc, VideoStream *ost, const VideoFrameType &frame, const int64_t pts)
        {
            return WriteFrame(oc, ost->enc, ost->st, GetVideoFrame(ost, frame, pts), ost->tmp_pkt);
        }

        static int WriteFrame(AVFormatContext *fmt_ctx, AVCodecContext *c,
                              AVStream *st, AVFrame *frame, AVPacket *pkt)
        {
            using namespace Detail;

            // send the frame to the encoder
            AV::AvcodecSendFrame(c, frame);

            int ret = 0;
            while (ret >= 0)
            {
                ret = avcodec_receive_packet(c, pkt);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                {
                    break;
                }
                else if (ret < 0)
                {
                    ThrowEx("[avcodec_receive_packet] {}", AV::AvStrError(ret));
                }

                /* rescale output packet timestamp values from codec to stream timebase */
                av_packet_rescale_ts(pkt, c->time_base, st->time_base);

                pkt->stream_index = st->index;

                /* Write the compressed frame to the media file. */
                AV::AvInterleavedWriteFrame(fmt_ctx, pkt);
                /* pkt is now blank (av_interleaved_write_frame() takes ownership of
                 * its contents and resets pkt), so that no unreferencing is necessary.
                 * This would be different if one used av_write_frame(). */
            }

            return ret == AVERROR_EOF ? 1 : 0;
        }

        static void FillImage(AVFrame *frame, const VideoFrameType &img)
        {
            auto ref = CuImg::ConvertToRef<T>(frame->data[0], frame->width, frame->height, frame->linesize[0]);
            CuImg::Convert(img, ref);
        }

        AVFrame *GetVideoFrame(VideoStream *ost, const VideoFrameType &img, const int64_t pts)
        {
            using namespace Detail;

            const auto *c = ost->enc;

            /* when we pass a frame to the encoder, it may keep a reference to it
             * internally; make sure we do not overwrite it here */

            AV::AvFrameMakeWritable(ost->frame);

            if (constexpr auto t = CuImgTypeToFFmpegType<T>{}(); c->pix_fmt != t)
            {
                ost->sws_ctx = AV::SwsGetCachedContext(ost->sws_ctx, c->width, c->height, t,
                                                       c->width, c->height, c->pix_fmt, SWS_BICUBIC, nullptr, nullptr, nullptr);

                FillImage(ost->tmp_frame, img);
                sws_scale(ost->sws_ctx, (const uint8_t *const *)ost->tmp_frame->data,
                          ost->tmp_frame->linesize, 0, c->height, ost->frame->data,
                          ost->frame->linesize);
            }
            else
            {
                FillImage(ost->frame, img);
            }

            ost->frame->pts = pts;

            return ost->frame;
        }

        template <typename T = Stream>
        static void CloseStream(AVFormatContext *oc, T *ost)
        {
            avcodec_free_context(&ost->enc);
            av_frame_free(&ost->frame);
            av_frame_free(&ost->tmp_frame);
            av_packet_free(&ost->tmp_pkt);

            if constexpr (std::is_same_v<T, VideoStream>)
            {
                sws_freeContext(ost->sws_ctx);
            }
            else if constexpr (std::is_same_v<T, AudioStream>)
            {
                swr_free(&ost->swr_ctx);
            }
        }
    };

    using EncoderRGB = Encoder<CuImg::CuRGB>;
    using EncoderRGBA = Encoder<CuImg::CuRGBA>;
    using EncoderBGR = Encoder<CuImg::CuBGR>;
};
