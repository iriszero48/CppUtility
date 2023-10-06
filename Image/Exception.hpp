#pragma once

#include "../Exception/Except.hpp"
#include "../String/String.hpp"

#include <string>
#include <stdexcept>
#include <string_view>

namespace CuImg
{
    CuExcept_MakeException(Exception, CuExcept, U8Exception);
    CuExcept_MakeException(StdException, CuImg, Exception);
    CuExcept_MakeException(StbException, CuImg, Exception);
    CuExcept_MakeException(DirectXTexException, CuImg, Exception);
    CuExcept_MakeException(QtException, CuImg, Exception);
    CuExcept_MakeException(WxImageException, CuImg, Exception);
    CuExcept_MakeException(OpenCvException, CuImg, Exception);
    CuExcept_MakeException(GraphicsMagickException, CuImg, Exception);

#define CuImg_MakeEx(ex, ...) ex(CuStr::AppendsU8(__VA_ARGS__))  
#define CuImg_MakeException(...) CuImg_MakeEx(Exception, __VA_ARGS__)
#define CuImg_StdException(...) CuImg_MakeEx(StdException, __VA_ARGS__)
#define CuImg_StbException(...) CuImg_MakeEx(StbException, __VA_ARGS__)
#define CuImg_DirectXTexException(...) CuImg_MakeEx(DirectXTexException, __VA_ARGS__)
#define CuImg_QtException(...) CuImg_MakeEx(QtException, __VA_ARGS__)
#define CuImg_WxImageException(...) CuImg_MakeEx(WxImageException, __VA_ARGS__)
#define CuImg_OpenCvException(...) CuImg_MakeEx(OpenCvException, __VA_ARGS__)
#define CuImg_GraphicsMagickException(...) CuImg_MakeEx(GraphicsMagickException, __VA_ARGS__)
};