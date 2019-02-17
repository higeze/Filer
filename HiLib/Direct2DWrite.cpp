//#include "stdafx.h"
#include "Direct2DWrite.h"

namespace d2dw
{
	//CPointF
	CPointF::CPointF() :D2D1_POINT_2F{ 0.0f } {}
	CPointF::CPointF(FLOAT a, FLOAT b) :D2D1_POINT_2F{ a, b } {}
	void CPointF::SetPoint(FLOAT a, FLOAT b) { x = a; y = b; }
	void CPointF::Offset(LONG xOffset, LONG yOffset) { x += xOffset; y += yOffset; }
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

	//CRectF
	CRectF::CRectF() :D2D1_RECT_F{ 0.0f } {}
	CRectF::CRectF(FLOAT l, FLOAT t, FLOAT r, FLOAT b) :D2D1_RECT_F{ l,t,r,b } {}
	CRectF::CRectF(const CSizeF& size) :D2D1_RECT_F{ 0, 0, size.width, size.height } {}
	void CRectF::SetRect(FLOAT l, FLOAT t, FLOAT r, FLOAT b)
	{
		left = l; top = t; right = r; bottom = b;
	}
	void CRectF::MoveToX(int x) { right = x + right - left; left = x; }
	void CRectF::MoveToY(int y) { bottom = y + bottom - top; top = y; }
	void CRectF::MoveToXY(int x, int y)
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
		return FamilyName == rhs.FamilyName && Size == rhs.Size && TextAlignment == rhs.TextAlignment && ParagraphAlignment == rhs.ParagraphAlignment;
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
		boost::hash_combine(seed, std::hash<decltype(TextAlignment)>()(TextAlignment));
		boost::hash_combine(seed, std::hash<decltype(ParagraphAlignment)>()(ParagraphAlignment));
		return seed;
	}






	//CDirect2DWrite
	CDirect2DWrite::CDirect2DWrite(HWND hWnd) :m_hWnd(hWnd) {}


	CComPtr<ID2D1Factory>& CDirect2DWrite::GetD2D1Factory()
	{
		if (!m_pD2D1Factory) {
			if (FAILED(
				::D2D1CreateFactory(
					D2D1_FACTORY_TYPE_SINGLE_THREADED,
					__uuidof(ID2D1Factory),
					(void**)&m_pD2D1Factory))) {
				throw std::exception(FILELINEFUNCTION);
			}
		}
		return m_pD2D1Factory;
	}

	CComPtr<IDWriteFactory>& CDirect2DWrite::GetDWriteFactory()
	{
		if (!m_pDWriteFactory) {
			if (FAILED(
				::DWriteCreateFactory(
					DWRITE_FACTORY_TYPE_SHARED,
					__uuidof(IDWriteFactory),
					(IUnknown**)&m_pDWriteFactory))) {
				throw std::exception(FILELINEFUNCTION);
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
				throw std::exception(FILELINEFUNCTION);
			}
		}
		return m_pWICImagingFactory;
	}

	//CComPtr<IWICFormatConverter>& CDirect2DWrite::GetWICFormatConverter()
	//{
	//	if (!m_pWICFormatConverter) {
	//		if (FAILED(
	//			GetWICImagingFactory()->CreateFormatConverter(&m_pWICFormatConverter))) {
	//			throw std::exception(FILELINEFUNCTION);
	//		}
	//	}
	//	return m_pWICFormatConverter;
	//}


#ifdef USE_ID2D1DCRenderTarget
	CComPtr<ID2D1DCRenderTarget>& CDirect2DWrite::GetHwndRenderTarget()
#else
	CComPtr<ID2D1HwndRenderTarget>& CDirect2DWrite::GetHwndRenderTarget()
#endif
	{

		if (!m_pHwndRenderTarget) {
#ifdef USE_ID2D1DCRenderTarget
			D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
				D2D1_RENDER_TARGET_TYPE_DEFAULT,
				D2D1::PixelFormat(
					DXGI_FORMAT_B8G8R8A8_UNORM,
					D2D1_ALPHA_MODE_IGNORE),
				0,
				0,
				D2D1_RENDER_TARGET_USAGE_NONE,
				D2D1_FEATURE_LEVEL_DEFAULT
			);

			if (FAILED(GetD2D1Factory()->CreateDCRenderTarget(&props, &m_pHwndRenderTarget))) {
				throw std::exception(FILELINEFUNCTION);
			}
#else
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
				throw std::exception(FILELINEFUNCTION);
			}
#endif
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
				throw std::exception(FILELINEFUNCTION);
			} else {
				auto ret = m_solidColorBrushMap.emplace(color, pBrush);
				return ret.first->second;
			}
		}
	}

	CComPtr<IDWriteTextFormat>& CDirect2DWrite::GetTextFormat(const CFontF& font)
	{
		auto iter = m_textFormatMap.find(font);
		if (iter != m_textFormatMap.end()) {
			return iter->second;
		} else {
			CComPtr<IDWriteTextFormat> pTextFormat;
			if (FAILED(GetDWriteFactory()->CreateTextFormat(
				font.FamilyName.c_str(),
				nullptr,
				DWRITE_FONT_WEIGHT_REGULAR,
				DWRITE_FONT_STYLE_NORMAL,
				DWRITE_FONT_STRETCH_NORMAL,
				font.Size,
				L"en-us",
				&pTextFormat))){
				throw std::exception(FILELINEFUNCTION);
			} else {
				pTextFormat->SetTextAlignment(font.TextAlignment);
				pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING::DWRITE_WORD_WRAPPING_CHARACTER);
				pTextFormat->SetParagraphAlignment(font.ParagraphAlignment);
				//pTextFormat->SetLineSpacing(DWRITE_LINE_SPACING_METHOD_UNIFORM, font.Size, font.Size*0.8f);
				auto ret = m_textFormatMap.emplace(font, pTextFormat);
				return ret.first->second;
			}
		}
	}

	CSizeF CDirect2DWrite::CalcTextSize(const CFontF& font, const std::wstring& text)
	{
		auto fontIter = m_charMap.find(font);
		if (fontIter == m_charMap.end()) {
			m_charMap.emplace(font, std::unordered_map<wchar_t, CSizeF>());
			fontIter = m_charMap.find(font);
		}

		CSizeF textSize( 0.0f, 0.0f );
		for (const auto& ch : text) {
			const auto& iter = fontIter->second.find(ch);
			CSizeF charSize( 0.0f, 0.0f );
			if (iter != fontIter->second.end()) {
				charSize = iter->second;
			} else {
				DWRITE_TEXT_METRICS charMetrics;
				CComPtr<IDWriteTextLayout> pTextLayout = NULL;
				GetDWriteFactory()->CreateTextLayout(
					(&ch), 1, GetTextFormat(font),
					FLT_MAX, FLT_MAX, &pTextLayout);
				pTextLayout->GetMetrics(&charMetrics);
				charSize.width = charMetrics.widthIncludingTrailingWhitespace;
				charSize.height = charMetrics.height;
				fontIter->second.emplace(ch, charSize);
			}
			textSize.width += charSize.width;
			textSize.height = (std::max)(textSize.height, charSize.height);
		}

		return textSize;
	}

	CSizeF CDirect2DWrite::CalcTextSizeWithFixedWidth(const CFontF& font, const std::wstring& text, const FLOAT width)
	{
		auto fontIter = m_charMap.find(font);
		if (fontIter == m_charMap.end()) {
			m_charMap.emplace(font, std::unordered_map<wchar_t, CSizeF>());
			fontIter = m_charMap.find(font);
		}

		std::vector<D2D1_SIZE_F> lineSizes;
		lineSizes.emplace_back(D2D1_SIZE_F{0.0f, 0.0f});

		for (const auto& ch : text) {
			const auto& iter = fontIter->second.find(ch);
			CSizeF charSize{ 0.0f, 0.0f };
			if (iter != fontIter->second.end()){
				charSize = iter->second;
			} else {
				DWRITE_TEXT_METRICS charMetrics;
				CComPtr<IDWriteTextLayout> pTextLayout = NULL;
				GetDWriteFactory()->CreateTextLayout(
					(&ch), 1, GetTextFormat(font),
					FLT_MAX, FLT_MAX, &pTextLayout);
				pTextLayout->GetMetrics(&charMetrics);
				charSize.width = charMetrics.widthIncludingTrailingWhitespace;
				charSize.height = charMetrics.height;
				fontIter->second.emplace(ch, charSize);
			}
			if (lineSizes.back().width + charSize.width > width) {
				lineSizes.push_back(charSize);
			} else {
				lineSizes.back().width += charSize.width;
				lineSizes.back().height = (std::max)(lineSizes.back().height, charSize.height);
			}
		}

		return D2D1::SizeF(width, std::accumulate(lineSizes.begin(), lineSizes.end(), 0.0f, [](FLOAT height, const CSizeF& rhs)->FLOAT {return height + rhs.height; }));
	}
#ifdef USE_ID2D1DCRenderTarget
	void CDirect2DWrite::BeginDraw(HDC hDC, const RECT& rc) 
	{
		m_hDC = hDC;
		::SetBkMode(m_hDC, TRANSPARENT);
		GetHwndRenderTarget()->BindDC(hDC, &rc);
		GetHwndRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());
		GetHwndRenderTarget()->BeginDraw(); 

	}
#else
	void CDirect2DWrite::BeginDraw()
	{
		GetHwndRenderTarget()->SetTransform(D2D1::Matrix3x2F::Identity());
		GetHwndRenderTarget()->BeginDraw();

	}
#endif

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
#ifdef USE_ID2D1DCRenderTarget
		for (auto& fun : m_gdifuncs) {
			fun();
		}
		m_gdifuncs.clear();
#endif
	}


	void CDirect2DWrite::DrawSolidLine(const SolidLine& line, const D2D1_POINT_2F& p0, const D2D1_POINT_2F& p1)
	{
		GetHwndRenderTarget()->DrawLine(LayoutRound(p0), LayoutRound(p1), GetColorBrush(line.Color),line.Width);
	}

	void CDirect2DWrite::DrawTextLayout(const FontAndColor& fnc, const std::wstring& text, const D2D1_POINT_2F& origin, const D2D1_SIZE_F& size)
	{
		CComPtr<IDWriteTextLayout> pTextLayout = NULL;
		GetDWriteFactory()->CreateTextLayout(text.c_str(), text.size(), GetTextFormat(fnc.Font), size.width, size.height, &pTextLayout);
		GetHwndRenderTarget()->DrawTextLayout(origin, pTextLayout, GetColorBrush(fnc.Color), D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP);
	}

	void CDirect2DWrite::DrawTextLayout(const FontAndColor& fnc, const std::wstring& text, const CRectF& rect)
	{
		DrawTextLayout(fnc, text, rect.LeftTop(), rect.Size());
	}

	void CDirect2DWrite::DrawTextInRect(const FontAndColor& fnc, const std::wstring& text, const CRectF& rect)
	{
#ifdef USE_ID2D1DCRenderTarget
	m_gdifuncs.push_back([this,fnc, rect, text]()->void {
		//Paint Text
		::SetTextColor(m_hDC, GetSysColor(COLOR_WINDOWTEXT));

		//Find font size
		CFont font;
		HFONT hFont = NULL;
		CRect rcContent;
		int i = 0;
		do {
			font = CFont(CDirect2DWrite::Dips2Points(fnc.Font.Size) - i, fnc.Font.FamilyName);
			hFont = (HFONT)::SelectObject(m_hDC, (HGDIOBJ)(font.operator HFONT()));
			DrawTextEx(m_hDC, const_cast<LPTSTR>(text.c_str()), text.size(), rcContent,
				DT_CALCRECT | DT_CENTER | DT_VCENTER, NULL);
			::SelectObject(m_hDC, hFont);
			i++;
		} while (rcContent.Width() > 16);
		hFont = (HFONT)::SelectObject(m_hDC, (HGDIOBJ)(font.operator HFONT()));
		//rcPaint.top = rcPaint.bottom - rcContent.Height();
		::DrawTextEx(m_hDC, const_cast<LPTSTR>(text.c_str()), -1, Dips2Pixels(rect), DT_CENTER | DT_VCENTER, NULL);
		::SelectObject(m_hDC, (HGDIOBJ)hFont);
	});

#elif
		d2dw::CFontF font = m_spProperty->FontAndColor->Font;
		d2dw::CSizeF size = direct.CalcTextSize(font, str);

		while (size.width > direct.Pixels2DipsX(16)) {
			font.Size -= 1.0f;
			size = direct.CalcTextSize(font, str);
		}
		font.TextAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER;
		font.ParagraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_FAR;
		rcPaint.right = rcPaint.left + direct.Pixels2DipsX(16);
		rcPaint.bottom = rcPaint.top + direct.Pixels2DipsY(16);

		direct.DrawTextLayout(d2dw::FontAndColor(font, m_spProperty->FontAndColor->Color), str, rcPaint);
#endif

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

#ifndef USE_ID2D1DCRenderTarget
		//CRect rc = Dips2Pixels(rect);
		//HDC hDC = GetDC(NULL);
		//HDC hMemDC = CreateCompatibleDC(hDC);
		//HBITMAP hMemBmp = CreateCompatibleBitmap(hDC, rc.Width(), rc.Height());
		//HBITMAP hResultBmp = NULL;
		//HGDIOBJ hOrgBMP = SelectObject(hMemDC, hMemBmp);

		//DrawIconEx(hMemDC, 0, 0, hIcon, rc.Width(), rc.Height(), 0, NULL, DI_NORMAL);

		//hResultBmp = hMemBmp;
		//hMemBmp = NULL;

		//SelectObject(hMemDC, hOrgBMP);
		//DeleteDC(hMemDC);
		//ReleaseDC(NULL, hDC);

		//CComPtr<IWICBitmap> pWICBitmap;
		//if (FAILED(GetWICImagingFactory()->CreateBitmapFromHBITMAP(hResultBmp, 0, WICBitmapAlphaChannelOption::WICBitmapUseAlpha, &pWICBitmap))) {
		//	throw std::exception(FILELINEFUNCTION);
		//}
		//::DeleteObject(hResultBmp);



		CComPtr<IWICBitmap> pWICBitmap;
		if (FAILED(GetWICImagingFactory()->CreateBitmapFromHICON(hIcon, &pWICBitmap))) {
			throw std::exception(FILELINEFUNCTION);
		}
		CComPtr<IWICFormatConverter> pWICFormatConverter;
		if (FAILED(GetWICImagingFactory()->CreateFormatConverter(&pWICFormatConverter))) {
			throw std::exception(FILELINEFUNCTION);
		}
		if (FAILED(pWICFormatConverter->Initialize(pWICBitmap, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, nullptr, 0.0f, WICBitmapPaletteTypeMedianCut))) {
			throw std::exception(FILELINEFUNCTION);
		}
		double dpix = 96.0f, dpiy = 96.0f;
		if (FAILED(pWICFormatConverter->GetResolution(&dpix, &dpiy))) {
			throw std::exception(FILELINEFUNCTION);
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
			BOOST_LOG_TRIVIAL(trace) << L"Failed";
			BOOST_LOG_TRIVIAL(trace) << boost::format("%08X") % hIcon;
			BOOST_LOG_TRIVIAL(trace) << boost::format("%08X") % hr;

			throw std::exception(FILELINEFUNCTION);
		}

		//CComPtr<ID2D1Bitmap> pBitmap;
		//HRESULT hr = GetHwndRenderTarget()->CreateBitmapFromWicBitmap(pWICBitmap, &pBitmap);
		//if (FAILED(hr)) {
		//		BOOST_LOG_TRIVIAL(trace) << L"Failed";
		//		BOOST_LOG_TRIVIAL(trace) << boost::format("%08X") % hIcon;
		//		BOOST_LOG_TRIVIAL(trace) << boost::format("%08X") % hr;
		//	throw std::exception(FILELINEFUNCTION);
		//}
		GetHwndRenderTarget()->DrawBitmap(pBitmap, rect);
#else

		CRect rc = Dips2Pixels(rect);
		m_gdifuncs.push_back([this, rc, hIcon]()->void{
			::DrawIconEx(m_hDC, rc.left, rc.top, hIcon, rc.Width(), rc.Height(), 0, NULL, DI_NORMAL);
		});
#endif

	}






	//CComPtr<ID2D1DCRenderTarget> CDirect2DWrite::GetDcRenderTarget()
	//{
	//	if (!m_pDCRenderTarget) {
	//		D2D1_RENDER_TARGET_PROPERTIES props =
	//			D2D1::RenderTargetProperties(
	//				D2D1_RENDER_TARGET_TYPE_DEFAULT,
	//				D2D1::PixelFormat(
	//					DXGI_FORMAT_B8G8R8A8_UNORM,
	//					D2D1_ALPHA_MODE_IGNORE),
	//				0.0F, 0.0F,
	//				D2D1_RENDER_TARGET_USAGE_GDI_COMPATIBLE
	//			);
	//		if (FAILED(GetD2D1Factory()->CreateDCRenderTarget(
	//			&props, &m_pDCRenderTarget))){
	//			throw std::exception(FILELINEFUNCTION);
	//		}
	//	}
	//	return m_pDCRenderTarget;
	//}

	//	if (pGradientBrush == NULL) {
	//		static const D2D1_GRADIENT_STOP stops[] =
	//		{
	//			 {   0.0f,  { 0.0f, 1.0f, 1.0f, 1.0f }  },
	//			 {   1.0f,  { 0.0f, 0.0f, 1.0f, 0.5f }  },
	//		};
	//		hr = GetHwndRenderTarget()->CreateGradientStopCollection(
	//			stops, sizeof(stops) / sizeof(D2D1_GRADIENT_STOP), &pGradientStops);
	//		if (!SUCCEEDED(hr)) return hr;
	//		hr = GetHwndRenderTarget()->CreateLinearGradientBrush(    //©[30] 
	//			D2D1::LinearGradientBrushProperties(
	//				D2D1::Point2F(100.0F, 120.0F),
	//				D2D1::Point2F(100.0F, 270.0F)),
	//			D2D1::BrushProperties(),
	//			pGradientStops,
	//			&pGradientBrush);
	//		if (!SUCCEEDED(hr)) return hr;
	//	}

	//	if (m_pHwndRenderTarget->CheckWindowState() & D2D1_WINDOW_STATE_OCCLUDED)
	//		return;

	//	RECT rect;
	//	::GetClientRect(hWnd, &rect);
	//	GetHwndRenderTarget()->Resize(D2D1::SizeU(rect.right, rect.bottom));


	//	GetHwndRenderTarget()->BeginDraw();  //©[35] 

	//	GetHwndRenderTarget()->Clear(D2D1::ColorF(1.0F, 1.0F, 1.0F));

	//	D2D1_ELLIPSE ellipse1 =
	//		D2D1::Ellipse(D2D1::Point2F(120.0F, 120.0F), 100.0F, 100.0F);
	//	GetHwndRenderTarget()->DrawEllipse(ellipse1, pGreenBrush, 10.0F);

	//	D2D1_RECT_F rect1 =
	//		D2D1::RectF(100.0F, 50.0F, 300.0F, 100.F);
	//	GetHwndRenderTarget()->FillRectangle(&rect1, pBlueBrush);

	//	D2D1_RECT_F rect2 =
	//		D2D1::RectF(100.0F, 120.0F, 250.0F, 270.F);
	//	GetHwndRenderTarget()->FillRectangle(&rect2, pGradientBrush);

	//	GetHwndRenderTarget()->EndDraw();
	//}

	//HRESULT CDirect2DWrite::CreateDeviceResourcesAlt()
	//{
	//	HRESULT hr = S_OK;

	//	if (pGreenBrush == NULL) {
	//		hr = GetDcRenderTarget()->CreateSolidColorBrush(
	//			D2D1::ColorF(0.0F, 1.0F, 0.0F), &pGreenBrush);
	//		if (!SUCCEEDED(hr)) return hr;
	//	}
	//	if (pBlueBrush == NULL) {
	//		hr = GetDcRenderTarget()->CreateSolidColorBrush(
	//			D2D1::ColorF(0.0F, 0.0F, 1.0F, 0.5F), &pBlueBrush);
	//		if (!SUCCEEDED(hr)) return hr;
	//	}
	//	if (pGradientBrush == NULL) {
	//		static const D2D1_GRADIENT_STOP stops[] =
	//		{
	//			 {   0.0f,  { 0.0f, 1.0f, 1.0f, 1.0f }  },
	//			 {   1.0f,  { 0.0f, 0.0f, 1.0f, 0.5f }  },
	//		};
	//		hr = GetDcRenderTarget()->CreateGradientStopCollection(
	//			stops, sizeof(stops) / sizeof(D2D1_GRADIENT_STOP), &pGradientStops);
	//		if (!SUCCEEDED(hr)) return hr;
	//		hr = GetDcRenderTarget()->CreateLinearGradientBrush(
	//			D2D1::LinearGradientBrushProperties(
	//				D2D1::Point2F(100.0F, 120.0F),
	//				D2D1::Point2F(100.0F, 270.0F)),
	//			D2D1::BrushProperties(),
	//			pGradientStops,
	//			&pGradientBrush);
	//		if (!SUCCEEDED(hr)) return hr;
	//	}

	//	if (!m_pTextFormat) {
	//		hr = GetDWriteFactory()->CreateTextFormat(
	//			L"Arial",
	//			nullptr,
	//			DWRITE_FONT_WEIGHT_REGULAR,
	//			DWRITE_FONT_STYLE_NORMAL,
	//			DWRITE_FONT_STRETCH_NORMAL,
	//			ConvertPointSizeToDIP(12.0f),
	//			L"en-us",
	//			&m_pTextFormat);
	//		m_pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING);
	//	}

	//	DWRITE_TEXT_METRICS textMetrics;
	//	FLOAT minWidth;
	//	if (!m_pTextLayout) {
	//		D2D1_SIZE_F sizeRT = { 0.0f,0.0f };
	//		const wchar_t* pStr = L"Direct Write Sample with DC";
	//		GetDWriteFactory()->CreateTextLayout(pStr, _tcslen(pStr), m_pTextFormat,
	//			FLT_MAX, FLT_MAX, &m_pTextLayout);
	//		m_pTextLayout->GetMetrics(&textMetrics);
	//		m_pTextLayout->DetermineMinWidth(&minWidth);
	//		//m_pTextLayout->SetMaxWidth(minWidth);
	//		//m_pTextLayout->SetMaxHeight(textMetrics.height);
	//	}

	//	return hr;
	//}

	//void CDirect2DWrite::RenderAlt(HDC hDC, LPRECT pRect)
	//{
	//	HRESULT hr;

	//	hr = CreateDeviceResourcesAlt();
	//	if (!SUCCEEDED(hr)) return;

	//	hr = GetDcRenderTarget()->BindDC(hDC, pRect);

	//	GetDcRenderTarget()->BeginDraw();
	//	GetDcRenderTarget()->SetTransform(D2D1::IdentityMatrix());
	//	GetDcRenderTarget()->Clear(D2D1::ColorF(1.0F, 1.0F, 1.0F));  //©[63] 

	//	D2D1_ELLIPSE ellipse1 =
	//		D2D1::Ellipse(D2D1::Point2F(120.0F, 120.0F), 100.0F, 100.0F);
	//	GetDcRenderTarget()->DrawEllipse(ellipse1, pGreenBrush, 10.0F);

	//	D2D1_RECT_F rect1 =
	//		D2D1::RectF(100.0F, 50.0F, 300.0F, 100.F);
	//	GetDcRenderTarget()->FillRectangle(&rect1, pBlueBrush);

	//	D2D1_RECT_F rect2 =
	//		D2D1::RectF(100.0F, 120.0F, 250.0F, 270.F);
	//	GetDcRenderTarget()->FillRectangle(&rect2, pGradientBrush);

	//	D2D1::Point2F ptOrigin = { 0.0f, 0.0f };
	//	GetDcRenderTarget()->DrawTextLayout(ptOrigin, m_pTextLayout, pBlueBrush);





	//	GetDcRenderTarget()->EndDraw();
	//}

}