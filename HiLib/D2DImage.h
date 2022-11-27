#pragma once
#include "ReactiveProperty.h"
#include "Direct2DWrite.h"
#include "getter_macro.h"

class CD2DImage
{
private:
	ReactiveWStringProperty m_path;
	mutable CComPtr<ID2D1Bitmap> m_pBitmap;
	mutable CDirect2DWrite* m_pCurDirect;
	mutable bool m_isPathChanged;

public:
	ReactiveWStringProperty& PropPath() { return m_path; }
	const std::wstring& GetPath() const { return m_path.get(); }
	const CComPtr<ID2D1Bitmap> GetBitmapPtr(CDirect2DWrite* pDirect) const;

	CD2DImage(const std::wstring& path = std::wstring())
		:m_pCurDirect(nullptr), m_isPathChanged(false), m_path(path), m_pBitmap()
	{
		m_path.Subscribe([this](const auto& arg) { m_isPathChanged = true; });
	}
	virtual ~CD2DImage() = default;
	CD2DImage(const CD2DImage& rhs)
	{
		m_path.set(rhs.m_path.get());
		m_pBitmap = rhs.m_pBitmap;
	}
	
	CD2DImage& operator=(const CD2DImage& rhs)
	{
		m_path.set(rhs.m_path.get());
		m_pBitmap = rhs.m_pBitmap;
		return *this;
	}

	bool operator==(const CD2DImage& rhs) const
	{
		return
		m_path.get() == rhs.m_path.get() &&
		m_pBitmap == rhs.m_pBitmap;
	}
	bool operator!=(const CD2DImage& rhs) const
	{
		return !operator==(rhs);
	}


	void Clear();
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