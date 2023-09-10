#pragma once

#include <string_view>
#include <bitset>
#include <assert.h>

#include "../Enum/Enum.hpp"

namespace CuImg
{
    enum ImageType : uint64_t
    {
        Unknown = 0,
        BMP = 1ULL << 0,
        JPEG = 1ULL << 1,
        PNG = 1ULL << 2,
        TIFF = 1ULL << 3,
        GIF = 1ULL << 4,
        ICO = 1ULL << 5,
        HDR = 1ULL << 6,
        TGA = 1ULL << 7,
        JPEG2000 = 1ULL << 8,
        JPEG_XR = 1ULL << 9,
        WMP = 1ULL << 10,
        DDS = 1ULL << 11,
        HEIF = 1ULL << 12,
        DNG = 1ULL << 13,
        PSD = 1ULL << 14,
        PIC = 1ULL << 15,
        PNM = 1ULL << 16,
        WEBP = 1ULL << 17,
        AVIF = 1ULL << 18,
        XBM = 1ULL << 19,
        XPM = 1ULL << 20,
        PCX = 1ULL << 21,
        ANI = 1ULL << 22,
        IFF = 1ULL << 23,
        RAW = 1ULL << 24,
        ImageTypeMax
    };

    namespace Detail
    {
        static auto ImageExtensions = []()
        {
            std::map<std::string, uint64_t, std::less<>> ret{};
#define MakeExt(ext, type)            \
    {                                 \
        constexpr auto es = "." #ext; \
        assert(!ret.contains(es));    \
        ret[es] = ImageType::type;    \
    }

            MakeExt(bmp, BMP);
            MakeExt(dib, BMP);

            MakeExt(jpg, JPEG);
            MakeExt(jpeg, JPEG);
            MakeExt(jpe, JPEG);
            MakeExt(jif, JPEG);
            MakeExt(jfif, JPEG);
            MakeExt(jfi, JPEG);

            MakeExt(png, PNG);

            MakeExt(tif, TIFF | RAW);
            MakeExt(tiff, TIFF);

            MakeExt(gif, GIF);

            MakeExt(ico, ICO);
            MakeExt(cur, ICO);

            MakeExt(hdr, HDR);

            MakeExt(tga, TGA);
            MakeExt(icb, TGA);
            MakeExt(vda, TGA);
            MakeExt(vst, TGA);

            MakeExt(jp2, JPEG2000);
            MakeExt(j2k, JPEG2000);
            MakeExt(jpf, JPEG2000);
            MakeExt(jpm, JPEG2000);
            MakeExt(jpg2, JPEG2000);
            MakeExt(j2c, JPEG2000);
            MakeExt(jpc, JPEG2000);
            MakeExt(jpx, JPEG2000);
            MakeExt(mj2, JPEG2000);

            MakeExt(jxr, JPEG_XR);

            MakeExt(hdp, WMP);
            MakeExt(wdp, WMP);

            MakeExt(dds, DDS);

            MakeExt(heif, HEIF);
            MakeExt(heifs, HEIF);

            // for hevc
            // MakeExt(heic, HEIF);
            // MakeExt(heics, HEIF);

            // for avc
            // MakeExt(avci, HEIF | AVIF);
            // MakeExt(avcs, HEIF | AVIF);

            MakeExt(dng, DNG | RAW);

            MakeExt(psd, PSD);
            MakeExt(psb, PSD);

            MakeExt(pic, PIC);
            MakeExt(pict, PIC);
            MakeExt(pct, PIC);

            MakeExt(ppm, PNM);
            MakeExt(pgm, PNM);
            MakeExt(pbm, PNM);

            MakeExt(webp, WEBP);

            MakeExt(avif, AVIF);

            MakeExt(xbm, XBM);

            MakeExt(xpm, XPM);

            MakeExt(pcx, PCX);

            MakeExt(ani, ANI);

            MakeExt(iff, IFF);

            MakeExt(3fr, RAW);
            MakeExt(ari, RAW);
            MakeExt(arw, RAW);
            MakeExt(bay, RAW);
            MakeExt(braw, RAW);
            MakeExt(crw, RAW);
            MakeExt(cr2, RAW);
            MakeExt(cr3, RAW);
            MakeExt(cap, RAW);
            MakeExt(data, RAW);
            MakeExt(dcs, RAW);
            MakeExt(dcr, RAW);
            MakeExt(drf, RAW);
            MakeExt(eip, RAW);
            MakeExt(erf, RAW);
            MakeExt(fff, RAW);
            MakeExt(gpr, RAW);
            MakeExt(iiq, RAW);
            MakeExt(k25, RAW);
            MakeExt(kdc, RAW);
            MakeExt(mdc, RAW);
            MakeExt(mef, RAW);
            MakeExt(mos, RAW);
            MakeExt(mrw, RAW);
            MakeExt(nef, RAW);
            MakeExt(nrw, RAW);
            MakeExt(obm, RAW);
            MakeExt(orf, RAW);
            MakeExt(pef, RAW);
            MakeExt(ptx, RAW);
            MakeExt(pxn, RAW);
            MakeExt(r3d, RAW);
            MakeExt(raf, RAW);
            MakeExt(raw, RAW);
            MakeExt(rwl, RAW);
            MakeExt(rw2, RAW);
            MakeExt(rwz, RAW);
            MakeExt(sr2, RAW);
            MakeExt(srf, RAW);
            MakeExt(srw, RAW);
            MakeExt(x3f, RAW);

#undef MakeExt
            return ret;
        }();
    }

    inline const auto &GetImageExtensions()
    {
        return Detail::ImageExtensions;
    }

    inline uint64_t GuessType(const std::string_view &lowerExt)
    {
        const auto it = GetImageExtensions().find(lowerExt);
        const auto val = it != GetImageExtensions().end() ? it->second : Unknown;
        assert(val < ImageTypeMax);
        return val;
    }

    inline std::vector<ImageType> GuessTypes(const std::string_view &lowerExt)
    {
        std::vector<ImageType> res{};
        const auto ts = GuessType(lowerExt);
        if (ts == Unknown)
            return res;

        for (size_t i = 0; i < 64; ++i)
        {
            if (const auto b = ts & (1ULL << i); b)
            {
                res.push_back((ImageType)b);
            }
        }

        return res;
    }

    inline ImageType GuessAType(const std::string_view &lowerExt)
    {
        if (const auto ts = GuessType(lowerExt); ts != Unknown)
        {
            for (size_t i = 0; i < 64; ++i)
            {
                if (const auto b = ts & (1ULL << i); b)
                {
                    return (ImageType)b;
                }
            }
        }

        return Unknown;
    }

#define MakeTypeQ(type)                                    \
    inline bool Is##type(const std::string_view &lowerExt) \
    {                                                      \
        return GuessType(lowerExt) & ImageType::type;      \
    }

    MakeTypeQ(BMP);
    MakeTypeQ(JPEG);
    MakeTypeQ(PNG);
    MakeTypeQ(TIFF);
    MakeTypeQ(GIF);
    MakeTypeQ(ICO);
    MakeTypeQ(HDR);
    MakeTypeQ(TGA);
    MakeTypeQ(JPEG2000);
    MakeTypeQ(JPEG_XR);
    MakeTypeQ(WMP);
    MakeTypeQ(DDS);
    MakeTypeQ(HEIF);
    MakeTypeQ(DNG);
    MakeTypeQ(PSD);
    MakeTypeQ(PIC);
    MakeTypeQ(PNM);
    MakeTypeQ(WEBP);
    MakeTypeQ(AVIF);
    MakeTypeQ(XBM);
    MakeTypeQ(XPM);
    MakeTypeQ(PCX);
    MakeTypeQ(ANI);
    MakeTypeQ(IFF);
    MakeTypeQ(RAW);

#undef MakeTypeQ
};
