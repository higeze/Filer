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

#include "JsonSerializer.h"


struct XTag;
struct YTag;

	template<typename ...>
	constexpr bool false_v = false;

	struct CPointF :public D2D1_POINT_2F
	{
		struct XTag{};
		struct YTag{};

		CPointF();
		CPointF(FLOAT a, FLOAT b);

		void SetPoint(FLOAT a, FLOAT b);
		void Offset(FLOAT xOffset, FLOAT yOffset);
		void Offset(CPointF& pt);
		CPointF operator -() const;
		CPointF operator +(CPointF pt) const;
		CPointF operator -(CPointF pt) const;
		CPointF& operator +=(const CPointF& pt);
		bool operator!=(const CPointF& pt)const;

		template<typename TXY>
		FLOAT Get() const
		{
			if constexpr (std::is_same_v<TXY, XTag>) {
				return x;
			} else if constexpr (std::is_same_v<TXY, YTag>) {
				return y;
			} else {
				static_assert(false_v<TXY>);
			}
		}

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("x", x);
			ar("y", y);
		}

		friend void to_json(json& j, const CPointF& o);
		friend void from_json(const json& j, CPointF& o);
	};

	void to_json(json& j, const CPointF& o)
	{
		j = json{
			{"x", o.x},
			{"y", o.y}
		};
	}
	void from_json(const json& j, CPointF& o)
	{
		j.at("x").get_to(o.x);
		j.at("y").get_to(o.y);
	}

	struct CSizeF :public D2D1_SIZE_F
	{
		CSizeF();
		CSizeF(FLOAT w, FLOAT h);
		CSizeF(const D2D1_SIZE_F& size);
		bool operator==(const CSizeF& rhs) const;
		bool operator!=(const CSizeF& rhs) const;
		std::size_t GetHashCode() const;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("width", width);
			ar("height", height);
		}

		friend void to_json(json& j, const CSizeF& o);
		friend void from_json(const json& j, CSizeF& o);
	};
	void to_json(json& j, const CSizeF& o)
	{
		j = json{
			{"width", o.width},
			{"height", o.height}
		};
	}
	void from_json(const json& j, CSizeF& o)
	{
		j.at("width").get_to(o.width);
		j.at("height").get_to(o.height);
	}


	struct CRectF :public D2D1_RECT_F
	{
		CRectF();
		CRectF(FLOAT l, FLOAT t, FLOAT r, FLOAT b);
		CRectF(const CSizeF& size);

		void SetRect(FLOAT l, FLOAT t, FLOAT r, FLOAT b);
		void MoveToX(FLOAT x);
		void MoveToY(FLOAT y);
		void MoveToXY(FLOAT x, FLOAT y);
		void MoveToXY(const CPointF& pt);
		void OffsetRect(FLOAT x, FLOAT y);
		void OffsetRect(const CPointF& pt);
		FLOAT Width()const;
		FLOAT Height()const;
		void InflateRect(FLOAT x, FLOAT y);
		void DeflateRect(FLOAT x, FLOAT y);
		void InflateRect(FLOAT x);
		void DeflateRect(FLOAT x);
		void InflateRect(const CRectF& rc);
		void DeflateRect(const CRectF& rc);
		bool PtInRect(const CPointF& pt) const;
		CRectF IntersectRect(const CRectF& rc) const;
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

		friend void to_json(json& j, const CRectF& o);
		friend void from_json(const json& j, CRectF& o);
	};
	void to_json(json& j, const CRectF& o)
	{
		j = json{
			{"left", o.left},
			{"top", o.top},
			{"right", o.right},
			{"bottom", o.bottom}
		};
	}
	void from_json(const json& j, CRectF& o)
	{
		j.at("left").get_to(o.left);
		j.at("top").get_to(o.top);
		j.at("right").get_to(o.right);
		j.at("bottom").get_to(o.bottom);
	}


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

		friend void to_json(json& j, const CColorF& o);
		friend void from_json(const json& j, CColorF& o);
	};

	void to_json(json& j, const CColorF& o)
	{
		j = json{
			{"r", o.r},
			{"g", o.g},
			{"b", o.b},
			{"a", o.a}
		};
	}
	void from_json(const json& j, CColorF& o)
	{
		j.at("r").get_to(o.r);
		j.at("g").get_to(o.g);
		j.at("b").get_to(o.b);
		j.at("a").get_to(o.a);
	}

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

		friend void to_json(json& j, const CFontF& o);
		friend void from_json(const json& j, CFontF& o);
	};
	void to_json(json& j, const CFontF& o)
	{
		j = json{
			{"FamilyName", o.FamilyName},
			{"Size", o.Size}
		};
	}
	void from_json(const json& j, CFontF& o)
	{
		j.at("FamilyName").get_to(o.FamilyName);
		j.at("Size").get_to(o.Size);
	}


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
		friend void to_json(json& j, const CAlignmentF& o);
		friend void from_json(const json& j, CAlignmentF& o);
	};
	void to_json(json& j, const CAlignmentF& o)
	{
		j = json{
			{"TextAlignment", o.TextAlignment},
			{"ParagraphAlignment", o.ParagraphAlignment}
		};
	}
	void from_json(const json& j, CAlignmentF& o)
	{
		j.at("TextAlignment").get_to(o.TextAlignment);
		j.at("ParagraphAlignment").get_to(o.ParagraphAlignment);
	}

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
		friend void to_json(json& j, const SolidLine& o);
		friend void from_json(const json& j, SolidLine& o);
	};
	void to_json(json& j, const SolidLine& o)
	{
		j = json{
			{"Color", o.Color},
			{"Width", o.Width}
		};
	}
	void from_json(const json& j, SolidLine& o)
	{
		j.at("Color").get_to(o.Color);
		j.at("Width").get_to(o.Width);
	}


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

		friend void to_json(json& j, const SolidFill& o);
		friend void from_json(const json& j, SolidFill& o);
	};
	void to_json(json& j, const SolidFill& o)
	{
		j = json{
			{"Color", o.Color}
		};
	}
	void from_json(const json& j, SolidFill& o)
	{
		j.at("Color").get_to(o.Color);
	}


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

		friend void to_json(json& j, const FormatF& o);
		friend void from_json(const json& j, FormatF& o);
	};
	void to_json(json& j, const FormatF& o)
	{
		j = json{
			{"Font", o.Font},
			{"Color", o.Color},
			{"Alignment", o.Alignment}
		};
	}
	void from_json(const json& j, FormatF& o)
	{
		j.at("Font").get_to(o.Font);
		j.at("Color").get_to(o.Color);
		j.at("Alignment").get_to(o.Alignment);
	}

	struct SyntaxFormatF
	{
		CColorF Color;
		bool IsBold;
		SyntaxFormatF()
			:Color(), IsBold(false){}
		SyntaxFormatF(const CColorF& color, bool isBold)
			:Color(color), IsBold(isBold){}

		bool operator==(const SyntaxFormatF& rhs) const;
		bool operator!=(const SyntaxFormatF& rhs) const;
		std::size_t GetHashCode() const;

		template <class Archive>
		void serialize(Archive& ar)
		{
			ar("Color", Color);
			ar("IsBold", IsBold);
		}

		friend void to_json(json& j, const SyntaxFormatF& o);
		friend void from_json(const json& j, SyntaxFormatF& o);
	};
	void to_json(json& j, const SyntaxFormatF& o)
	{
		j = json{
			{"Color", o.Color},
			{"IsBold", o.IsBold}
		};
	}
	void from_json(const json& j, SyntaxFormatF& o)
	{
		j.at("Color").get_to(o.Color);
		j.at("IsBold").get_to(o.IsBold);
	}

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

	class CFileIconCache;

	class CDirect2DWrite
	{
	private:
		HWND m_hWnd;
		std::unordered_map<CColorF, CComPtr<ID2D1SolidColorBrush>> m_solidColorBrushMap;
		std::unordered_map<FormatF, CComPtr<IDWriteTextFormat>> m_textFormatMap;
		//std::unordered_map<FormatF, std::unordered_map<wchar_t, CSizeF>> m_charMap;
		std::unordered_map<FormatF, std::unordered_map<std::pair<std::wstring, CSizeF>, CComPtr<IDWriteTextLayout>, StrSizeHash, StrSizeEqual>> m_textLayoutMap;
		std::unordered_map<FormatF, FLOAT> m_defaultHeightMap;

		FLOAT m_xPixels2Dips = 0.0f;
		FLOAT m_yPixels2Dips = 0.0f;

		std::unique_ptr<CFileIconCache> m_pIconCache;//TODO Create only when necessary

	public:
		CDirect2DWrite(HWND hWnd);
		~CDirect2DWrite();
		CDirect2DWrite(const CDirect2DWrite&) = delete;
		CDirect2DWrite& operator=(const CDirect2DWrite&) = delete;

		std::unique_ptr<CFileIconCache>& GetIconCachePtr() { return m_pIconCache; }
		
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

		void BeginDraw();
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
		void DrawTextInRect(const FormatF& fnc, const std::wstring& text, const CRectF& rect);

		void DrawSolidRectangle(const SolidLine& line, const D2D1_RECT_F& rect);
		void FillSolidRectangle(const SolidFill& fill, const D2D1_RECT_F& rect);
		void DrawSolidRoundedRectangle(const SolidLine& line, const D2D1_RECT_F& rect, FLOAT radiusX, FLOAT radiusY);
		void FillSolidRoundedRectangle(const SolidFill& fill, const D2D1_RECT_F& rect, FLOAT radiusX, FLOAT radiusY);
		void DrawSolidGeometry(const SolidLine& line, CComPtr<ID2D1PathGeometry>& path);
		void FillSolidGeometry(const SolidFill& fill, CComPtr<ID2D1PathGeometry>& path);
		void DrawIcon(HICON hIcon, CRectF& rect);
		void DrawBitmap(const CComPtr<ID2D1Bitmap>& pBitmap, const CRectF& rect);
		void DrawBitmap(const CComPtr<ID2D1Bitmap1>& pBitmap, const CRectF& rect);

		CSizeF CalcTextSize(const FormatF& format, const std::wstring& text);
		CSizeF CalcTextSizeWithFixedWidth(const FormatF& fca, const std::wstring& text, const FLOAT width);

		std::vector<CRectF> CalcCharRects(const FormatF& format, const std::wstring& text, const CSizeF& size);
		static std::vector<CRectF> CalcCharRects(const CComPtr<IDWriteTextLayout>& pTextLayout, const size_t& count);
		static std::vector<CRectF> CalcCharRects(const CComPtr<IDWriteTextLayout1>& pTextLayout, const size_t& count);
		void PushAxisAlignedClip(const CRectF& clipRect, D2D1_ANTIALIAS_MODE antialiasMode);
		void PopAxisAlignedClip();

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
			return CRectF{ LayoutRound(rc.left), LayoutRound(rc.top), LayoutRound(rc.right), LayoutRound(rc.bottom) };
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
