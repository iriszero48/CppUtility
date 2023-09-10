#pragma once

namespace CuImg
{
    enum class Backend
    {
        None,
        Ref,
        ConstRef,
        STB,
        DirectXTex,
        OpenCV,
        QT,
        wxWidgets,
        GraphicsMagick
    };
};