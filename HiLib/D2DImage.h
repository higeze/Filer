#pragma once
#include "reactive_property.h"
#include "Direct2DWrite.h"
#include "D2DWTypes.h"
#include "ShellFile.h"
#include "getter_macro.h"

class CD2DImage: public CShellFile
{
private:
	DECLARE_LAZY_GETTER(CComPtr<IWICImagingFactory2>, Factory);

	DECLARE_LAZY_GETTER(CComPtr<IWICBitmapDecoder>, Decoder);
	DECLARE_LAZY_GETTER(CComPtr<IWICBitmapFrameDecode>, FrameDecode);
	DECLARE_LAZY_GETTER(CSizeU, SizeU);
	const CSizeF GetSizeF() const;
	//DECLARE_LAZY_GETTER(CComPtr<IWICBitmapScaler>, Scaler);
	//DECLARE_LAZY_GETTER(CComPtr<IWICBitmapClipper>, Clipper);
	//DECLARE_LAZY_GETTER(CComPtr<IWICFormatConverter>, FormatConverter);

public:
	bool IsValid() const { return ::PathFileExists(m_optPath->c_str()); }
	CComPtr<ID2D1Bitmap1> GetBitmap(const CComPtr<ID2D1DeviceContext>& pContext, const FLOAT& scale) const;
	CComPtr<ID2D1Bitmap1> GetClipBitmap(const CComPtr<ID2D1DeviceContext>& pContext, const FLOAT& scale, const CRectU& rcClip, std::function<bool()> cancelz = nullptr) const;

	//const CComPtr<ID2D1Bitmap1>& GetBitmapPtr() const { return m_pBitmap; }

	CD2DImage(const std::wstring& path = std::wstring())
		:CShellFile(path)/*, m_pBitmap()*/{}
	virtual ~CD2DImage() = default;
	CD2DImage(const CD2DImage& rhs)
	{
		CShellFile::Load(rhs.GetPath());
		m_optDecoder.reset();
		m_optFrameDecode.reset();
		m_optSizeU.reset();
		//m_pBitmap = rhs.m_pBitmap;
	}
	
	CD2DImage& operator=(const CD2DImage& rhs)
	{
		CShellFile::Load(rhs.GetPath());
		m_optDecoder.reset();
		m_optFrameDecode.reset();
		m_optSizeU.reset();
		//m_pBitmap = rhs.m_pBitmap;
		return *this;
	}

	bool operator==(const CD2DImage& rhs) const
	{
		return GetPath() == rhs.GetPath();
		//&&
		//m_pBitmap == rhs.m_pBitmap;
	}
	bool operator!=(const CD2DImage& rhs) const
	{
		return !operator==(rhs);
	}

	void Open(const std::wstring& path);
	void Close();
	void Save(const std::wstring& path);

	friend void to_json(json& j, const CD2DImage& o)
	{
		j["Path"] = o.GetPath();
	}
	friend void from_json(const json& j, CD2DImage& o)
	{
		std::wstring path;
		j.at("Path").get_to(path);
		o.CShellFile::Load(path);
	}

};