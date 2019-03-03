#pragma once

#include <d2d1_1.h>
#include <D2d1_1helper.h>
#pragma comment (lib, "D2d1.lib")
#include <dwrite_1.h>
#pragma comment (lib, "dwrite.lib")
#include <wincodec.h>
//#include <wrl/client.h>
//using namespace Microsoft::WRL;
#include "atlcomcli.h"
#include <exception>
#include <unordered_map>
#include <boost/functional/hash.hpp>

#include "MyPoint.h"
#include "MyRect.h"
#include "MyFont.h"

#define FILELINEFUNCTION (boost::format("File:%1%, Line:%2%, Func:%3%") % ::PathFindFileNameA(__FILE__) % __LINE__ % __FUNCTION__).str().c_str()

#define USE_ID2D1DCRenderTarget

struct XTag;
struct YTag;

namespace d2dw
{
	struct CPointF :public D2D1_POINT_2F
	{
		CPointF();
		CPointF(FLOAT a, FLOAT b);

		void SetPoint(FLOAT a, FLOAT b);
		void Offset(LONG xOffset, LONG yOffset);
		void Offset(CPointF& pt);
		CPointF operator -() const;
		CPointF operator +(CPointF pt) const;
		CPointF operator -(CPointF pt) const;
		CPointF& operator +=(const CPointF& pt);
		bool operator!=(const CPointF& pt)const;

		template<typename TXY>
		FLOAT Get()const { return 0; }

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("x", x);
			ar("y", y);
		}
	};

	template<> inline FLOAT CPointF::Get<XTag>()const { return x; }
	template<> inline FLOAT CPointF::Get<YTag>()const { return y; }

	struct CSizeF :public D2D1_SIZE_F
	{
		CSizeF();
		CSizeF(FLOAT w, FLOAT h);
		CSizeF(const D2D1_SIZE_F& size);

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("width", width);
			ar("height", height);
		}
	};

	struct CRectF :public D2D1_RECT_F
	{
		CRectF();
		CRectF(FLOAT l, FLOAT t, FLOAT r, FLOAT b);
		CRectF(const CSizeF& size);

		void SetRect(FLOAT l, FLOAT t, FLOAT r, FLOAT b);
		void MoveToX(int x);
		void MoveToY(int y);
		void MoveToXY(int x, int y);
		void MoveToXY(const CPointF& pt);
		void OffsetRect(FLOAT x, FLOAT y);
		void OffsetRect(const CPointF& pt);
		FLOAT Width()const;
		FLOAT Height()const;
		void InflateRect(FLOAT x, FLOAT y);
		void DeflateRect(FLOAT x, FLOAT y);
		void InflateRect(const CRectF& rc);
		void DeflateRect(const CRectF& rc);
		bool PtInRect(const CPointF& pt) const;
		CPointF LeftTop() const;
		CPointF CenterPoint() const;
		CSizeF Size() const;
		//bool IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2) { return ::IntersectRect(this, lpRect1, lpRect2); }
		CRectF operator+(CRectF rc)const;
		CRectF& operator+=(CRectF rc);
		CRectF operator-(CRectF rc)const;
		CRectF& operator-=(CRectF rc);
		bool operator==(const CRectF& rc)const;
		bool operator!=(const CRectF& rc)const;
		//void operator &=(const CRectF& rect) { ::IntersectRect(this, this, &rect); }
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("left", left);
			ar("top", top);
			ar("right", right);
			ar("bottom", bottom);
		}
	};

	struct CColorF :public D2D1::ColorF
	{
	public:
		CColorF();
		CColorF(FLOAT r, FLOAT g, FLOAT b, FLOAT a = 1.0f);

		bool operator==(const CColorF& rhs) const;
		bool operator!=(const CColorF& rhs) const;
		std::size_t GetHashCode() const;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("r", r);
			ar("g", g);
			ar("b", b);
			ar("a", a);
		}
	};
}

namespace std
{
	template <>
	struct hash<d2dw::CColorF>
	{
		std::size_t operator() (d2dw::CColorF const & key) const
		{
//			BOOST_LOG_TRIVIAL(trace) << L"Color" << key.GetHashCode();
			return key.GetHashCode();
		}
	};
}

namespace d2dw
{
	struct CFontF
	{
	public:
		CFontF(const std::wstring& familyName = L"Meiryo UI", FLOAT size = 12.0f);

		std::wstring FamilyName;
		FLOAT Size;

		bool operator==(const CFontF& rhs) const;
		bool operator!=(const CFontF& rhs) const;
		std::size_t GetHashCode() const;

		CFont GetGDIFont() const;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("FamilyName", FamilyName);
			ar("Size", Size);
		}
	};
}

namespace std
{
	template <>
	struct hash<d2dw::CFontF>
	{
		std::size_t operator() (d2dw::CFontF const & key) const
		{
			//			BOOST_LOG_TRIVIAL(trace) << L"Font" <<key.GetHashCode();
			return key.GetHashCode();
		}
	};
}

namespace d2dw
{
	struct CAlignmentF
	{
	public:
		CAlignmentF(const DWRITE_TEXT_ALIGNMENT& textAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING,
					const DWRITE_PARAGRAPH_ALIGNMENT& paragraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_NEAR);

		DWRITE_TEXT_ALIGNMENT TextAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_LEADING;
		DWRITE_PARAGRAPH_ALIGNMENT ParagraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_NEAR;

		bool operator==(const CAlignmentF& rhs) const;
		bool operator!=(const CAlignmentF& rhs) const;
		std::size_t GetHashCode() const;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("TextAlignment", TextAlignment);
			ar("ParagraphAlignment", ParagraphAlignment);
		}
	};

}

namespace std
{
	template <>
	struct hash<d2dw::CAlignmentF>
	{
		std::size_t operator() (d2dw::CAlignmentF const & key) const
		{
			//			BOOST_LOG_TRIVIAL(trace) << L"Font" <<key.GetHashCode();
			return key.GetHashCode();
		}
	};
}



namespace d2dw
{
	struct SolidLine
	{
	public:
		SolidLine(FLOAT r = 1.0f, FLOAT g = 1.0f, FLOAT b = 1.0f, FLOAT a = 1.0f, FLOAT w = 1.0f)
			:Color(r, g, b, a), Width(w){ }
		CColorF Color;
		FLOAT Width;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("Color", Color);
			ar("Width", Width);
		}
	};

	struct SolidFill
	{
	public:
		SolidFill(const CColorF& color)
			:Color(color){}
		SolidFill(const FLOAT r = 1.0f, const FLOAT g = 1.0f, const FLOAT b = 1.0f, const FLOAT a = 1.0f)
			:Color(r, g, b, a){}

		CColorF Color;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("Color", Color);
		}
	};

	struct FormatF
	{
	public:
		FormatF(const CFontF& font, const CColorF& color, const CAlignmentF& alignment )
			:Font(font), Color(color), Alignment(alignment){}
		FormatF(const std::wstring& familyName = L"Meiryo", const FLOAT size = 9.0f, FLOAT r = 1.0f, FLOAT g = 1.0f, FLOAT b = 1.0f, FLOAT a = 1.0f)
			:Font{familyName, size}, Color(r, g, b, a), Alignment(){}
		CFontF Font;
		CColorF Color;
		CAlignmentF Alignment;

		bool operator==(const FormatF& rhs) const;
		bool operator!=(const FormatF& rhs) const;
		std::size_t GetHashCode() const;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("Font", Font);
			ar("Color", Color);
			ar("Alignment", Alignment);
		}

	};

};

namespace std
{
	template <>
	struct hash<d2dw::FormatF>
	{
		std::size_t operator() (d2dw::FormatF const & key) const
		{
			//			BOOST_LOG_TRIVIAL(trace) << L"Font" <<key.GetHashCode();
			return key.GetHashCode();
		}
	};
}


namespace d2dw{

	class CDirect2DWrite
	{
	private:
		HWND m_hWnd;
		CComPtr<ID2D1Factory1> m_pD2D1Factory = NULL;
		CComPtr<IDWriteFactory1> m_pDWriteFactory = NULL;

#ifdef USE_ID2D1DCRenderTarget
		CComPtr<ID2D1DCRenderTarget> m_pHwndRenderTarget = NULL;
		HDC m_hDC;
		std::vector<std::function<void()>> m_gdifuncs;
#else
		CComPtr<ID2D1HwndRenderTarget> m_pHwndRenderTarget = NULL;
#endif
		CComPtr<IWICImagingFactory> m_pWICImagingFactory = NULL;
		//CComPtr<IWICFormatConverter> m_pWICFormatConverter = NULL;


		std::unordered_map<CColorF, CComPtr<ID2D1SolidColorBrush>> m_solidColorBrushMap;
		std::unordered_map<FormatF, CComPtr<IDWriteTextFormat>> m_textFormatMap;
		std::unordered_map<FormatF, std::unordered_map<wchar_t, CSizeF>> m_charMap;

		FLOAT m_xPixels2Dips = 0.0f;
		FLOAT m_yPixels2Dips = 0.0f;

	public:
		CDirect2DWrite(HWND hWnd);
		CDirect2DWrite(const CDirect2DWrite&) = delete;
		CDirect2DWrite& operator=(const CDirect2DWrite&) = delete;

		CComPtr<ID2D1Factory1>& GetD2D1Factory();
		CComPtr<IDWriteFactory1>& GetDWriteFactory();
		CComPtr<IWICImagingFactory>& GetWICImagingFactory();
		//CComPtr<IWICFormatConverter>& GetWICFormatConverter();
#ifdef USE_ID2D1DCRenderTarget
		CComPtr<ID2D1DCRenderTarget>& GetHwndRenderTarget();
#else
		CComPtr<ID2D1HwndRenderTarget>& GetHwndRenderTarget();
#endif
		CComPtr<ID2D1SolidColorBrush>& GetColorBrush(const CColorF& color);
		CComPtr<IDWriteTextFormat>& GetTextFormat(const FormatF& fca);
#ifdef USE_ID2D1DCRenderTarget
		void BeginDraw(HDC hDC, const RECT& rc);
#else
		void BeginDraw();

#endif

		void ClearSolid(const SolidFill& fill);
		void EndDraw();

		void DrawSolidLine(const SolidLine& line, const D2D1_POINT_2F& p0, const D2D1_POINT_2F& p1);
		void DrawTextLayout(const FormatF& fnc, const std::wstring& text, const D2D1_POINT_2F& origin, const D2D1_SIZE_F& size);
		void DrawTextLayout(const FormatF& fnc, const std::wstring& text, const CRectF& rect);
		void DrawTextInRect(const FormatF& fnc, const std::wstring& text, const CRectF& rect);

		void DrawSolidRectangle(const SolidLine& line, const D2D1_RECT_F& rect);
		void FillSolidRectangle(const SolidFill& fill, const D2D1_RECT_F& rect);
		void DrawSolidGeometry(const SolidLine& line, CComPtr<ID2D1PathGeometry>& path);
		void FillSolidGeometry(const SolidFill& fill, CComPtr<ID2D1PathGeometry>& path);
		void DrawIcon(HICON hIcon, d2dw::CRectF& rect);

		CSizeF CalcTextSize(const FormatF& format, const std::wstring& text);
		CSizeF CalcTextSizeWithFixedWidth(const FormatF& fca, const std::wstring& text, const FLOAT width);

		FLOAT LayoutRound(FLOAT value, FLOAT unit = 0.5f)
		{
			auto ret = static_cast<int>(std::round(value / unit));
			return (ret % 2 == 0) ? (ret - 1)*unit : ret*unit;
		}

		D2D1_POINT_2F LayoutRound(const D2D1_POINT_2F& pt, FLOAT unit = 0.5f)
		{
			return D2D1_POINT_2F{ LayoutRound(pt.x), LayoutRound(pt.y) };
		}

		D2D1_RECT_F LayoutRound(const D2D1_RECT_F& rc, FLOAT unit = 0.5f)
		{
			return D2D1_RECT_F{ LayoutRound(rc.left), LayoutRound(rc.top), LayoutRound(rc.right), LayoutRound(rc.bottom) };
		}


		FLOAT Pixels2DipsX(int x)
		{
			if (m_xPixels2Dips == 0.0f || m_yPixels2Dips == 0.0f) {
				CPointF dpi(96.0f, 96.0f);
				GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
				m_xPixels2Dips = 96.0f / dpi.x;
				m_yPixels2Dips = 96.0f / dpi.y;
			}
			return x * m_xPixels2Dips;
		}

		FLOAT Pixels2DipsY(int y)
		{
			if (m_xPixels2Dips == 0.0f || m_yPixels2Dips == 0.0f) {
				CPointF dpi(96.0f, 96.0f);
				GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
				m_xPixels2Dips = 96.0f / dpi.x;
				m_yPixels2Dips = 96.0f / dpi.y;
			}
			return y * m_yPixels2Dips;
		}

		int Dips2PixelsX(FLOAT x)
		{
			if (m_xPixels2Dips == 0.0f || m_yPixels2Dips == 0.0f) {
				CPointF dpi(96.0f, 96.0f);
				GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
				m_xPixels2Dips = 96.0f / dpi.x;
				m_yPixels2Dips = 96.0f / dpi.y;
			}
			return x / m_xPixels2Dips;
		}

		int Dips2PixelsY(FLOAT y)
		{
			if (m_xPixels2Dips == 0.0f || m_yPixels2Dips == 0.0f) {
				CPointF dpi(96.0f, 96.0f);
				GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
				m_xPixels2Dips = 96.0f / dpi.x;
				m_yPixels2Dips = 96.0f / dpi.y;
			}
			return y / m_yPixels2Dips;
		}

		CPointF Pixels2Dips(CPoint pt)
		{
			return CPointF(Pixels2DipsX(pt.x), Pixels2DipsY(pt.y));
		}

		CPoint Dips2Pixels(CPointF pt)
		{
			return CPoint(Dips2PixelsX(pt.x), Dips2PixelsY(pt.y));
		}

		CRectF Pixels2Dips(CRect rc)
		{
			return CRectF(Pixels2DipsX(rc.left), Pixels2DipsY(rc.top),
						  Pixels2DipsX(rc.right), Pixels2DipsY(rc.bottom));
		}

		CRect Dips2Pixels(CRectF rc)
		{
			return CRect(Dips2PixelsX(rc.left), Dips2PixelsY(rc.top),
				Dips2PixelsX(rc.right), Dips2PixelsY(rc.bottom));
		}

		static FLOAT Points2Dips(int points)
		{
			return points / 72.0f * 96.0f;
		}

		static int Dips2Points(FLOAT dips)
		{
			return dips * 72.0f / 96.0f;
		}

		void Clear()
		{
			m_pD2D1Factory = nullptr;
			m_pDWriteFactory = nullptr;
			m_pWICImagingFactory = nullptr;
			m_pHwndRenderTarget = nullptr;

			m_solidColorBrushMap.clear();
			m_textFormatMap.clear();
			m_charMap.clear();

			m_xPixels2Dips = 0.0f;
			m_yPixels2Dips = 0.0f;
		}

	};
}

