#pragma once

#include "Image.hpp"
#include "Exception.hpp"

#include "File_Utility.hpp"

#include "../String/String.hpp"

#ifdef CU_IMG_HAS_STB
#include "File_STB.hpp"
#endif

#ifdef CU_IMG_HAS_DIRECTXTEX
#include "File_DirectXTex.hpp"
#endif

#ifdef CU_IMG_HAS_QT
#include "File_QT.hpp"
#endif

#ifdef CU_IMG_HAS_WXWIDGETS
#include "File_wxWidgets.hpp"
#endif

#ifdef CU_IMG_HAS_OPENCV
#include "File_OPENCV.hpp"
#endif

#ifdef CU_IMG_HAS_GRAPHICSMAGICK
#include "File_GraphicsMagick.hpp"
#endif

namespace CuImg
{
    namespace Detail
    {
        template <typename, Backend>
        struct LoadFile
        {
        };

        template <typename, Backend>
        struct SaveFile
        {
        };

#define MakeImageLoadFileImpl(Color, Backend, Impl, Image)                                                  \
    template <>                                                                                             \
    struct LoadFile<Color, Backend>                                                                         \
    {                                                                                                       \
        void operator()(const std::filesystem::path &path, Image<Color, Backend> &img) { Impl(path, img); } \
    }

#define MakeImageSaveFileImpl(Color, Backend, Impl, Image)                                                        \
    template <>                                                                                                   \
    struct SaveFile<Color, Backend>                                                                               \
    {                                                                                                             \
        void operator()(const std::filesystem::path &path, const Image<Color, Backend> &img) { Impl(path, img); } \
    }

#define MakeImageLoadFile(Color, Backend, Impl) MakeImageLoadFileImpl(Color, Backend, Impl, Image)
#define MakeDiscreteImageLoadFile(Color, Backend, Impl) MakeImageLoadFileImpl(Color, Backend, Impl, DiscreteImage)

#define MakeImageSaveFile(Color, Backend, Impl) MakeImageSaveFileImpl(Color, Backend, Impl, Image)
#define MakeDiscreteImageSaveFile(Color, Backend, Impl) MakeImageSaveFileImpl(Color, Backend, Impl, DiscreteImage)

#ifdef CU_IMG_HAS_STB
        MakeImageLoadFile(CuRGB, Backend::STB, STB::StbLoadFileImpl<CuRGB>);
        MakeImageLoadFile(CuRGBA, Backend::STB, STB::StbLoadFileImpl<CuRGBA>);

        MakeImageSaveFile(CuRGB, Backend::STB, STB::StbSaveFileImpl<CuRGB>);
        MakeImageSaveFile(CuRGBA, Backend::STB, STB::StbSaveFileImpl<CuRGBA>);
#endif

#ifdef CU_IMG_HAS_DIRECTXTEX
        MakeImageLoadFile(CuRGBA, Backend::DirectXTex, DxTex::DirectXTexLoadFile);
        MakeImageSaveFile(CuRGBA, Backend::DirectXTex, DxTex::DirectXTexSaveFile);
#endif

#ifdef CU_IMG_HAS_QT
#define MakeQtLoadFile(c, f) MakeImageLoadFile(c, Backend::QT, (QT::QtLoadFile<c, f>))
#define MakeQtSaveFile(c) MakeImageSaveFile(c, Backend::QT, (QT::QtSaveFile<c>))

        MakeQtLoadFile(CuRGBA, QImage::Format::Format_RGBA8888);
        MakeQtLoadFile(CuRGB, QImage::Format::Format_RGB888);

        MakeQtSaveFile(CuRGBA);
        MakeQtSaveFile(CuRGB);

#undef _CuImgMakeQtLoadFile
#undef _CuImgMakeQtSaveFile
#endif

#ifdef CU_IMG_HAS_WXWIDGETS
        MakeDiscreteImageLoadFile(CuRGBA, Backend::wxWidgets, WxWidgets::WxImageLoadFileImpl);
        MakeDiscreteImageSaveFile(CuRGBA, Backend::wxWidgets, WxWidgets::WxImageSaveFileImpl);
#endif

#ifdef CU_IMG_HAS_OPENCV
        MakeImageLoadFile(CuRGB, Backend::OpenCV, OCV::OpenCvLoadFile<CuRGB>);
        MakeImageLoadFile(CuRGBA, Backend::OpenCV, OCV::OpenCvLoadFile<CuRGBA>);
        MakeImageSaveFile(CuRGB, Backend::OpenCV, OCV::OpenCvSaveFile<CuRGB>);
        MakeImageSaveFile(CuRGBA, Backend::OpenCV, OCV::OpenCvSaveFile<CuRGBA>);
#endif

#ifdef CU_IMG_HAS_GRAPHICSMAGICK
        MakeDiscreteImageLoadFile(CuRGBOpacity16, Backend::GraphicsMagick, GraphicsMagick::GraphicsMagickLoadFile);
        MakeDiscreteImageSaveFile(CuRGBOpacity16, Backend::GraphicsMagick, GraphicsMagick::GraphicsMagickSaveFile);
#endif

#undef MakeImageLoadFile
#undef MakeDiscreteImageLoadFile

#undef MakeImageSaveFile
#undef MakeDiscreteImageSaveFile

#undef MakeImageLoadFileImpl
#undef MakeImageSaveFileImpl
    }

    template <typename T = CuRGBA, Backend B>
    void LoadFile(const std::filesystem::path &path, Image<T, B> &img)
    {
        Detail::LoadFile<T, B>{}(path, img);
    }

    template <typename T = CuRGBA, Backend B>
    void LoadFile(const std::filesystem::path &path, DiscreteImage<T, B> &img)
    {
        Detail::LoadFile<T, B>{}(path, img);
    }

    template <typename T = CuRGBA, Backend B>
    decltype(auto) LoadFile_Image(const std::filesystem::path &path)
    {
        Image<T, B> ret{};
        LoadFile(path, ret);
        return ret;
    }

    template <typename T = CuRGBA, Backend B>
    decltype(auto) LoadFile_DiscreteImage(const std::filesystem::path &path)
    {
        DiscreteImage<T, B> ret{};
        LoadFile(path, ret);
        return ret;
    }

    template <typename T, Backend B>
    void SaveFile(const std::filesystem::path &path, const Image<T, B> &img)
    {
        Detail::SaveFile<T, B>{}(path, img);
    }

    template <typename T, Backend B>
    void SaveFile(const std::filesystem::path &path, const DiscreteImage<T, B> &img)
    {
        Detail::SaveFile<T, B>{}(path, img);
    }

#define MakeImageFileDefImpl(C, B, I)                                                                                                           \
    inline void LoadFile_##I##C##_##B(const std::filesystem::path &path, I<Cu##C, Backend::B> &img) { LoadFile<Cu##C, Backend::B>(path, img); } \
    inline I<Cu##C, Backend::B> LoadFile_##I##C##_##B(const std::filesystem::path &path) { return LoadFile_##I<Cu##C, Backend::B>(path); }

#define MakeImageFileDef(C, B) MakeImageFileDefImpl(C, B, Image)
#define MakeDiscreteImageFileDef(C, B) MakeImageFileDefImpl(C, B, DiscreteImage)

#ifdef CU_IMG_HAS_STB
    MakeImageFileDef(RGBA, STB);
    MakeImageFileDef(RGB, STB);
#endif

#ifdef CU_IMG_HAS_DIRECTXTEX
    MakeImageFileDef(RGBA, DirectXTex);
#endif

#ifdef CU_IMG_HAS_QT
    MakeImageFileDef(RGBA, QT);
    MakeImageFileDef(RGB, QT);
#endif

#ifdef CU_IMG_HAS_WXWIDGETS
    MakeDiscreteImageFileDef(RGBA, wxWidgets);
#endif

#ifdef CU_IMG_HAS_OPENCV
    MakeImageFileDef(RGBA, OpenCV);
    MakeImageFileDef(RGB, OpenCV);
#endif

#ifdef CU_IMG_HAS_GRAPHICSMAGICK
    MakeDiscreteImageFileDef(RGBOpacity16, GraphicsMagick);
#endif

#undef MakeImageFileDef
#undef MakeDiscreteImageFileDef
#undef MakeImageFileDefImpl
};