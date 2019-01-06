#pragma once

#include <D2d1.h>
#include <D2d1helper.h>
#pragma comment (lib, "D2d1.lib")
#include <dwrite.h>
#pragma comment (lib, "dwrite.lib")
//#include <wrl/client.h>
//using namespace Microsoft::WRL;
#include "atlcomcli.h"
#include <exception>
#include <unordered_map>
#include <boost/functional/hash.hpp>

#include "MyPoint.h"
#include "MyRect.h"

#define FILELINEFUNCTION (boost::format("File:%1%, Line:%2%, Func:%3%") % ::PathFindFileNameA(__FILE__) % __LINE__ % __FUNCTION__).str().c_str()

struct XTag;
struct YTag;

namespace d2dw
{
	struct CPointF :public D2D1_POINT_2F
	{
		CPointF() :D2D1_POINT_2F{ 0.0f } {}
		CPointF(FLOAT a, FLOAT b) :D2D1_POINT_2F{ a, b } {}
		void SetPoint(FLOAT a, FLOAT b) { x = a; y = b; }
		void Offset(LONG xOffset, LONG yOffset) { x += xOffset; y += yOffset; }
		void Offset(CPointF pt) { x += pt.x; y += pt.y; }
		CPointF operator -() const { return CPointF(-x, -y); }
		CPointF operator +(CPointF pt) const
		{
			return CPointF(x + pt.x, y + pt.y);
		}
		CPointF operator -(CPointF pt) const
		{
			return CPointF(x - pt.x, y - pt.y);
		}
		CPointF& operator +=(const CPointF& pt)
		{
			x += pt.x; y += pt.y; return *this;
		}

		bool operator!=(const CPointF& pt)const
		{
			return (x != pt.x || y != pt.y);
		}

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
		CSizeF(FLOAT w = 0.0f, FLOAT h = 0.0f) :D2D1_SIZE_F{ w, h } {}
		CSizeF(const D2D1_SIZE_F& size)
		{
			width = size.width;
			height = size.height;
		}
		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("width", width);
			ar("height", height);
		}
	};

	struct CRectF :public D2D1_RECT_F
	{
		CRectF() :D2D1_RECT_F{ 0.0f} {}
		CRectF(FLOAT l, FLOAT t, FLOAT r, FLOAT b) :D2D1_RECT_F{ l,t,r,b } {}
		CRectF(CSizeF size) :D2D1_RECT_F{ 0, 0, size.width, size.height } {}

		void SetRect(FLOAT l, FLOAT t, FLOAT r, FLOAT b)
		{
			left = l; top = t; right = r; bottom = b;
		}
		void MoveToX(int x) { right = x + right - left; left = x; }
		void MoveToY(int y) { bottom = y + bottom - top; top = y; }
		void MoveToXY(int x, int y)
		{
			right += x - left;
			bottom += y - top;
			left = x;
			top = y;
		}
		void MoveToXY(CPointF pt)
		{
			MoveToXY(pt.x, pt.y);
		}
		void OffsetRect(FLOAT x, FLOAT y) { left += x; right += x; top += y; bottom += y; }
		void OffsetRect(CPointF pt) { OffsetRect(pt.x, pt.y); }

		FLOAT Width()const { return right - left; }
		FLOAT Height()const { return bottom - top; }

		void InflateRect(FLOAT x, FLOAT y) { left -= x; right += x; top -= y; bottom += y; }
		void DeflateRect(FLOAT x, FLOAT y) { left += x; right -= x; top += y; bottom -= y; }
		bool PtInRect(CPointF pt) const { return pt.x >= left && pt.x <= right && pt.y >= top && pt.y <= bottom; }
		CPointF LeftTop() const { return CPointF(left, top); }
		CPointF CenterPoint() const
		{
			return CPointF((left + right) / 2, (top + bottom) / 2);
		}

		CSizeF Size() const
		{
			return CSizeF(Width(), Height());
		}

		void InflateRect(CRectF rc)
		{
			left -= rc.left;
			top -= rc.top;
			right += rc.right;
			bottom += rc.bottom;
		}
		void DeflateRect(CRectF rc)
		{
			left += rc.left;
			top += rc.top;
			right -= rc.right;
			bottom -= rc.bottom;
		}

		//bool IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2) { return ::IntersectRect(this, lpRect1, lpRect2); }

		CRectF operator+(CRectF rc)const
		{
			CRectF ret(*this);
			ret.InflateRect(rc);
			return ret;
		}
		CRectF& operator+=(CRectF rc)
		{
			this->InflateRect(rc);
			return *this;
		}

		CRectF operator-(CRectF rc)const
		{
			CRectF ret(*this);
			ret.DeflateRect(rc);
			return ret;
		}
		CRectF& operator-=(CRectF rc)
		{
			this->DeflateRect(rc);
			return *this;
		}
		bool operator==(const CRectF& rc)const { return left == rc.left && top == rc.top && right == rc.right && bottom == rc.bottom; }
		bool operator!=(const CRectF& rc)const { return !operator==(rc); }
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

	struct Color :public D2D1::ColorF
	{
	public:
		Color() :D2D1::ColorF(0.0F, 0.0f, 0.0f, 0.0f) {}
		Color(FLOAT r, FLOAT g, FLOAT b, FLOAT a = 1.0f) :D2D1::ColorF(r, g, b, a) {}

		bool operator==(const Color& rhs) const
		{
			return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.b;
		}

		bool operator!=(const Color& rhs) const
		{
			return !operator==(rhs);
		}

		std::size_t GetHashCode() const
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, std::hash<decltype(r)>()(r));
			boost::hash_combine(seed, std::hash<decltype(g)>()(g));
			boost::hash_combine(seed, std::hash<decltype(b)>()(b));
			boost::hash_combine(seed, std::hash<decltype(a)>()(a));
			return seed;
		}

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
	struct hash<d2dw::Color>
	{
		std::size_t operator() (d2dw::Color const & key) const
		{
			return key.GetHashCode();
		}
	};
}

namespace d2dw
{
	struct SolidLine
	{
	public:
		SolidLine(FLOAT r, FLOAT g, FLOAT b, FLOAT a, FLOAT w)
			:Color(r, g, b, a), Width(w){ }
		Color Color;
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
		SolidFill(FLOAT r, FLOAT g, FLOAT b, FLOAT a)
			:Color(r, g, b, a){}
		Color Color;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("Color", Color);
		}
	};
}

namespace d2dw
{
	struct Font
	{
	public:
		std::wstring FamilyName;
		FLOAT Size;

		bool operator==(const Font& rhs) const
		{
			return FamilyName == rhs.FamilyName && Size == rhs.Size;
		}

		bool operator!=(const Font& rhs) const
		{
			return !operator==(rhs);
		}

		std::size_t GetHashCode() const
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, std::hash<decltype(FamilyName)>()(FamilyName));
			boost::hash_combine(seed, std::hash<decltype(Size)>()(Size));
			return seed;
		}

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
	struct hash<d2dw::Font>
	{
		std::size_t operator() (d2dw::Font const & key) const
		{
			return key.GetHashCode();
		}
	};
}

namespace d2dw
{
	struct FontAndColor
	{
	public:
		FontAndColor(const std::wstring& familyName, const FLOAT size, FLOAT r, FLOAT g, FLOAT b, FLOAT a)
			:Font{familyName, size}, Color(r, g, b, a){}
		Font Font;
		Color Color;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("Font", Font);
			ar("Color", Color);
		}

	};

};

namespace d2dw{

	class CDirect2DWrite
	{
	private:
		HWND m_hWnd;
		CComPtr<ID2D1Factory> m_pD2D1Factory = NULL;
		CComPtr<IDWriteFactory> m_pDWriteFactory = NULL;
		CComPtr<ID2D1HwndRenderTarget> m_pHwndRenderTarget = NULL;


		std::unordered_map<Color, CComPtr<ID2D1SolidColorBrush>> m_solidColorBrushMap;
		std::unordered_map<Font, CComPtr<IDWriteTextFormat>> m_textFormatMap;
		std::unordered_map<Font, std::unordered_map<wchar_t, CSizeF>> m_charMap;




	public:
		CDirect2DWrite(HWND hWnd);
		CDirect2DWrite(const CDirect2DWrite&) = delete;
		CDirect2DWrite& operator=(const CDirect2DWrite&) = delete;

		CComPtr<ID2D1Factory>& GetD2D1Factory();
		CComPtr<IDWriteFactory>& GetDWriteFactory();
		CComPtr<ID2D1HwndRenderTarget>& GetHwndRenderTarget();
		CComPtr<ID2D1SolidColorBrush>& GetColorBrush(const Color& color);
		CComPtr<IDWriteTextFormat>& GetTextFormat(const Font& font);

		void BeginDraw();
		void ClearSolid(const SolidFill& fill);
		void EndDraw();

		void DrawSolidLine(const SolidLine& line, const D2D1_POINT_2F& p0, const D2D1_POINT_2F& p1);
		void DrawTextLayout(const FontAndColor& fnc, const std::wstring& text, const D2D1_POINT_2F& origin, const D2D1_SIZE_F& size);
		void DrawTextLayout(const FontAndColor& fnc, const std::wstring& text, const CRectF& rect);
		void DrawSolidRectangle(const SolidLine& line, const D2D1_RECT_F& rect);
		void FillSolidRectangle(const SolidFill& fill, const D2D1_RECT_F& rect);

		CSizeF CalcTextSize(const Font& font, const std::wstring& text);
		CSizeF CalcTextSizeWithFixedWidth(const Font& font, const std::wstring& text, const FLOAT width);

		FLOAT Pixels2DipsX(int x)
		{
			CPointF dpi(96.0f, 96.0f);
			GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
			return x * 96.0f / dpi.x;
		}

		FLOAT Pixels2DipsY(int y)
		{
			CPointF dpi(96.0f, 96.0f);
			GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
			return y * 96.0f / dpi.y;
		}

		int Dips2PixelsX(FLOAT x)
		{
			CPointF dpi(96.0f, 96.0f);
			GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
			return x * dpi.x / 96.0f;
		}

		int Dips2PixelsY(FLOAT y)
		{
			CPointF dpi(96.0f, 96.0f);
			GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
			return y * dpi.y / 96.0f;
		}

		CPointF Pixels2Dips(CPoint pt)
		{
			CPointF dpi(96.0f, 96.0f);
			GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
			return CPointF(pt.x * 96.0f / dpi.x, pt.y * 96.0f / dpi.y);
		}

		CPoint Dips2Pixels(CPointF pt)
		{
			CPointF dpi(96.0f, 96.0f);
			GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
			return CPoint(pt.x * dpi.x / 96.0f, pt.y * dpi.y / 96.0f);
		}

		CRectF Pixels2Dips(CRect rc)
		{
			CPointF dpi(96.0f, 96.0f);
			GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
			return CRectF(rc.left * 96.0f / dpi.x, rc.top * 96.0f / dpi.y, rc.right * 96.0f / dpi.x, rc.bottom * 96.0f / dpi.y);
		}

		CRect Dips2Pixels(CRectF rc)
		{
			CPointF dpi(96.0f, 96.0f);
			GetD2D1Factory()->GetDesktopDpi(&dpi.x, &dpi.y);
			return CRect(rc.left * dpi.x / 96.0f, rc.top * dpi.y / 96.0f, rc.right * dpi.x / 96.0f, rc.bottom * dpi.y / 96.0f);
		}

		FLOAT Points2Dips(int points)
		{
			return points / 72.0f * 96.0f;
		}

		int Dips2Points(FLOAT dips)
		{
			return dips * 72.0f / 96.0f;
		}

		void Clear()
		{
			m_pD2D1Factory = nullptr;
			m_pDWriteFactory = nullptr;
			m_pHwndRenderTarget = nullptr;

			m_solidColorBrushMap.clear();
			m_textFormatMap.clear();
			m_charMap.clear();
		}

	};
}

