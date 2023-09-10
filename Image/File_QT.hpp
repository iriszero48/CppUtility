#pragma once

#include <QImageReader>
#include <QImageWriter>

#include "Image.hpp"

namespace CuImg::Detail::QT
{
    template <typename T, QImage::Format F>
    decltype(auto) QtLoadFile(const std::filesystem::path &path, Image<T, Backend::QT> &img)
    {
        using CtxType = typename Image<T, Backend::QT>::ContextType;

        CtxType ctx{};
        QImageReader reader;
        reader.setFileName(QString::fromUtf8(path.u8string().c_str()));
        if (!reader.read(&ctx.Img))
        {
            throw CuImg_QtException("[QImageReader::read] ", reader.errorString().toStdString());
        }

        if (auto &i = ctx.Img; i.format() != F)
        {
            i = i.convertToFormat(F);
        }
        img.GetContext() = std::move(ctx);
    }

    template <typename T>
    void QtSaveFile(const std::filesystem::path &path, const Image<T, Backend::QT> &img)
    {
        QImageWriter writer;
        writer.setFileName(QString::fromUtf8(path.u8string().c_str()));
        if (!writer.write(img.GetContext().Img))
        {
            throw CuImg_QtException("[QImageWriter::write] ", writer.errorString().toStdString());
        }
    }
}