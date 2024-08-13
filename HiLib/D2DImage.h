#pragma once
#include "reactive_property.h"
#include "Direct2DWrite.h"
#include "D2DWTypes.h"
#include "ShellFile.h"
#include "getter_macro.h"
#include "WICImagingFactory.h"

class CD2DImage: public CShellFile
{
private:
	DECLARE_LAZY_COMPTR_GETTER(IWICBitmapSource, BitmapSource);
	DECLARE_RELEASE_COMPTRS(BitmapSource);

	DECLARE_LAZY_GETTER(CSizeU, SizeU);
	const CSizeF GetSizeF() const;

public:
	std::shared_ptr<int> Dummy;
	mutable reactive_property_ptr<WICBitmapTransformOptions> Rotate;

public:
	bool IsValid() const { return ::PathFileExists(GetPath().c_str()); }
	CComPtr<IWICBitmapSource> GetBitmap(const FLOAT& scale, std::function<bool()> cancel = nullptr) const;
	CComPtr<IWICBitmapSource> GetClipBitmap(const FLOAT& scale, const CRectU& rcClip, std::function<bool()> cancel = nullptr) const;

	CD2DImage(const std::wstring& path = std::wstring());
	virtual ~CD2DImage() = default;

	CD2DImage(const CD2DImage& rhs)
	{
		*this = rhs;
	}
	
	CD2DImage& operator=(const CD2DImage& rhs)
	{
		Rotate = rhs.Rotate;
		Dummy = rhs.Dummy;
		ReleaseComPtrs();
		m_optSizeU.reset();

		CShellFile::Load(rhs.GetPath());
		return *this;
	}

	bool operator==(const CD2DImage& rhs) const
	{
		return GetPath() == rhs.GetPath();
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
		json_safe_to(j, "Path", o.GetPath());
	}
	friend void from_json(const json& j, CD2DImage& o)
	{
		std::wstring path;
		json_safe_from(j, "Path", path);
		o.CShellFile::Load(path);
	}
};