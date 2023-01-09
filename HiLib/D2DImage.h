#pragma once
#include "ReactiveProperty.h"
#include "Direct2DWrite.h"
#include "getter_macro.h"

class CD2DImage
{
private:
	std::wstring m_path;
	CComPtr<ID2D1Bitmap1> m_pBitmap;

public:
	const std::wstring& GetPath() const { return m_path; }
	const CComPtr<ID2D1Bitmap1>& GetBitmapPtr() const { return m_pBitmap; }

	CD2DImage(const std::wstring& path = std::wstring())
		:m_path(path), m_pBitmap(){}
	virtual ~CD2DImage() = default;
	CD2DImage(const CD2DImage& rhs)
	{
		m_path = rhs.m_path;
		m_pBitmap = rhs.m_pBitmap;
	}
	
	CD2DImage& operator=(const CD2DImage& rhs)
	{
		m_path = rhs.m_path;
		m_pBitmap = rhs.m_pBitmap;
		return *this;
	}

	bool operator==(const CD2DImage& rhs) const
	{
		return
		m_path == rhs.m_path &&
		m_pBitmap == rhs.m_pBitmap;
	}
	bool operator!=(const CD2DImage& rhs) const
	{
		return !operator==(rhs);
	}

	void Open(CDirect2DWrite* pDirect, const std::wstring& path);
	void Close();
	void Save(const std::wstring& path);

	friend void to_json(json& j, const CD2DImage& o)
	{
		j["Path"] = o.m_path;
	}
	friend void from_json(const json& j, CD2DImage& o)
	{
		j.at("Path").get_to(o.m_path);
	}

};