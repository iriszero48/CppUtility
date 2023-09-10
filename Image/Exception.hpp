#pragma once

#include "../Exception/Except.hpp"

#include <string>
#include <stdexcept>
#include <string_view>

namespace CuImg
{
    namespace Detail
    {
        template <typename>
        inline constexpr bool False = false;
    }

    class Exception : public std::runtime_error
    {
    public:
        using std::runtime_error::runtime_error;
    };

    class StdException : public Exception
    {
    public:
        using Exception::Exception;
    };

    class StbException : public Exception
    {
    public:
        using Exception::Exception;
    };

    class DirectXTexException : public Exception
    {
    public:
        using Exception::Exception;
    };

    class QtException : public Exception
    {
    public:
        using Exception::Exception;
    };

    class WxImageException : public Exception
    {
    public:
        using Exception::Exception;
    };

    class OpenCvException : public Exception
    {
    public:
        using Exception::Exception;
    };

    class GraphicsMagickException : public Exception
    {
    public:
        using Exception::Exception;
    };
    
#define CuImg_MakeEx(ex, ...) CuExcept_MakeException(ex, __VA_ARGS__)  
#define CuImg_MakeException(...) CuImg_MakeEx(Exception, __VA_ARGS__)
#define CuImg_StdException(...) CuImg_MakeEx(StdException, __VA_ARGS__)
#define CuImg_StbException(...) CuImg_MakeEx(StbException, __VA_ARGS__)
#define CuImg_DirectXTexException(...) CuImg_MakeEx(DirectXTexException, __VA_ARGS__)
#define CuImg_QtException(...) CuImg_MakeEx(QtException, __VA_ARGS__)
#define CuImg_WxImageException(...) CuImg_MakeEx(WxImageException, __VA_ARGS__)
#define CuImg_OpenCvException(...) CuImg_MakeEx(OpenCvException, __VA_ARGS__)
#define CuImg_GraphicsMagickException(...) CuImg_MakeEx(GraphicsMagickException, __VA_ARGS__)
};