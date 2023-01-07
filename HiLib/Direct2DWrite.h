#pragma once

#include <d2d1_1.h>
#include <D2d1_1helper.h>
#pragma comment (lib, "D2d1.lib")
#include <dwrite_1.h>
#pragma comment (lib, "dwrite.lib")
#include <d3d11_1.h>
#pragma comment (lib, "d3d11.lib")

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
#include "Debug.h"
#include <fmt/format.h>
#include "D2DWTypes.h"
#include "JsonSerializer.h"

class CD2DFileIconDrawer;
class CD2DThumbnailDrawer;

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

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(CColorF,
			r,
			g,
			b,
			a)
	};

namespace std
{
	template <>
	struct hash<CSizeF>
	{
		std::size_t operator() (CSizeF const & key) const
		{
			return key.GetHashCode();
		}
	};
}


namespace std
{
	template <>
	struct hash<CColorF>
	{
		std::size_t operator() (CColorF const & key) const
		{
			return key.GetHashCode();
		}
	};
}
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

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(CFontF,
			FamilyName,
			Size)
	};

namespace std
{
	template <>
	struct hash<CFontF>
	{
		std::size_t operator() (CFontF const & key) const
		{
			return key.GetHashCode();
		}
	};
}

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

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(CAlignmentF,
			TextAlignment,
			ParagraphAlignment)
	};

namespace std
{
	template <>
	struct hash<CAlignmentF>
	{
		std::size_t operator() (CAlignmentF const & key) const
		{
			return key.GetHashCode();
		}
	};
}

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
		NLOHMANN_DEFINE_TYPE_INTRUSIVE(SolidLine,
			Color,
			Width)
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

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(SolidFill,
			Color)
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

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(FormatF,
			Font,
			Color,
			Alignment)
	};

	struct SyntaxFormatF
	{
		CColorF Color;
		bool IsBold;
		bool IsUnderline;
		SyntaxFormatF()
			:Color(), IsBold(false), IsUnderline(false){}
		SyntaxFormatF(const CColorF& color, bool isBold, bool isUnderline)
			:Color(color), IsBold(isBold), IsUnderline(isUnderline){}

		bool operator==(const SyntaxFormatF& rhs) const;
		bool operator!=(const SyntaxFormatF& rhs) const;
		std::size_t GetHashCode() const;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("Color", Color);
			ar("IsBold", IsBold);
			ar("IsUnderline", IsUnderline);
		}

		NLOHMANN_DEFINE_TYPE_INTRUSIVE(SyntaxFormatF,
			Color,
			IsBold,
			IsUnderline)
	};

namespace std
{
	template <>
	struct hash<FormatF>
	{
		std::size_t operator() (FormatF const & key) const
		{
			return key.GetHashCode();
		}
	};
}




	struct StrSizeHash
	{
		inline std::size_t operator()(const std::pair<std::wstring, CSizeF>& key)const
		{
			std::size_t seed = 0;
			boost::hash_combine(seed, std::hash<decltype(key.first)>()(key.first));
			boost::hash_combine(seed, std::hash<decltype(key.second)>()(key.second));
			return seed;
		}
	};

	struct StrSizeEqual
	{
		inline std::size_t operator()(const std::pair<std::wstring, CSizeF>& left, const std::pair<std::wstring, CSizeF>& right)const
		{
			return left.first == right.first && left.second == right.second;
		}
	};

	class CDirect2DWrite
	{
	public:
		HWND m_hWnd;
		HDC m_hDC;
		std::unordered_map<CColorF, CComPtr<ID2D1SolidColorBrush>> m_solidColorBrushMap;
		std::unordered_map<FormatF, CComPtr<IDWriteTextFormat>> m_textFormatMap;
		//std::unordered_map<FormatF, std::unordered_map<wchar_t, CSizeF>> m_charMap;
		std::unordered_map<FormatF, std::unordered_map<std::pair<std::wstring, CSizeF>, CComPtr<IDWriteTextLayout>, StrSizeHash, StrSizeEqual>> m_textLayoutMap;
		std::unordered_map<FormatF, FLOAT> m_defaultHeightMap;
		std::unique_ptr<CD2DFileIconDrawer> m_pIconDrawer;
		std::unique_ptr<CD2DThumbnailDrawer> m_pThumbnailDrawer;

		FLOAT m_xPixels2Dips = 0.0f;
		FLOAT m_yPixels2Dips = 0.0f;

	public:
		CDirect2DWrite(HWND hWnd);
		~CDirect2DWrite();
		CDirect2DWrite(const CDirect2DWrite&) = delete;
		CDirect2DWrite& operator=(const CDirect2DWrite&) = delete;
		
		//Closure
		std::function<CComPtr<ID2D1Device>& ()> GetD2DDevice;
		std::function<CComPtr<ID2D1DeviceContext>& ()> GetD2DDeviceContext;
		std::function<std::tuple<CComPtr<ID3D11Device1>&, CComPtr<ID3D11DeviceContext1>&>()> GetD3DDevices;
		std::function<CComPtr<IDXGIDevice>& ()> GetDXGIDevice;
		std::function<CComPtr<IDXGIAdapter>& ()> GetDXGIAdapter;
		std::function<CComPtr<IDXGIFactory2>&()> GetDXGIFactory;
		
		std::function<CComPtr<IDXGISwapChain1>&()> GetDXGISwapChain;
		std::function<CComPtr<ID2D1Bitmap1>&()> GetD2DBackBuffer;

		std::function<CComPtr<ID2D1Factory1>&()> GetD2DFactory;
		std::function<CComPtr<IDWriteFactory1>&()> GetDWriteFactory;
		std::function<CComPtr<IWICImagingFactory2>&()> GetWICImagingFactory;
		std::unique_ptr<CD2DFileIconDrawer>& GetFileIconDrawerPtr() { return m_pIconDrawer; }
		std::unique_ptr<CD2DThumbnailDrawer>& GetFileThumbnailDrawerPtr() { return m_pThumbnailDrawer; }

		HWND GetHWnd() const { return m_hWnd; }
		HDC GetHDC() const { return m_hDC; }

		void BeginDraw(HDC hDC);
		CComPtr<ID2D1SolidColorBrush>& GetColorBrush(const CColorF& color);
		CComPtr<IDWriteTextFormat>& GetTextFormat(const FormatF& fca);
		CComPtr<IDWriteTextLayout>& GetTextLayout(const FormatF& format, const std::wstring& text, const CSizeF& size);
		FLOAT GetVirtualHeight(const FormatF& format);
		void ClearTextLayoutMap() { m_textLayoutMap.clear(); }
		void ClearSolid(const SolidFill& fill);
		void Resize();
		void EndDraw();

		void DrawSolidLine(const SolidLine& line, const D2D1_POINT_2F& p0, const D2D1_POINT_2F& p1);
		void DrawSolidLine(const CColorF& color, const FLOAT& width, const D2D1_POINT_2F& p0, const D2D1_POINT_2F& p1);
		void DrawSolidTriangleWave(const SolidLine& line, const D2D1_POINT_2F& p0, const D2D1_POINT_2F& p1, const FLOAT& amplitude, const FLOAT& period);
		void DrawTab(const SolidLine& line, const CRectF& rc);
		void DrawLineFeed(const SolidLine& line, const CRectF& rc);
		void DrawHalfSpace(const SolidLine& line, const CRectF& rc);

		void DrawTextLayout(const FormatF& fnc, const std::wstring& text, const D2D1_POINT_2F& origin, const D2D1_SIZE_F& size);
		void DrawTextLayout(const FormatF& fnc, const std::wstring& text, const CRectF& rect);
		void DrawTextFromPoint(const FormatF& format, const std::wstring& text, const CPointF& point);
		void DrawTextInRect(const FormatF& fnc, const std::wstring& text, const CRectF& rect);
		void DrawSolidRectangle(const SolidLine& line, const D2D1_RECT_F& rc);
		void DrawSolidRectangleByLine(const SolidLine& line, const D2D1_RECT_F& rect);
		void FillSolidRectangle(const SolidFill& fill, const D2D1_RECT_F& rect);
		void DrawSolidRoundedRectangle(const SolidLine& line, const D2D1_RECT_F& rect, FLOAT radiusX, FLOAT radiusY);
		void FillSolidRoundedRectangle(const SolidFill& fill, const D2D1_RECT_F& rect, FLOAT radiusX, FLOAT radiusY);
		void DrawSolidGeometry(const SolidLine& line, CComPtr<ID2D1PathGeometry>& path);
		void FillSolidGeometry(const SolidFill& fill, CComPtr<ID2D1PathGeometry>& path);
		void DrawIcon(HICON hIcon, CRectF& rect);
		void DrawBitmap(const CComPtr<ID2D1Bitmap>& pBitmap,
			const CRectF& destinationRectangle,
			FLOAT opacity = 1.0f,
			D2D1_BITMAP_INTERPOLATION_MODE interpolationMode = D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
			const D2D1_RECT_F *sourceRectangle = NULL );
		void DrawBitmap(const CComPtr<ID2D1Bitmap1>& pBitmap, const CRectF& rect);

		CSizeF CalcTextSize(const FormatF& format, const std::wstring& text);
		CSizeF CalcTextSizeWithFixedWidth(const FormatF& fca, const std::wstring& text, const FLOAT width);

		std::vector<CRectF> CalcCharRects(const FormatF& format, const std::wstring& text, const CSizeF& size);
		static std::vector<CRectF> CalcCharRects(const CComPtr<IDWriteTextLayout>& pTextLayout, const size_t& count);
		static std::vector<CRectF> CalcCharRects(const CComPtr<IDWriteTextLayout1>& pTextLayout, const size_t& count);
		void PushAxisAlignedClip(const CRectF& clipRect, D2D1_ANTIALIAS_MODE antialiasMode);
		void PopAxisAlignedClip();

		void SaveBitmap(const std::wstring& dstPath, const CComPtr<ID2D1Bitmap1>& pSrcBitmap) const;

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

		CRectF LayoutRound(const CRectF& rc, FLOAT unit = 0.5f)
		{
			return CRectF( LayoutRound(rc.left), LayoutRound(rc.top), LayoutRound(rc.right), LayoutRound(rc.bottom) );
		}

		FLOAT GetPixels2DipsRatioX();
		FLOAT GetPixels2DipsRatioY();
		FLOAT Pixels2DipsX(int x);
		FLOAT Pixels2DipsY(int y);
		int Dips2PixelsX(FLOAT x);
		int Dips2PixelsY(FLOAT y);
		CPointF Pixels2Dips(CPoint pt);
		CPoint Dips2Pixels(CPointF pt);
		CSizeF Pixels2Dips(CSize sz);
		CSize Dips2Pixels(CSizeF sz);
		CRectF Pixels2Dips(CRect rc);
		CRect Dips2Pixels(CRectF rc);

		static FLOAT Points2Dips(int points)
		{
			return points / 72.0f * 96.0f;
		}

		static int Dips2Points(FLOAT dips)
		{
			return static_cast<int>(std::round(dips * 72.0f / 96.0f));
		}

		void Clear();

	};
