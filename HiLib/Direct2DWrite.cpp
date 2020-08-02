//#include "stdafx.h"
#include "Direct2DWrite.h"
#include "FileIconCache.h"
#include <fmt/format.h>

namespace d2dw
{
	//CPointF
	CPointF::CPointF() :D2D1_POINT_2F{ 0.0f } {}
	CPointF::CPointF(FLOAT a, FLOAT b) :D2D1_POINT_2F{ a, b } {}
	void CPointF::SetPoint(FLOAT a, FLOAT b) { x = a; y = b; }
	void CPointF::Offset(FLOAT xOffset, FLOAT yOffset) { x += xOffset; y += yOffset; }
	void CPointF::Offset(CPointF& pt) { x += pt.x; y += pt.y; }
	CPointF CPointF::operator -() const { return CPointF(-x, -y); }
	CPointF CPointF::operator +(CPointF pt) const
	{
		return CPointF(x + pt.x, y + pt.y);
	}
	CPointF CPointF::operator -(CPointF pt) const
	{
		return CPointF(x - pt.x, y - pt.y);
	}
	CPointF& CPointF::operator +=(const CPointF& pt)
	{
		x += pt.x; y += pt.y; return *this;
	}

	bool CPointF::operator!=(const CPointF& pt)const
	{
		return (x != pt.x || y != pt.y);
	}

	//CSizeF
	CSizeF::CSizeF() :D2D1_SIZE_F{ 0.0f } {}
	CSizeF::CSizeF(FLOAT w, FLOAT h) :D2D1_SIZE_F{ w, h } {}
	CSizeF::CSizeF(const D2D1_SIZE_F& size)
	{
		width = size.width;
		height = size.height;
	}
	bool CSizeF::operator==(const CSizeF& rhs) const
	{
		return width == rhs.width && height == rhs.height;
	}

	bool CSizeF::operator!=(const CSizeF& rhs) const
	{
		return !operator==(rhs);
	}

	std::size_t CSizeF::GetHashCode() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, std::hash<decltype(width)>()(width));
		boost::hash_combine(seed, std::hash<decltype(height)>()(height));
		return seed;
	}

	//CRectF
	CRectF::CRectF() :D2D1_RECT_F{ 0.0f } {}
	CRectF::CRectF(FLOAT l, FLOAT t, FLOAT r, FLOAT b) :D2D1_RECT_F{ l,t,r,b } {}
	CRectF::CRectF(const CSizeF& size) :D2D1_RECT_F{ 0, 0, size.width, size.height } {}
	void CRectF::SetRect(FLOAT l, FLOAT t, FLOAT r, FLOAT b)
	{
		left = l; top = t; right = r; bottom = b;
	}
	void CRectF::MoveToX(FLOAT x) { right = x + right - left; left = x; }
	void CRectF::MoveToY(FLOAT y) { bottom = y + bottom - top; top = y; }
	void CRectF::MoveToXY(FLOAT x, FLOAT y)
	{
		right += x - left;
		bottom += y - top;
		left = x;
		top = y;
	}
	void CRectF::MoveToXY(const CPointF& pt)
	{
		MoveToXY(pt.x, pt.y);
	}
	void CRectF::OffsetRect(FLOAT x, FLOAT y) { left += x; right += x; top += y; bottom += y; }
	void CRectF::OffsetRect(const CPointF& pt) { OffsetRect(pt.x, pt.y); }

	FLOAT CRectF::Width()const { return right - left; }
	FLOAT CRectF::Height()const { return bottom - top; }

	void CRectF::InflateRect(FLOAT x, FLOAT y) { left -= x; right += x; top -= y; bottom += y; }
	void CRectF::DeflateRect(FLOAT x, FLOAT y) { left += x; right -= x; top += y; bottom -= y; }
	void CRectF::InflateRect(FLOAT x) { left -= x; right += x; top -= x; bottom += x; }
	void CRectF::DeflateRect(FLOAT x) { left += x; right -= x; top += x; bottom -= x; }
	bool CRectF::PtInRect(const CPointF& pt) const { return pt.x >= left && pt.x <= right && pt.y >= top && pt.y <= bottom; }
	CPointF CRectF::LeftTop() const { return CPointF(left, top); }
	CPointF CRectF::CenterPoint() const
	{
		return CPointF((left + right) / 2, (top + bottom) / 2);
	}

	CSizeF CRectF::Size() const
	{
		return CSizeF(Width(), Height());
	}

	void CRectF::InflateRect(const CRectF& rc)
	{
		left -= rc.left;
		top -= rc.top;
		right += rc.right;
		bottom += rc.bottom;
	}
	void CRectF::DeflateRect(const CRectF& rc)
	{
		left += rc.left;
		top += rc.top;
		right -= rc.right;
		bottom -= rc.bottom;
	}

	//bool IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2) { return ::IntersectRect(this, lpRect1, lpRect2); }

	CRectF CRectF::operator+(CRectF rc)const
	{
		CRectF ret(*this);
		ret.InflateRect(rc);
		return ret;
	}
	CRectF& CRectF::operator+=(CRectF rc)
	{
		this->InflateRect(rc);
		return *this;
	}

	CRectF CRectF::operator-(CRectF rc)const
	{
		CRectF ret(*this);
		ret.DeflateRect(rc);
		return ret;
	}
	CRectF& CRectF::operator-=(CRectF rc)
	{
		this->DeflateRect(rc);
		return *this;
	}
	bool CRectF::operator==(const CRectF& rc)const { return left == rc.left && top == rc.top && right == rc.right && bottom == rc.bottom; }
	bool CRectF::operator!=(const CRectF& rc)const { return !operator==(rc); }
	//void operator &=(const CRectF& rect) { ::IntersectRect(this, this, &rect); }

	//CColor
	CColorF::CColorF() :D2D1::ColorF(1.0F, 1.0f, 1.0f, 1.0f) {}
	CColorF::CColorF(FLOAT r, FLOAT g, FLOAT b, FLOAT a) :D2D1::ColorF(r, g, b, a) {}

	bool CColorF::operator==(const CColorF& rhs) const
	{
		return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
	}

	bool CColorF::operator!=(const CColorF& rhs) const
	{
		return !operator==(rhs);
	}

	std::size_t CColorF::GetHashCode() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, std::hash<decltype(r)>()(r));
		boost::hash_combine(seed, std::hash<decltype(g)>()(g));
		boost::hash_combine(seed, std::hash<decltype(b)>()(b));
		boost::hash_combine(seed, std::hash<decltype(a)>()(a));
		return seed;
	}

	//CFontF
	CFontF::CFontF(const std::wstring& familyName, FLOAT size)
		:FamilyName(familyName), Size(size){}

	bool CFontF::operator==(const CFontF& rhs) const
	{
		return FamilyName == rhs.FamilyName && Size == rhs.Size;
	}

	bool CFontF::operator!=(const CFontF& rhs) const
	{
		return !operator==(rhs);
	}

	CFont CFontF::GetGDIFont() const
	{
		return CFont(CDirect2DWrite::Dips2Points(Size), FamilyName);
	}

	std::size_t CFontF::GetHashCode() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, std::hash<decltype(FamilyName)>()(FamilyName));
		boost::hash_combine(seed, std::hash<decltype(Size)>()(Size));
		return seed;
	}

	//CAlignmentF
	CAlignmentF::CAlignmentF(const DWRITE_TEXT_ALIGNMENT& textAlignment, const DWRITE_PARAGRAPH_ALIGNMENT& paragraphAlignment)
	:TextAlignment(textAlignment), ParagraphAlignment(paragraphAlignment){}

	bool CAlignmentF::operator==(const CAlignmentF& rhs) const
	{
		return TextAlignment == rhs.TextAlignment && ParagraphAlignment == rhs.ParagraphAlignment;
	}

	bool CAlignmentF::operator!=(const CAlignmentF& rhs) const
	{
		return !operator==(rhs);
	}

	std::size_t CAlignmentF::GetHashCode() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, std::hash<decltype(TextAlignment)>()(TextAlignment));
		boost::hash_combine(seed, std::hash<decltype(ParagraphAlignment)>()(ParagraphAlignment));
		return seed;
	}

	//Format
	bool FormatF::operator==(const FormatF& rhs) const
	{
		return Font == rhs.Font && Color == rhs.Color && Alignment == rhs.Alignment;
	}

	bool FormatF::operator!=(const FormatF& rhs) const
	{
		return !operator==(rhs);
	}

	std::size_t FormatF::GetHashCode() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, std::hash<decltype(Font)>()(Font));
		boost::hash_combine(seed, std::hash<decltype(Color)>()(Color));
		boost::hash_combine(seed, std::hash<decltype(Alignment)>()(Alignment));

		return seed;
	}






	//CDirect2DWrite
	CDirect2DWrite::CDirect2DWrite(HWND hWnd) :m_hWnd(hWnd), m_pIconCache(std::make_unique<CFileIconCache>(this)) {}
	CDirect2DWrite::~CDirect2DWrite() = default;


	CComPtr<ID2D1Factory1>& CDirect2DWrite::GetD2D1Factory()
	{
		if (!m_pD2D1Factory) {
			if (FAILED(
				::D2D1CreateFactory(
					D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_MULTI_THREADED,
					__uuidof(ID2D1Factory1),
					(void**)&m_pD2D1Factory))) {
				throw std::exception(FILE_LINE_FUNC);
			}
		}
		return m_pD2D1Factory;
	}

	CComPtr<IDWriteFactory1>& CDirect2DWrite::GetDWriteFactory()
	{
		if (!m_pDWriteFactory) {
			if (FAILED(
				::DWriteCreateFactory(
					DWRITE_FACTORY_TYPE::DWRITE_FACTORY_TYPE_SHARED,
					__uuidof(IDWriteFactory1),
					(IUnknown**)&m_pDWriteFactory))) {
				throw std::exception(FILE_LINE_FUNC);
			}
		}
		return m_pDWriteFactory;
	}

	CComPtr<IWICImagingFactory>& CDirect2DWrite::GetWICImagingFactory()
	{
		if (!m_pWICImagingFactory) {
			if (FAILED(
				m_pWICImagingFactory.CoCreateInstance(
					CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER))) {
				throw std::exception(FILE_LINE_FUNC);
			}
		}
		return m_pWICImagingFactory;
	}


	CComPtr<ID2D1HwndRenderTarget>& CDirect2DWrite::GetHwndRenderTarget()
	{

		if (!m_pHwndRenderTarget) {
			RECT rect;
			::GetClientRect(m_hWnd, &rect);
			D2D1_SIZE_U size = D2D1::Size<UINT>(rect.right, rect.bottom);
			if (FAILED(
				GetD2D1Factory()->CreateHwndRenderTarget(
					D2D1::RenderTargetProperties(),
					//D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_HARDWARE,
					//	D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
					D2D1::HwndRenderTargetProperties(m_hWnd, size),
					&m_pHwndRenderTarget))) {
				throw std::exception(FILE_LINE_FUNC);
			}
		}
		return m_pHwndRenderTarget;
	}

	CComPtr<ID2D1SolidColorBrush>& CDirect2DWrite::GetColorBrush(const CColorF& color)
	{
		auto iter = m_solidColorBrushMap.find(color);
		if (iter != m_solidColorBrushMap.end()) {
			return iter->second;
		} else {
			CComPtr<ID2D1SolidColorBrush> pBrush;
			if (FAILED(GetHwndRenderTarget()->CreateSolidColorBrush(color, &pBrush))) {
				throw std::exception(FILE_LINE_FUNC);
			} else {
				auto ret = m_solidColorBrushMap.emplace(color, pBrush);
				return ret.first->second;
			}
		}
	}

	CComPtr<IDWriteTextFormat>& CDirect2DWrite::GetTextFormat(const FormatF& format)
	{
		auto iter = m_textFormatMap.find(format);
		if (iter != m_textFormatMap.end()) {
			return iter->second;
		} else {
			CComPtr<IDWriteTextFormat> pTextFormat;
			if (FAILED(GetDWriteFactory()->CreateTextFormat(
				format.Font.FamilyName.c_str(),
				nullptr,
				DWRITE_FONT_WEIGHT_REGULAR,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				format.Font.Size,
				L"en-us",
				&pTextFormat))){
				throw std::exception(FILE_LINE_FUNC);
			} else {
				pTextFormat->SetTextAlignment(format.Alignment.TextAlignment);
				pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING::DWRITE_WORD_WRAPPING_CHARACTER);
				pTextFormat->SetParagraphAlignment(format.Alignment.ParagraphAlignment);
				//pTextFormat->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, font.Size, font.Size*0.8f);
				auto ret = m_textFormatMap.emplace(format, pTextFormat);
				return ret.first->second;
			}
		}
	}

	CComPtr<IDWriteTextLayout>& CDirect2DWrite::GetTextLayout(const FormatF& format, const std::wstring& text, const CSizeF& size)
	{
		auto mapIter = m_textLayoutMap.find(format);
		if (mapIter == m_textLayoutMap.end()) {
			m_textLayoutMap.emplace(format, std::unordered_map<std::pair<std::wstring, CSizeF>, CComPtr<IDWriteTextLayout>, StrSizeHash, StrSizeEqual>());
			mapIter = m_textLayoutMap.find(format);
		}

		auto layoutIter = mapIter->second.find(std::make_pair(text, size));
		if (layoutIter != mapIter->second.end()) {
			return layoutIter->second;
		} else {
			CComPtr<IDWriteTextLayout> pTextLayout = NULL;
			if (FAILED(GetDWriteFactory()->CreateTextLayout(text.c_str(), text.size(), GetTextFormat(format), size.width, size.height, &pTextLayout))) {
				throw std::exception(FILE_LINE_FUNC);
			} else {
				CComPtr<IDWriteTypography> typo;
				GetDWriteFactory()->CreateTypography(&typo);

				DWRITE_FONT_FEATURE feature;
				feature.nameTag = DWRITE_FONT_FEATURE_TAG_STANDARD_LIGATURES;
				feature.parameter = 0;
				typo->AddFontFeature(feature);
				DWRITE_TEXT_RANGE range;
				range.startPosition = 0;
				range.length = text.size();
				pTextLayout->SetTypography(typo, range);

				auto ret = mapIter->second.emplace(std::make_pair(text, size), pTextLayout);
				return ret.first->second;
			}
		}
	}

	FLOAT CDirect2DWrite::GetVirtualHeight(const FormatF& format)
	{
		auto iter = m_defaultHeightMap.find(format);
		if (iter != m_defaultHeightMap.end()) {
			return iter->second;
		} else {
			auto ret = m_defaultHeightMap.emplace(format, CalcTextSize(format, L"A").height);
			return ret.first->second;
		}
	}

	CSizeF CDirect2DWrite::CalcTextSize(const FormatF& format, const std::wstring& text)
	{
		CSizeF textSize( 0.0f, 0.0f );
		CComQIPtr<IDWriteTextLayout1> pTextLayout1(GetTextLayout(format, text, CSizeF(FLT_MAX, FLT_MAX)));
		pTextLayout1->SetCharacterSpacing(0.0f, 0.0f, 0.0f, DWRITE_TEXT_RANGE{ 0, text.size() });
		pTextLayout1->SetPairKerning(FALSE, DWRITE_TEXT_RANGE{ 0, text.size() });
		DWRITE_TEXT_METRICS charMetrics;
		pTextLayout1->GetMetrics(&charMetrics);
		textSize.width = charMetrics.width;
		textSize.height = charMetrics.height;

		return textSize;
	}

	CSizeF CDirect2DWrite::CalcTextSizeWithFixedWidth(const FormatF& format, const std::wstring& text, const FLOAT width)
	{
		CSizeF textSize(0.0f, 0.0f);
		CComQIPtr<IDWriteTextLayout1> pTextLayout1(GetTextLayout(format, text, CSizeF(width, FLT_MAX)));
		pTextLayout1->SetCharacterSpacing(0.0f, 0.0f, 0.0f, DWRITE_TEXT_RANGE{ 0, text.size() });
		pTextLayout1->SetPairKerning(FALSE, DWRITE_TEXT_RANGE{ 0, text.size() });
		DWRITE_TEXT_METRICS charMetrics;
		pTextLayout1->GetMetrics(&charMetrics);
		textSize.width = charMetrics.width;
		textSize.height = charMetrics.height;

		return textSize;
	}
	void CDirect2DWrite::BeginDraw()
	{
		GetHwndRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());
		GetHwndRenderTarget()->BeginDraw();

	}

	void CDirect2DWrite::ClearSolid(const SolidFill& fill)
	{
		GetHwndRenderTarget()->Clear(fill.Color);
	}

	void CDirect2DWrite::EndDraw()
	{
		HRESULT hr = GetHwndRenderTarget()->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
			Clear();
		}
	}


	void CDirect2DWrite::DrawSolidLine(const SolidLine& line, const D2D1_POINT_2F& p0, const D2D1_POINT_2F& p1)
	{
		GetHwndRenderTarget()->DrawLine(LayoutRound(p0), LayoutRound(p1), GetColorBrush(line.Color),line.Width);
	}

	void CDirect2DWrite::DrawTextLayout(const FormatF& format, const std::wstring& text, const D2D1_POINT_2F& origin, const D2D1_SIZE_F& size)
	{
		GetHwndRenderTarget()->DrawTextLayout(origin, GetTextLayout(format, text, size), GetColorBrush(format.Color), D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP);
	}

	void CDirect2DWrite::DrawTextLayout(const FormatF& format, const std::wstring& text, const CRectF& rect)
	{
		DrawTextLayout(format, text, rect.LeftTop(), rect.Size());
	}

	void CDirect2DWrite::DrawTextInRect(const FormatF& format, const std::wstring& text, const CRectF& rect)
	{
		d2dw::FormatF tmpFormat(format);
		tmpFormat.Alignment.TextAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER;
		tmpFormat.Alignment.ParagraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER;

		d2dw::CSizeF size = CalcTextSize(tmpFormat, text);

		while (size.width > rect.Width() || size.height > rect.Height()) {
			tmpFormat.Font.Size -= 1.0f;
			size = CalcTextSize(tmpFormat, text);
		}

		DrawTextLayout(tmpFormat, text, rect);
	}

	void CDirect2DWrite::DrawSolidRectangle(const SolidLine& line, const D2D1_RECT_F& rc)
	{
		auto rect = LayoutRound(rc);

		//GetHwndRenderTarget()->DrawRectangle(LayoutRound(rect), GetColorBrush(line.Color), line.Width);
		GetHwndRenderTarget()->DrawLine(CPointF(rect.left, rect.top), CPointF(rect.right, rect.top), GetColorBrush(line.Color), line.Width);
		GetHwndRenderTarget()->DrawLine(CPointF(rect.right, rect.top), CPointF(rect.right, rect.bottom), GetColorBrush(line.Color), line.Width);
		GetHwndRenderTarget()->DrawLine(CPointF(rect.right, rect.bottom), CPointF(rect.left, rect.bottom), GetColorBrush(line.Color), line.Width);
		GetHwndRenderTarget()->DrawLine(CPointF(rect.left, rect.bottom), CPointF(rect.left, rect.top), GetColorBrush(line.Color), line.Width);

	}

	void CDirect2DWrite::FillSolidRectangle(const SolidFill& fill, const D2D1_RECT_F& rect)
	{
		GetHwndRenderTarget()->FillRectangle(LayoutRound(rect), GetColorBrush(fill.Color));
	}

	void CDirect2DWrite::DrawSolidGeometry(const SolidLine& line, CComPtr<ID2D1PathGeometry>& path)
	{
		GetHwndRenderTarget()->DrawGeometry(path, GetColorBrush(line.Color), line.Width);
	}
	void CDirect2DWrite::FillSolidGeometry(const SolidFill& fill, CComPtr<ID2D1PathGeometry>& path)
	{
		GetHwndRenderTarget()->FillGeometry(path, GetColorBrush(fill.Color));
	}

	void CDirect2DWrite::DrawIcon(HICON hIcon, d2dw::CRectF& rect)
	{

		CComPtr<IWICBitmap> pWICBitmap;
		if (FAILED(GetWICImagingFactory()->CreateBitmapFromHICON(hIcon, &pWICBitmap))) {
			throw std::exception(FILE_LINE_FUNC);
		}
		CComPtr<IWICFormatConverter> pWICFormatConverter;
		if (FAILED(GetWICImagingFactory()->CreateFormatConverter(&pWICFormatConverter))) {
			throw std::exception(FILE_LINE_FUNC);
		}
		if (FAILED(pWICFormatConverter->Initialize(pWICBitmap, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut))) {
			throw std::exception(FILE_LINE_FUNC);
		}
		double dpix = 96.0f, dpiy = 96.0f;
		if (FAILED(pWICFormatConverter->GetResolution(&dpix, &dpiy))) {
			throw std::exception(FILE_LINE_FUNC);
		}

		D2D1_BITMAP_PROPERTIES bitmapProps;
		//bitmapProps.bitmapOptions = D2D1_BITMAP_OPTIONS_NONE;
		bitmapProps.dpiX = (FLOAT)dpix;
		bitmapProps.dpiY = (FLOAT)dpiy;
		bitmapProps.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
		bitmapProps.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
		//bitmapProps.colorContext = nullptr;

		CComPtr<ID2D1Bitmap> pBitmap;
		HRESULT hr = GetHwndRenderTarget()->CreateBitmapFromWicBitmap(pWICBitmap, bitmapProps, &pBitmap);
		if (FAILED(hr)) {
			SPDLOG_INFO("Failed");
			SPDLOG_INFO(fmt::format("{:08X}", (LONG_PTR)hIcon));
			SPDLOG_INFO(fmt::format("{:08X}", hr));

			throw std::exception(FILE_LINE_FUNC);
		}

		GetHwndRenderTarget()->DrawBitmap(pBitmap, rect);
	}

	void CDirect2DWrite::DrawBitmap(const CComPtr<ID2D1Bitmap>& pBitmap, const CRectF& rect)
	{
		if (pBitmap) {
			auto bmpRect = d2dw::CRectF(
				std::round(rect.left),
				std::round(rect.top),
				std::round(rect.right),
				std::round(rect.bottom));
			GetHwndRenderTarget()->DrawBitmap(pBitmap, bmpRect);
		}
	}

	std::vector<CRectF> CDirect2DWrite::CalcCharRects(const FormatF& format, const std::wstring& text, const CSizeF& size)
	{
		if (text.empty()) {
			return std::vector<CRectF>();
		} else {
			auto pLayout = GetTextLayout(format, text, size);
			std::vector<CRectF> rects(text.size(), d2dw::CRectF());
			for (size_t i = 0; i < rects.size(); i++) {
				float x, y;
				DWRITE_HIT_TEST_METRICS tm;
				pLayout->HitTestTextPosition(i, false, &x, &y, &tm);
				rects[i].SetRect(tm.left, tm.top, tm.left + tm.width, tm.top + tm.height);
			}
			return rects;
		}
	}

	FLOAT CDirect2DWrite::GetPixels2DipsRatioX()
	{
		if (m_xPixels2Dips == 0.0f || m_yPixels2Dips == 0.0f) {
			//UINT dpi = ::GetDpiForWindow(m_hWnd);
			UINT dpi = 96;
			m_xPixels2Dips = 96.0f / dpi;
			m_yPixels2Dips = 96.0f / dpi;
		}
		return m_xPixels2Dips;
	}

	FLOAT CDirect2DWrite::GetPixels2DipsRatioY()
	{
		if (m_xPixels2Dips == 0.0f || m_yPixels2Dips == 0.0f) {
			//UINT dpi = ::GetDpiForWindow(m_hWnd);
			UINT dpi = 96;
			m_xPixels2Dips = 96.0f / dpi;
			m_yPixels2Dips = 96.0f / dpi;
		}
		return m_yPixels2Dips;
	}

	FLOAT CDirect2DWrite::Pixels2DipsX(int x)
	{
		return x * GetPixels2DipsRatioX();
	}

	FLOAT CDirect2DWrite::Pixels2DipsY(int y)
	{
		return y * GetPixels2DipsRatioY();
	}

	int CDirect2DWrite::Dips2PixelsX(FLOAT x)
	{
		return static_cast<int>(std::round(x / GetPixels2DipsRatioX()));
	}

	int CDirect2DWrite::Dips2PixelsY(FLOAT y)
	{
		return static_cast<int>(std::round(y / GetPixels2DipsRatioY()));
	}

	CPointF CDirect2DWrite::Pixels2Dips(CPoint pt)
	{
		return CPointF(Pixels2DipsX(pt.x), Pixels2DipsY(pt.y));
	}

	CPoint CDirect2DWrite::Dips2Pixels(CPointF pt)
	{
		return CPoint(Dips2PixelsX(pt.x), Dips2PixelsY(pt.y));
	}

	CSizeF CDirect2DWrite::Pixels2Dips(CSize sz)
	{
		return CSizeF(Pixels2DipsX(sz.cx), Pixels2DipsY(sz.cy));
	}

	CSize CDirect2DWrite::Dips2Pixels(CSizeF sz)
	{
		return CSize(Dips2PixelsX(sz.width), Dips2PixelsY(sz.height));
	}


	CRectF CDirect2DWrite::Pixels2Dips(CRect rc)
	{
		return CRectF(Pixels2DipsX(rc.left), Pixels2DipsY(rc.top),
			Pixels2DipsX(rc.right), Pixels2DipsY(rc.bottom));
	}

	CRect CDirect2DWrite::Dips2Pixels(CRectF rc)
	{
		return CRect(Dips2PixelsX(rc.left), Dips2PixelsY(rc.top),
			Dips2PixelsX(rc.right), Dips2PixelsY(rc.bottom));
	}

	void CDirect2DWrite::Clear()
	{
		m_pD2D1Factory = nullptr;
		m_pDWriteFactory = nullptr;
		m_pWICImagingFactory = nullptr;
		m_pHwndRenderTarget = nullptr;

		m_solidColorBrushMap.clear();
		m_textFormatMap.clear();
		m_textLayoutMap.clear();
		m_defaultHeightMap.clear();

		m_xPixels2Dips = 0.0f;
		m_yPixels2Dips = 0.0f;

		m_pIconCache->Clear();
	}

}