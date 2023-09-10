#pragma once

#include <wincodec.h>
#include <propvarutil.h>
#include <strsafe.h>

#include "File_Utility.hpp"
#include "Image.hpp"
#include "Convert/Convert.hpp"

#pragma comment(lib, "Propsys.lib")

namespace CuImg::Detail::DxTex
{
	namespace Api
	{
		inline void Compress(
			ID3D11Device *pDevice,
			const DirectX::Image &srcImage,
			DXGI_FORMAT format,
			DirectX::TEX_COMPRESS_FLAGS compress,
			float alphaWeight,
			DirectX::ScratchImage &image)
		{
			CuImg__DxTex_WIN_API(DirectX::Compress(pDevice, srcImage, format, compress, alphaWeight, image));
		}

		inline void Compress(
			const DirectX::Image &srcImage,
			DXGI_FORMAT format,
			DirectX::TEX_COMPRESS_FLAGS compress,
			float alphaWeight,
			DirectX::ScratchImage &image)
		{
			CuImg__DxTex_WIN_API(DirectX::Compress(srcImage, format, compress, alphaWeight, image));
		}

		inline void Compress(const DirectX::Image *srcImages, size_t nimages,
							 const DirectX::TexMetadata &metadata,
							 DXGI_FORMAT format, DirectX::TEX_COMPRESS_FLAGS compress, float threshold,
							 DirectX::ScratchImage &cImages)
		{
			CuImg__DxTex_WIN_API(DirectX::Compress(srcImages, nimages, metadata, format, compress, threshold, cImages));
		}

		inline void Convert(
			const DirectX::Image &srcImage, DXGI_FORMAT format, DirectX::TEX_FILTER_FLAGS filter, float threshold,
			DirectX::ScratchImage &image)
		{
			CuImg__DxTex_WIN_API(DirectX::Convert(srcImage, format, filter, threshold, image));
		}

		inline void SaveToDDSFile(const DirectX::Image &image, DirectX::DDS_FLAGS flags, const wchar_t *szFile)
		{
			CuImg__DxTex_WIN_API(DirectX::SaveToDDSFile(image, flags, szFile));
		}

		inline void LoadFromDDSFile(const wchar_t *szFile, DirectX::DDS_FLAGS flags, DirectX::TexMetadata *metadata,
									DirectX::ScratchImage &image)
		{
			CuImg__DxTex_WIN_API(DirectX::LoadFromDDSFile(szFile, flags, metadata, image));
		}

		inline void LoadFromTGAFile(const wchar_t *szFile,
									DirectX::TGA_FLAGS flags,
									DirectX::TexMetadata *metadata, DirectX::ScratchImage &image)
		{
			CuImg__DxTex_WIN_API(DirectX::LoadFromTGAFile(szFile, flags, metadata, image));
		}

		inline void SaveToTGAFile(const DirectX::Image &image,
								  DirectX::TGA_FLAGS flags,
								  const wchar_t *szFile,
								  const DirectX::TexMetadata *metadata = nullptr)
		{
			CuImg__DxTex_WIN_API(DirectX::SaveToTGAFile(image, flags, szFile, metadata));
		}

		inline void LoadFromHDRFile(const wchar_t *szFile,
									DirectX::TexMetadata *metadata, DirectX::ScratchImage &image)
		{
			CuImg__DxTex_WIN_API(DirectX::LoadFromHDRFile(szFile, metadata, image));
		}

		inline void SaveToHDRFile(const DirectX::Image &image, const wchar_t *szFile)
		{
			CuImg__DxTex_WIN_API(DirectX::SaveToHDRFile(image, szFile));
		}

		inline void LoadFromWICFile(const wchar_t *szFile,
									DirectX::WIC_FLAGS flags, DirectX::TexMetadata *metadata,
									DirectX::ScratchImage &image,
									std::function<void(IWICMetadataQueryReader *)> getMQR = nullptr)
		{
			CuImg__DxTex_WIN_API(DirectX::LoadFromWICFile(szFile, flags, metadata, image, getMQR));
		}

		inline void SaveToWICFile(const DirectX::Image &image,
								  DirectX::WIC_FLAGS flags, REFGUID guidContainerFormat,
								  const wchar_t *szFile, const GUID *targetFormat = nullptr,
								  std::function<void(IPropertyBag2 *)> setCustomProps = nullptr)
		{
			CuImg__DxTex_WIN_API(
				DirectX::SaveToWICFile(image, flags, guidContainerFormat, szFile, targetFormat, setCustomProps));
		}
	}

	enum class WicParam
	{
		Other,
		JPEG,
		JPEGXR,
		BMP,
		PNG,
		HDPhoto,
		TIFF
	};

	inline void DirectXTexWicLoadInfo(IWICMetadataQueryReader *reader,
									  decltype(DirectXTexContext::LoadInfo::WicProps) &dict)
	{
		for (const auto &kv : CuEnum::Values<DirectXTexContext::PhotoProp>())
		{
			const auto k = CuStr::FormatW("System.Photo.{}", CuEnum::ToString(kv));

			PROPVARIANT value;
			PropVariantInit(&value);
			const auto clear = [&]()
			{
				return PropVariantClear(&value);
			};

			try
			{
				auto hr = reader->GetMetadataByName(k.c_str(), &value);
				if (FAILED(hr))
				{
					clear();
					if (hr != WINCODEC_ERR_PROPERTYNOTFOUND && hr != WINCODEC_ERR_PROPERTYNOTSUPPORTED)
						OutputDebugString(std::filesystem::path(CuStr::Combine(
																	CuImg_DirectXTexException("[IWICMetadataQueryReader::GetMetadataByName] ", ErrStr(hr)).what()))
											  .native()
											  .c_str());
					continue;
				}

				DirectXTexContext::PropType val;
				bool isEmpty = false;

				switch (value.vt)
				{
#define MakeVtBastType(InfoType, WinType)                                                \
	{                                                                                    \
		DirectXTexContext::InfoType v;                                                   \
		hr = PropVariantTo##WinType(value, &v);                                          \
		if (FAILED(hr))                                                                  \
			throw CuImg_DirectXTexException("[PropVariantTo" #WinType "] ", ErrStr(hr)); \
		val.emplace<DirectXTexContext::InfoType>(v);                                     \
	}

				case VT_I1:
					val.emplace<DirectXTexContext::VtI1>(value.cVal);
					break;
				case VT_UI1:
					val.emplace<DirectXTexContext::VtUi1>(value.bVal);
					break;

				case VT_I2:
					MakeVtBastType(VtI2, Int16);
					break;
				case VT_UI2:
					MakeVtBastType(VtUi2, UInt16);
					break;

				case VT_I4:
					MakeVtBastType(VtI4, Int32);
					break;
				case VT_UI4:
					MakeVtBastType(VtUi4, UInt32);
					break;

				case VT_I8:
					MakeVtBastType(VtI8, Int64);
					break;
				case VT_UI8:
					MakeVtBastType(VtUi8, UInt64);
					break;

				case VT_BOOL:
				{
					BOOL v;
					hr = PropVariantToBoolean(value, &v);
					if (FAILED(hr))
						throw CuImg_DirectXTexException("[PropVariantToBoolean] ", ErrStr(hr));
					val.emplace<DirectXTexContext::VtBool>(v);
				}
				break;

				case VT_R4:
					val.emplace<DirectXTexContext::VtR4>(value.fltVal);
					break;

				case VT_R8:
					MakeVtBastType(VtR8, Double);
					break;

				case VT_LPWSTR:
				case VT_BSTR:
				{
					PWSTR pszValue;
					hr = PropVariantGetStringElem(value, 0, &pszValue);
					if (FAILED(hr))
						throw CuImg_DirectXTexException("[PropVariantGetStringElem] ", ErrStr(hr));
					val.emplace<DirectXTexContext::VtWStr>(pszValue);
					CoTaskMemFree(pszValue);
				}
				break;

				case VT_VECTOR | VT_LPWSTR:
				case VT_VECTOR | VT_BSTR:
				case VT_ARRAY | VT_LPWSTR:
				case VT_ARRAY | VT_BSTR:
				{
					DirectXTexContext::VtWStrArray buf;
					const auto cElem = PropVariantGetElementCount(value);
					for (UINT iElem = 0; iElem < cElem; ++iElem)
					{
						PWSTR pszValue;
						hr = PropVariantGetStringElem(value, iElem, &pszValue);
						if (FAILED(hr))
							throw CuImg_DirectXTexException("[PropVariantGetStringElem] ", ErrStr(hr));

						buf.emplace_back(pszValue);
						CoTaskMemFree(pszValue);
					}
					val.emplace<DirectXTexContext::VtWStrArray>(std::move(buf));
				}
				break;

				case VT_DATE:
				case VT_FILETIME:
				{
					FILETIME ftModified;
					hr = PropVariantToFileTime(value, PSTF_LOCAL, &ftModified);
					if (FAILED(hr))
						throw CuImg_DirectXTexException("[PropVariantToFileTime] ", ErrStr(hr));

					val.emplace<DirectXTexContext::VtTime>(std::chrono::file_clock::duration(
						static_cast<int64_t>(ftModified.dwHighDateTime) << 32 | ftModified.dwLowDateTime));
				}
				break;

				case VT_ARRAY | VT_UI1:
				case VT_VECTOR | VT_UI1:
				{
					const auto cElem = PropVariantGetElementCount(value);
					DirectXTexContext::VtUi1Array buf(cElem);
					CuUtil_Assert(buf.size() < UINT_MAX, DirectXTexException);
					hr = PropVariantToBuffer(value, buf.data(), static_cast<UINT>(buf.size()));
					if (FAILED(hr))
						throw CuImg_DirectXTexException("[PropVariantToBuffer] ", ErrStr(hr));
					val.emplace<DirectXTexContext::VtUi1Array>(std::move(buf));
				}
				break;
				case VT_BLOB:
				{
					DirectXTexContext::VtUi1Array buf(value.blob.cbSize);
					std::copy_n(value.blob.pBlobData, buf.size(), buf.begin());
					val.emplace<DirectXTexContext::VtUi1Array>(std::move(buf));
				}
				break;

				default:
					isEmpty = true;
					OutputDebugString(
						std::filesystem::path(CuStr::Combine("ignore PROPVARIANT type: 0x",
															 CuConv::ToString(value.vt, 16).value()))
							.native()
							.c_str());
					break;
#undef MakeVtBastType
				}

				if (!isEmpty)
					dict[kv] = std::move(val);
			}
			catch (const std::exception_ptr &e)
			{
				clear();
				std::rethrow_exception(e);
			}

			if (const auto hr = clear(); FAILED(hr))
				throw CuImg_DirectXTexException("[PropVariantClear] ", ErrStr(hr));
		}
	}

	inline void DirectXTexLoadFile(const std::filesystem::path &path, ImageRGBA_DirectXTex &img)
	{
		using CtxType = ImageRGBA_DirectXTex::ContextType;
		CtxType ctx{};
		ctx.GetInfo().SkipLoadWicProps = img.GetInfo().SkipLoadWicProps;
		const auto skipLoadWicProps = ctx.GetInfo().SkipLoadWicProps;

		DirectX::TexMetadata info{};
		DirectX::ScratchImage raw;
		HRESULT hr;

		if (const auto ext = CuStr::ToLower(path.extension().string()); IsDDS(ext))
		{
			Api::LoadFromDDSFile(path.wstring().c_str(), DirectX::DDS_FLAGS::DDS_FLAGS_NONE, &info, raw);
		}
		else if (IsHDR(ext))
		{
			Api::LoadFromHDRFile(path.wstring().c_str(), &info, raw);
		}
		else if (IsTGA(ext))
		{
			Api::LoadFromTGAFile(path.wstring().c_str(), DirectX::TGA_FLAGS_NONE, &info, raw);
		}
		else
		{
			std::function<void(IWICMetadataQueryReader *)> mqr = nullptr;
			if (!skipLoadWicProps)
			{
				mqr = [&](IWICMetadataQueryReader *reader)
				{
					DirectXTexWicLoadInfo(reader, ctx.GetInfo().WicProps);
				};
			}
			Api::LoadFromWICFile(
				path.wstring().c_str(), DirectX::WIC_FLAGS_NONE, &info, raw, mqr);
		}

		ctx.GetInfo().SrcFmt = info.format;

		if (DirectX::IsPlanar(info.format))
		{
			DirectX::ScratchImage nonPlanared;
			hr = ConvertToSinglePlane(*raw.GetImages(), nonPlanared);
			if (FAILED(hr))
			{
				throw Exception("LoadFromDDSFile error");
			}
			raw = std::move(nonPlanared);
		}

		if (raw.GetImages()->format != DXGI_FORMAT_R8G8B8A8_UNORM)
		{
			if (DirectX::IsCompressed(raw.GetImages()->format))
			{
				DirectX::ScratchImage decompressed;
				hr = DirectX::Decompress(*raw.GetImages(), DXGI_FORMAT_R8G8B8A8_UNORM, decompressed);
				if (FAILED(hr))
				{
					throw Exception("LoadFromDDSFile error");
				}
				raw = std::move(decompressed);
			}
			else
			{
				DirectX::ScratchImage converted;
				hr = DirectX::Convert(*raw.GetImages(),
									  DXGI_FORMAT_R8G8B8A8_UNORM,
									  DirectX::TEX_FILTER_FLAGS::TEX_FILTER_DEFAULT,
									  DirectX::TEX_THRESHOLD_DEFAULT, converted);
				if (FAILED(hr))
				{
					throw Exception("Convert error");
				}

				raw = std::move(converted);
			}
		}

		ctx.Image = std::move(raw);
		img.GetContext() = std::move(ctx);
	}

	template <typename T>
	struct DirectXTexWicWriteVariant
	{
	};

#define MakeWicWriteVariantBase(stdType, winType)                                              \
	template <>                                                                                \
	struct DirectXTexWicWriteVariant<stdType>                                                  \
	{                                                                                          \
		void operator()(VARIANT &var, const stdType &val)                                      \
		{                                                                                      \
			const auto hr = InitVariantFrom##winType(val, &var);                               \
			if (FAILED(hr))                                                                    \
				throw CuImg_DirectXTexException("[InitVariantFrom" #winType "] ", ErrStr(hr)); \
		}                                                                                      \
	}

#define MakeWicWriteVariantBaseArray(stdType, winType)                                                 \
	template <>                                                                                        \
	struct DirectXTexWicWriteVariant<stdType>                                                          \
	{                                                                                                  \
		void operator()(VARIANT &var, const stdType &val)                                              \
		{                                                                                              \
			const auto hr = InitVariantFrom##winType(val.data(), static_cast<UINT>(val.size()), &var); \
			if (FAILED(hr))                                                                            \
				throw CuImg_DirectXTexException("[InitVariantFrom" #winType "] ", ErrStr(hr));         \
		}                                                                                              \
	}

#define MakeWicWriteVariantEnumUi1(enumType)                         \
	template <>                                                      \
	struct DirectXTexWicWriteVariant<enumType>                       \
	{                                                                \
		void operator()(VARIANT &var, const enumType &val)           \
		{                                                            \
			DirectXTexWicWriteVariant<uint8_t>{}(var, (uint8_t)val); \
		}                                                            \
	}

#define MakeWicWriteVariantOptional(stdType)                             \
	template <>                                                          \
	struct DirectXTexWicWriteVariant<std::optional<stdType>>             \
	{                                                                    \
		void operator()(VARIANT &var, const std::optional<stdType> &val) \
		{                                                                \
			if (val.has_value())                                         \
				DirectXTexWicWriteVariant<stdType>{}(var, *val);         \
			else                                                         \
				var.vt = VT_NULL;                                        \
		}                                                                \
	}

#define MakeWicWriteVariantRaw(stdType, varType, varMember)     \
	template <>                                                 \
	struct DirectXTexWicWriteVariant<stdType>                   \
	{                                                           \
		void operator()(VARIANT &var, const stdType &val) const \
		{                                                       \
			var.varMember = val;                                \
			var.vt = varType;                                   \
		}                                                       \
	}

	MakeWicWriteVariantRaw(std::int8_t, VT_I1, cVal);

	MakeWicWriteVariantRaw(std::uint8_t, VT_UI1, bVal);

	MakeWicWriteVariantBase(std::int16_t, Int16);

	MakeWicWriteVariantBase(std::uint16_t, UInt16);

	MakeWicWriteVariantBase(std::int32_t, Int32);

	MakeWicWriteVariantBase(std::uint32_t, UInt32);

	MakeWicWriteVariantBase(std::int64_t, Int64);

	MakeWicWriteVariantBase(std::uint64_t, UInt64);

	MakeWicWriteVariantRaw(float, VT_R4, fltVal);

	MakeWicWriteVariantBase(double, Double);

	MakeWicWriteVariantBaseArray(DirectXTexContext::VtUi1Array, Buffer);

	template <>
	struct DirectXTexWicWriteVariant<DirectXTexContext::VtWStr>
	{
		void operator()(VARIANT &var, const DirectXTexContext::VtWStr &val) const
		{
			if (const auto hr = InitVariantFromString(val.c_str(), &var); FAILED(hr))
				throw CuImg_DirectXTexException("[InitVariantFromString] ", ErrStr(hr));
		}
	};

	template <>
	struct DirectXTexWicWriteVariant<DirectXTexContext::VtWStrArray>
	{
		void operator()(VARIANT &var, const DirectXTexContext::VtWStrArray &val) const
		{
			std::vector<const wchar_t *> data{};
			for (const auto &str : val)
			{
				data.push_back(str.data());
			}
			if (const auto hr = InitVariantFromStringArray(data.data(), static_cast<ULONG>(data.size()), &var); FAILED(hr))
				throw CuImg_DirectXTexException("[InitVariantFromStringArray] ", ErrStr(hr));
		}
	};

	MakeWicWriteVariantBase(bool, Boolean);

	template <>
	struct DirectXTexWicWriteVariant<DirectXTexContext::VtTime>
	{
		void operator()(VARIANT &var, const DirectXTexContext::VtTime &val) const
		{
			const auto ts = val.time_since_epoch().count();
			const FILETIME ft{static_cast<DWORD>(ts & 0xffffffff), static_cast<DWORD>(ts >> 32)};
			if (const auto hr = InitVariantFromFileTime(&ft, &var); FAILED(hr))
				throw CuImg_DirectXTexException("[InitVariantFromFileTime] ", ErrStr(hr));
		}
	};

	template <>
	struct DirectXTexWicWriteVariant<std::array<std::uint32_t, 64>>
	{
		void operator()(VARIANT &var, const std::array<std::uint32_t, 64> &val) const
		{
			const auto hr = InitVariantFromUInt32Array(reinterpret_cast<const ULONG *>(val.data()), (ULONG)val.size(), &var);
			if (FAILED(hr))
				throw CuImg_DirectXTexException("[InitVariantFromUInt32Array] ", ErrStr(hr));
		}
	};

	MakeWicWriteVariantOptional(std::uint8_t);

	MakeWicWriteVariantOptional(std::uint16_t);

	using DxTexUInt32Array = std::array<std::uint32_t, 64>;
	MakeWicWriteVariantOptional(DxTexUInt32Array);

	MakeWicWriteVariantEnumUi1(WICBitmapTransformOptions);

	MakeWicWriteVariantEnumUi1(WICJpegYCrCbSubsamplingOption);

	MakeWicWriteVariantEnumUi1(WICPngFilterOption);

	MakeWicWriteVariantEnumUi1(WICTiffCompressionOption);

#undef MakeWicWriteVariantBase
#undef MakeWicWriteVariantBaseArray
#undef MakeWicWriteVariantEnumUi1
#undef MakeWicWriteVariantOptional
#undef MakeWicWriteVariantRaw

	template <typename T>
	void DirectXTexWicWriteProp(IPropertyBag2 *pb, std::wstring key, const T &val)
	{
		PROPBAG2 pairKey{0};
		pairKey.pstrName = key.data();

		VARIANT pairValue;
		VariantInit(&pairValue);
		const auto clear = [&]()
		{
			return VariantClear(&pairValue);
		};
		try
		{
			DirectXTexWicWriteVariant<T>{}(pairValue, val);
			if (pairValue.vt != VT_NULL)
			{
				if (const auto hr = pb->Write(1, &pairKey, &pairValue); FAILED(hr))
					throw CuImg_DirectXTexException("[IPropertyBag2::Write] ", ErrStr(hr));
			}
		}
		catch (const std::exception_ptr &e)
		{
			clear();
			std::rethrow_exception(e);
		}
		if (const auto hr = clear(); FAILED(hr))
			throw CuImg_DirectXTexException("[VariantClear] ", ErrStr(hr));
	}

	template <typename T>
	decltype(auto) DirectXTexRangeAssertContinueQ(const std::optional<T> &v)
	{
		return v.has_value();
	}

	template <typename T>
	decltype(auto) DirectXTexRangeAssertContinueQ(const T &v)
	{
		return true;
	}

	template <typename T>
	decltype(auto) DirectXTexRangeAssertValues(const std::optional<T> &v)
	{
		return *v;
	}

	template <typename T>
	decltype(auto) DirectXTexRangeAssertValues(const T &v)
	{
		return v;
	}

	inline void DirectXTexWicSaveParam(IPropertyBag2 *pb, const DirectXTexContext::SaveParam &params,
									   const WicParam codec)
	{
#define MakeWriteProp(codec, prop) DirectXTexWicWriteProp(pb, L"" #prop, params.codec.prop)
#define RangeAssert(codec, prop, beg, end)                                                                                           \
	{                                                                                                                                \
		if (const auto rv = params.codec.prop; DirectXTexRangeAssertContinueQ(rv))                                                   \
		{                                                                                                                            \
			if (const auto v = DirectXTexRangeAssertValues(rv); v < (beg) || v > (end))                                              \
				throw CuImg_DirectXTexException("RangeAssert: " #beg " <= " #codec "." #prop "(", CuStr::ToString(v), ") <= " #end); \
		}                                                                                                                            \
	}

		switch (codec)
		{
		case WicParam::BMP:
			MakeWriteProp(BMP, EnableV5Header32bppBGRA);
			break;

		case WicParam::HDPhoto:
			RangeAssert(HDPhoto, ImageQuality, 0.f, 1.f);
			RangeAssert(HDPhoto, Quality, 1, 255);
			RangeAssert(HDPhoto, Overlap, 0, 2);
			RangeAssert(HDPhoto, Subsampling, 0, 3);
			RangeAssert(HDPhoto, HorizontalTileSlices, 0, 4095);
			RangeAssert(HDPhoto, VerticalTileSlices, 0, 4095);
			RangeAssert(HDPhoto, AlphaQuality, 1, 255);
			RangeAssert(HDPhoto, ImageDataDiscard, 0, 3);
			RangeAssert(HDPhoto, AlphaDataDiscard, 0, 4);

			MakeWriteProp(HDPhoto, ImageQuality);
			MakeWriteProp(HDPhoto, Lossless);
			MakeWriteProp(HDPhoto, BitmapTransform);
			MakeWriteProp(HDPhoto, UseCodecOptions);
			MakeWriteProp(HDPhoto, Quality);
			MakeWriteProp(HDPhoto, Overlap);
			MakeWriteProp(HDPhoto, Subsampling);
			MakeWriteProp(HDPhoto, HorizontalTileSlices);
			MakeWriteProp(HDPhoto, VerticalTileSlices);
			MakeWriteProp(HDPhoto, FrequencyOrder);
			MakeWriteProp(HDPhoto, InterleavedAlpha);
			MakeWriteProp(HDPhoto, AlphaQuality);
			MakeWriteProp(HDPhoto, CompressedDomainTranscode);
			MakeWriteProp(HDPhoto, ImageDataDiscard);
			MakeWriteProp(HDPhoto, AlphaDataDiscard);
			MakeWriteProp(HDPhoto, IgnoreOverlap);
			break;

		case WicParam::JPEG:
			RangeAssert(JPEG, ImageQuality, 0.f, 1.f);

			MakeWriteProp(JPEG, ImageQuality);
			MakeWriteProp(JPEG, BitmapTransform);
			MakeWriteProp(JPEG, Luminance);
			MakeWriteProp(JPEG, Chrominance);
			MakeWriteProp(JPEG, JpegYCrCbSubsampling);
			MakeWriteProp(JPEG, SuppressApp0);
			break;

		case WicParam::JPEGXR:
			RangeAssert(JPEGXR, AlphaDataDiscard, 0, 4);
			RangeAssert(JPEGXR, AlphaQuality, 1, 255);
			RangeAssert(JPEGXR, HorizontalTileSlices, 0, 4095);
			RangeAssert(JPEGXR, ImageDataDiscard, 0, 3);
			RangeAssert(JPEGXR, ImageQuality, 0.f, 1.f);
			RangeAssert(JPEGXR, Overlap, 0, 4);
			RangeAssert(JPEGXR, Quality, 1, 255);
			RangeAssert(JPEGXR, Subsampling, 0, 3);
			RangeAssert(JPEGXR, VerticalTileSlices, 0, 4095);

			MakeWriteProp(JPEGXR, AlphaDataDiscard);
			MakeWriteProp(JPEGXR, AlphaQuality);
			MakeWriteProp(JPEGXR, BitmapTransform);
			MakeWriteProp(JPEGXR, CompressedDomainTranscode);
			MakeWriteProp(JPEGXR, FrequencyOrder);
			MakeWriteProp(JPEGXR, HorizontalTileSlices);
			MakeWriteProp(JPEGXR, IgnoreOverlap);
			MakeWriteProp(JPEGXR, ImageDataDiscard);
			MakeWriteProp(JPEGXR, ImageQuality);
			MakeWriteProp(JPEGXR, InterleavedAlpha);
			MakeWriteProp(JPEGXR, Lossless);
			MakeWriteProp(JPEGXR, Overlap);
			MakeWriteProp(JPEGXR, ProgressiveMode);
			MakeWriteProp(JPEGXR, Quality);
			MakeWriteProp(JPEGXR, StreamOnly);
			MakeWriteProp(JPEGXR, Subsampling);
			MakeWriteProp(JPEGXR, UseCodecOptions);
			MakeWriteProp(JPEGXR, VerticalTileSlices);
			break;

		case WicParam::PNG:
			MakeWriteProp(PNG, InterlaceOption);
			MakeWriteProp(PNG, FilterOption);
			break;

		case WicParam::TIFF:
			RangeAssert(TIFF, CompressionQuality, 0.f, 1.f);

			MakeWriteProp(TIFF, CompressionQuality);
			MakeWriteProp(TIFF, TiffCompressionMethod);
			break;

		default:
			break;
		}

		for (const auto &pair : params.WicProps)
		{
			std::visit([&](const auto &val)
					   { DirectXTexWicWriteProp(pb, CuStr::FormatW("System.Photo.{}", CuEnum::ToString(pair.first)),
												val); },
					   pair.second);
		}
#undef MakeWriteProp
#undef RangeAssert
	}

	inline void DirectXTexSaveFile(const std::filesystem::path &path, const ImageRGBA_DirectXTex &img)
	{
		const auto ext = CuStr::ToLower(path.extension().native());
		const DirectX::Image out{
			img.Width(),
			img.Height(),
			DXGI_FORMAT_R8G8B8A8_UNORM,
			img.Linesize(),
			img.Linesize() * img.Height(),
			const_cast<uint8_t *>(img.Data())};

		DirectX::ScratchImage premulted{};
		DirectX::ScratchImage compressed{};
		DirectX::ScratchImage conved{};

		auto const *ptr = &out;

		if (img.GetParam().TexAlphaModePremultiplied)
		{
			DirectX::PremultiplyAlpha(*ptr, DirectX::TEX_PMALPHA_DEFAULT, premulted);
			ptr = premulted.GetImages();
		}

		if (DirectX::IsCompressed(img.GetParam().DestFmt))
		{
			if (img.GetParam().CompressDevice)
			{
				Api::Compress(img.GetParam().CompressDevice, *ptr, img.GetParam().DestFmt,
							  DirectX::TEX_COMPRESS_DEFAULT, 1.0f, compressed);
			}
			else
			{
				Api::Compress(*ptr, img.GetParam().DestFmt, DirectX::TEX_COMPRESS_DEFAULT, 1.0f, compressed);
			}
			ptr = compressed.GetImages();
		}

		if (img.GetParam().DestFmt != ptr->format)
		{
			Api::Convert(*ptr, img.GetParam().DestFmt, DirectX::TEX_FILTER_DEFAULT, DirectX::TEX_THRESHOLD_DEFAULT,
						 conved);
			ptr = conved.GetImages();
		}

		if (IsDDS(ext))
		{
			Api::SaveToDDSFile(*ptr, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, path.wstring().c_str());
		}
		else if (IsHDR(ext))
		{
			Api::SaveToHDRFile(*ptr, path.wstring().c_str());
		}
		else if (IsTGA(ext))
		{
			Api::SaveToTGAFile(*ptr, DirectX::TGA_FLAGS_NONE, path.wstring().c_str());
		}
		else
		{
			const GUID *codec;
			auto param = WicParam::Other;

			if (IsBMP(ext))
			{
				param = WicParam::BMP;
				codec = &GetWICCodec(DirectX::WIC_CODEC_BMP);
			}
			else if (IsJPEG(ext))
			{
				param = WicParam::JPEG;
				codec = &GetWICCodec(DirectX::WIC_CODEC_JPEG);
			}
			else if (IsPNG(ext))
			{
				param = WicParam::PNG;
				codec = &GetWICCodec(DirectX::WIC_CODEC_PNG);
			}
			else if (IsTIFF(ext))
			{
				param = WicParam::TIFF;
				codec = &GetWICCodec(DirectX::WIC_CODEC_TIFF);
			}
			else if (IsGIF(ext))
			{
				codec = &GetWICCodec(DirectX::WIC_CODEC_GIF);
			}
			else if (IsWMP(ext))
			{
				param = WicParam::HDPhoto;
				codec = &GetWICCodec(DirectX::WIC_CODEC_WMP);
			}
			else if (IsJPEG_XR(ext))
			{
				param = WicParam::JPEGXR;
				codec = &GetWICCodec(DirectX::WIC_CODEC_WMP);
			}
			else if (IsICO(ext))
			{
				codec = &GetWICCodec(DirectX::WIC_CODEC_ICO);
			}
			else if (IsHEIF(ext))
			{
				codec = &GetWICCodec(DirectX::WIC_CODEC_HEIF);
			}
			else if (IsDNG(ext))
			{
				codec = &GUID_ContainerFormatAdng;
			}
			else if (IsWEBP(ext))
			{
				codec = &GUID_ContainerFormatWebp;
			}
			else if (IsRAW(ext))
			{
				codec = &GUID_ContainerFormatRaw;
			}
			else
			{
				codec = &GUID_ContainerFormatRaw;
			}
			Api::SaveToWICFile(*ptr, DirectX::WIC_FLAGS_NONE, *codec, path.wstring().c_str(), img.GetParam().WicFmt,
							   [&](IPropertyBag2 *pb)
							   {
								   DirectXTexWicSaveParam(pb, img.GetParam(), param);
							   });
		}
	}
}
