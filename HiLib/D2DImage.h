#pragma once
//#include "ReactiveProperty.h"
//#include "Direct2DWrite.h"

//class CD2DImage
//{
//private:
//	CDirect2DWrite* m_pDirect;
//	ReactiveWStringProperty m_path;
//	CComPtr<ID2D1Bitmap> m_pBitmap;
//
//public:
//	ReactiveWStringProperty& PropPath() { return m_path; }
//	CComPtr<ID2D1Bitmap>& GetBitmapPtr() { return m_pBitmap; }
//
//	CD2DImage(CDirect2DWrite* pDirect, const std::wstring& path)
//		:m_pDirect(pDirect), m_path(path){}
//
//	void Open(const std::wstring& path);
//	void Save(const std::wstring& path);
//
//};