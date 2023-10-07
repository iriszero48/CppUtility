#pragma once

#ifdef CU_IMG_HAS_DIRECTXTEX

#include "IContext.hpp"
#include "Exception.hpp"

#include <string_view>
#include <chrono>
#include <variant>
#include <set>

#include <wincodec.h>
#include <DirectXTex.h>

#include "../Enum/Enum.hpp"
#include "../String/String.hpp"
#include "../Assert/Assert.hpp"

namespace CuImg
{
    enum class DirectXTexLang : DWORD { Default = 0, EN_US = 0x0409 };

	static struct 
	{
        DirectXTexLang LangId = DirectXTexLang::Default;
	} DirectXTexConfig;

    namespace Detail::DxTex
    {
#pragma region WicPhotoProp
        CuEnum_MakeEnumDef(WicPhotoProp,
                    Aperture,
                    ApertureDenominator,
                    ApertureNumerator,
                    Brightness,
                    BrightnessDenominator,
                    BrightnessNumerator,
                    CameraManufacturer,
                    CameraModel,
                    CameraSerialNumber,
                    Contrast,
                    ContrastText,
                    DateTaken,
                    DigitalZoom,
                    DigitalZoomDenominator,
                    DigitalZoomNumerator,
                    EXIFVersion,
                    Event,
                    ExposureBias,
                    ExposureBiasDenominator,
                    ExposureBiasNumerator,
                    ExposureIndex,
                    ExposureIndexDenominator,
                    ExposureIndexNumerator,
                    ExposureProgram,
                    ExposureProgramText,
                    ExposureTime,
                    ExposureTimeDenominator,
                    ExposureTimeNumerator,
                    FNumber,
                    FNumberDenominator,
                    FNumberNumerator,
                    Flash,
                    FlashEnergy,
                    FlashEnergyDenominator,
                    FlashEnergyNumerator,
                    FlashManufacturer,
                    FlashModel,
                    FlashText,
                    FocalLength,
                    FocalLengthDenominator,
                    FocalLengthInFilm,
                    FocalLengthNumerator,
                    FocalPlaneXResolution,
                    FocalPlaneXResolutionDenominator,
                    FocalPlaneXResolutionNumerator,
                    FocalPlaneYResolution,
                    FocalPlaneYResolutionDenominator,
                    FocalPlaneYResolutionNumerator,
                    GainControl,
                    GainControlDenominator,
                    GainControlNumerator,
                    GainControlText,
                    ISOSpeed,
                    LensManufacturer,
                    LensModel,
                    LightSource,
                    MakerNote,
                    MakerNoteOffset,
                    MaxAperture,
                    MaxApertureDenominator,
                    MaxApertureNumerator,
                    MeteringMode,
                    MeteringModeText,
                    Orientation,
                    OrientationText,
                    PeopleNames,
                    PhotometricInterpretation,
                    PhotometricInterpretationText,
                    ProgramMode,
                    ProgramModeText,
                    RelatedSoundFile,
                    Saturation,
                    SaturationText,
                    Sharpness,
                    SharpnessText,
                    ShutterSpeed,
                    ShutterSpeedDenominator,
                    ShutterSpeedNumerator,
                    SubjectDistance,
                    SubjectDistanceDenominator,
                    SubjectDistanceNumerator,
                    TagViewAggregate,
                    TranscodedForSync,
                    WhiteBalance,
                    WhiteBalanceText);
#pragma endregion WicPhotoProp

	    inline std::u8string ErrStr(const HRESULT hr)
	    {
		    std::wstring err = CuStr::CombineW("Error 0x", std::hex, static_cast<uint32_t>(hr), ": ");

		    LPTSTR errorText = nullptr;

		    FormatMessage(
			    FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS,
			    nullptr,
			    hr,
			    CuUtil::ToUnderlying(DirectXTexConfig.LangId),
			    reinterpret_cast<LPTSTR>(&errorText),
			    0,
			    nullptr);

		    if (nullptr != errorText)
		    {
			    err.append(errorText);

			    LocalFree(errorText);

			    return CuStr::ToU8String(err);
		    }

            return CuStr::FormatU8("{}unknown error(FormatMessage got an error: 0x{})", err,
                CuStr::Combine(std::hex, std::setw(8), std::setfill('0'), GetLastError()));
	    }

        template <size_t S>
        constexpr std::array<char8_t, S> GetFunctionName(const std::array<char, S>& str)
        {
            size_t pos = 0;
            for (; pos < S - 1; ++pos)
            {
                if (str[pos] == '(') break;
            }

            std::array<char8_t, S> buf{};
            for (size_t i = 0; i < pos; ++i)
            {
                buf[i] = static_cast<char8_t>(str[i]);
            }
            buf[pos] = 0;

            return buf;
        }

        namespace Api
        {
#define CuImg__DxTex_WIN_API(expr)              \
    if (const auto hr = expr; FAILED(hr))       \
    throw CuImg_DirectXTexException(std::u8string_view(GetFunctionName(CuUtil::String::ToBuffer(#expr)).data()), u8": ", ErrStr(hr))

            inline void CoInitializeEx(LPVOID pvReserved, DWORD dwCoInit)
            {
                CuImg__DxTex_WIN_API(::CoInitializeEx(pvReserved, dwCoInit));
            }
        }

        [[maybe_unused]] static bool Init = []()
        {
            Api::CoInitializeEx(nullptr, COINIT_MULTITHREADED);
            return true;
        }();
    }

    struct DirectXTexContext : IImageContext<DirectXTexContext, CuRGBA>
    {
        // PROPVARIANT Type
        using VtI1 = std::int8_t;                           // VT_I1
        using VtUi1 = std::uint8_t;                         // VT_UI1
        using VtI2 = std::int16_t;                          // VT_I2
        using VtUi2 = std::uint16_t;                        // VT_UI2
        using VtI4 = std::int32_t;                          // VT_I4
        using VtUi4 = std::uint32_t;                        // VT_UI4
        using VtI8 = std::int64_t;                          // VT_I8
        using VtUi8 = std::uint64_t;                        // VT_UI8
        using VtR4 = float;                                 // VT_R4
        using VtR8 = double;                                // VT_R8
        using VtUi1Array = std::vector<VtUi1>;              // VT_ARRAY | VT_UI1
        using VtWStr = std::wstring;                        // VT_BSTR, VT_LPWSTR
        using VtWStrArray = std::vector<VtWStr>;            // VT_ARRAY | VT_BSTR
        using VtBool = bool;                                // VT_BOOL
        using VtTime = std::chrono::file_clock::time_point; // VT_DATE, VT_FILETIME

        using PhotoProp = Detail::DxTex::WicPhotoProp;
        using PropType = std::variant<
            VtI1, VtUi1, VtI2, VtUi2, VtI4, VtUi4, VtI8, VtUi8, VtR4, VtR8, VtUi1Array, VtWStr, VtWStrArray, VtBool, VtTime>;

        struct LoadInfo
        {
            // [in]
            bool SkipLoadWicProps = false;

            // [out]
            DXGI_FORMAT SrcFmt = DXGI_FORMAT_R8G8B8A8_UNORM;
            // [out]
            std::unordered_map<PhotoProp, PropType> WicProps{};
        } Info{};

        struct SaveParam
        {
            // DDS
            DXGI_FORMAT DestFmt = DXGI_FORMAT_R8G8B8A8_UNORM;
            ID3D11Device *CompressDevice = nullptr;
            bool TexAlphaModePremultiplied = false;

            // WIC
            GUID *WicFmt = nullptr;

            struct WicJPEG
            {
                // 0-1.0
                float ImageQuality = 0.9f;
                WICBitmapTransformOptions BitmapTransform = WICBitmapTransformRotate0;
                // 64 Entries (DCT)
                std::optional<std::array<std::uint32_t, 64>> Luminance{};
                // 64 Entries (DCT)
                std::optional<std::array<std::uint32_t, 64>> Chrominance{};
                WICJpegYCrCbSubsamplingOption JpegYCrCbSubsampling = WICJpegYCrCbSubsampling420;
                bool SuppressApp0 = false;
            } JPEG{};

            struct WicJPEGXR
            {
                std::optional<std::uint8_t> AlphaDataDiscard{};
                // 1-255
                std::uint8_t AlphaQuality = 1;
                WICBitmapTransformOptions BitmapTransform = WICBitmapTransformRotate0;
                bool CompressedDomainTranscode = true;
                bool FrequencyOrder = true;
                // 0 - 4095, Default Value: (image width - 1) >> 8
                std::optional<std::uint16_t> HorizontalTileSlices{};
                bool IgnoreOverlap = false;
                // 0-3
                std::uint8_t ImageDataDiscard = 0;
                // 0-1.0
                float ImageQuality = 0.9f;
                bool InterleavedAlpha = false;
                bool Lossless = false;
                // 0-4
                std::uint8_t Overlap = 1;
                bool ProgressiveMode = false;
                // 1-255
                std::uint8_t Quality = 1;
                bool StreamOnly = false;
                // 0 - 3, Default Value: 3 if ImageQuality > 0.8; otherwise 1;
                std::optional<std::uint8_t> Subsampling{};
                bool UseCodecOptions = false;
                // 0 - 4095, Default Value: (image height - 1) >> 8
                std::optional<std::uint16_t> VerticalTileSlices{};
            } JPEGXR{};

            struct WicHDPhoto
            {
                // 0-1.0
                float ImageQuality = 0.9f;
                bool Lossless = false;
                WICBitmapTransformOptions BitmapTransform = WICBitmapTransformRotate0;
                bool UseCodecOptions = false;
                // 1-255
                std::uint8_t Quality = 10;
                // 0-2
                std::uint8_t Overlap = 1;
                // 0 - 3, Default Value: 3 if ImageQuality > 0.8; otherwise 1;
                std::optional<std::uint8_t> Subsampling{};
                // 0 - 4095, Default Value: (image width - 1) >> 8
                std::optional<std::uint16_t> HorizontalTileSlices{};
                // 0 - 4095, Default Value: (image height - 1) >> 8
                std::optional<std::uint16_t> VerticalTileSlices{};
                bool FrequencyOrder = true;
                bool InterleavedAlpha = false;
                // 1-255
                std::uint8_t AlphaQuality = 1;
                bool CompressedDomainTranscode = true;
                // 0-3
                std::uint8_t ImageDataDiscard = 0;
                std::optional<std::uint8_t> AlphaDataDiscard{};
                bool IgnoreOverlap = false;
            } HDPhoto{};

            struct WicTIFF
            {
                // 0 - 1.0
                float CompressionQuality = 0.0;
                WICTiffCompressionOption TiffCompressionMethod = WICTiffCompressionDontCare;
            } TIFF{};

            struct WicPNG
            {
                bool InterlaceOption = false;
                WICPngFilterOption FilterOption = WICPngFilterUnspecified;
            } PNG{};

            struct WicBMP
            {
                // On/Off
                bool EnableV5Header32bppBGRA = false;
            } BMP{};

            std::unordered_map<PhotoProp, PropType> WicProps{};
        } Param{};

        DirectX::ScratchImage Image{};

        void Create(const size_t width, const size_t height)
        {
            if (const auto hr = Image.Initialize2D(DXGI_FORMAT_R8G8B8A8_UNORM, width, height, 1, 1); FAILED(hr))
                throw CuImg_DirectXTexException(u8"[ScratchImage::Initialize2D] ", Detail::DxTex::ErrStr(hr));
        }

        [[nodiscard]] const uint8_t *Data() const { return GetImage()->pixels; }
        [[nodiscard]] uint8_t *Data() { return Image.GetPixels(); }
        [[nodiscard]] size_t Width() const { return GetImage() ? GetImage()->width : 0; }
        [[nodiscard]] size_t Height() const { return GetImage() ? GetImage()->height : 0; }
        [[nodiscard]] size_t Size() const { return Linesize() * Height(); }
        [[nodiscard]] size_t Count() const { return Width() * Height(); }
        [[nodiscard]] size_t Linesize() const { return GetImage() ? GetImage()->rowPitch : 0; }
        [[nodiscard]] size_t Empty() const { return Image.GetImageCount() == 0; }

        [[nodiscard]] const DirectX::ScratchImage &Raw() const { return Image; }
        [[nodiscard]] DirectX::ScratchImage &Raw() { return Image; }

        [[nodiscard]] const LoadInfo &GetInfo() const { return Info; }
        [[nodiscard]] LoadInfo &GetInfo() { return Info; }
        [[nodiscard]] const SaveParam &GetParam() const { return Param; }
        [[nodiscard]] SaveParam &GetParam() { return Param; }

    private:
        [[nodiscard]] const DirectX::Image *GetImage() const { return Image.GetImages(); }
    };
};

CuEnum_MakeEnumSpec(CuImg::Detail::DxTex, WicPhotoProp);
#endif