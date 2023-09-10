#pragma once

#include "Backend.hpp"
#include "Context_None.hpp"
#include "Context_STB.hpp"
#include "Context_DirectXTex.hpp"
#include "Context_GraphicsMagick.hpp"
#include "Context_QT.hpp"
#include "Context_wxWidgets.hpp"
#include "Context_OPENCV.hpp"

namespace CuImg
{
    template <typename, Backend>
    struct DiscreteImageContextType
    {
    };

    template <typename, Backend>
    struct ImageContextType
    {
    };

#define MakeContextType(i, c, b, t) \
    template <>                     \
    struct i<c, Backend::b>         \
    {                               \
        using Type = t;             \
    }

#define MakeDiscreteImageContextType(c, b, t) MakeContextType(DiscreteImageContextType, c, b, t)
#define MakeImageContextType(c, b, t) MakeContextType(ImageContextType, c, b, t)

    template <typename T>
    struct ImageContextType<T, Backend::None>
    {
        using Type = StdContext<T>;
    };

    template <typename T>
    struct ImageContextType<T, Backend::Ref>
    {
        using Type = RefContext<T>;
    };

    template <typename T>
    struct ImageContextType<T, Backend::ConstRef>
    {
        using Type = ConstRefContext<T>;
    };

#ifdef CU_IMG_HAS_STB
    MakeImageContextType(CuRGB, STB, StbContext<CuRGB>);
    MakeImageContextType(CuRGBA, STB, StbContext<CuRGBA>);
#endif

#ifdef CU_IMG_HAS_DIRECTXTEX
    MakeImageContextType(CuRGBA, DirectXTex, DirectXTexContext);
#endif

#ifdef CU_IMG_HAS_QT
    MakeImageContextType(CuRGB, QT, QtImageContext<CuRGB>);
    MakeImageContextType(CuRGBA, QT, QtImageContext<CuRGBA>);
#endif

#ifdef CU_IMG_HAS_OPENCV
    MakeImageContextType(CuRGB, OpenCV, OpenCvContext<CuRGB>);
    MakeImageContextType(CuRGBA, OpenCV, OpenCvContext<CuRGBA>);
    MakeImageContextType(CuBGR, OpenCV, OpenCvContext<CuBGR>);
#endif

#ifdef CU_IMG_HAS_WXWIDGETS
    MakeDiscreteImageContextType(CuRGBA, wxWidgets, WxImageDiscreteContext);
#endif

#ifdef CU_IMG_HAS_GRAPHICSMAGICK
    MakeDiscreteImageContextType(CuRGBOpacity16, GraphicsMagick, GraphicsMagickContext);
#endif

#undef MakeContextType
#undef MakeDiscreteImageContextType
#undef MakeImageContextType
};