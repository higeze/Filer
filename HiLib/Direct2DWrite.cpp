//#include "stdafx.h"
#include "Direct2DWrite.h"
#include "FileIconCache.h"
#include <fmt/format.h>

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
	CRectF CRectF::IntersectRect(const CRectF& rc) const 
	{
		FLOAT l = (std::max)(left, rc.left);
		FLOAT t = (std::max)(top, rc.top);
		FLOAT r = (std::min)(right, rc.right);
		FLOAT b = (std::min)(bottom, rc.bottom);
		if (l > r || t > b) {
			return CRectF();
		} else {
			return CRectF(l, t, r, b);
		}
	}
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

	//SyntaxFormat
	bool SyntaxFormatF::operator==(const SyntaxFormatF& rhs) const
	{
		return Color == rhs.Color && IsBold == rhs.IsBold;
	}

	bool SyntaxFormatF::operator!=(const SyntaxFormatF& rhs) const
	{
		return !operator==(rhs);
	}

	std::size_t SyntaxFormatF::GetHashCode() const
	{
		std::size_t seed = 0;
		boost::hash_combine(seed, std::hash<decltype(Color)>()(Color));
		boost::hash_combine(seed, std::hash<decltype(IsBold)>()(IsBold));

		return seed;
	}





	//CDirect2DWrite
	CDirect2DWrite::CDirect2DWrite(HWND hWnd) :m_hWnd(hWnd), m_pIconCache(std::make_unique<CFileIconCache>(this)) 
	{
		GetD3DDevices = [p = CComPtr<ID3D11Device1>(), q = CComPtr<ID3D11DeviceContext1>(), this]() mutable->std::tuple<CComPtr<ID3D11Device1>&, CComPtr<ID3D11DeviceContext1>&>
		{
			if (!p) {
				// Set feature levels supported by our application
				D3D_FEATURE_LEVEL featureLevels[] =
				{
					D3D_FEATURE_LEVEL_11_1,
					D3D_FEATURE_LEVEL_11_0,
					D3D_FEATURE_LEVEL_10_1,
					D3D_FEATURE_LEVEL_10_0,
					D3D_FEATURE_LEVEL_9_3,
					D3D_FEATURE_LEVEL_9_2,
					D3D_FEATURE_LEVEL_9_1
				};
				// This flag adds support for surfaces with a different color channel ordering
				// than the API default. It is required for compatibility with Direct2D.
				UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
 
				CComPtr<ID3D11Device> device;
				CComPtr<ID3D11DeviceContext> context;
				D3D_FEATURE_LEVEL returnedFeatureLevel;
 
				D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags, featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
									&device, &returnedFeatureLevel, &context);
				device->QueryInterface(__uuidof(ID3D11Device1), (void **)&p);
				context->QueryInterface(__uuidof(ID3D11DeviceContext1), (void **)&q);
			}
			return {p, q};
		};

		GetD2DDevice =  [p = CComPtr<ID2D1Device>(), this]() mutable->CComPtr<ID2D1Device>&
		{
			if (!p) {
				GetD2DFactory()->CreateDevice(GetDXGIDevice(), &p);
			}
			return p;
		};

		GetD2DDeviceContext =  [p = CComPtr<ID2D1DeviceContext>(), this]() mutable->CComPtr<ID2D1DeviceContext>&
		{
			if (!p) {
				GetD2DDevice()->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &p);
			}
			return p;
		};


		GetDXGIDevice =  [p = CComPtr<IDXGIDevice>(), this]() mutable->CComPtr<IDXGIDevice>&
		{
			if (!p) {
				std::get<0>(GetD3DDevices())->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&p));
			}
			return p;
		};
		GetDXGIAdapter = [p = CComPtr<IDXGIAdapter>(), this]() mutable->CComPtr<IDXGIAdapter>&
		{
			if (!p) {
				GetDXGIDevice()->GetAdapter(&p);
			}
			return p;
		};

		GetDXGIFactory = [p = CComPtr<IDXGIFactory2>(), this]() mutable->CComPtr<IDXGIFactory2>&
		{
			if (!p) {
				GetDXGIAdapter()->GetParent(IID_PPV_ARGS(&p));
			}
			return p;
		};

		GetDXGISwapChain = [p = CComPtr<IDXGISwapChain1>(), this]() mutable->CComPtr<IDXGISwapChain1>&
		{
			if (!p) {
				// Describe Windows 7-compatible Windowed swap chain
				DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };

				swapChainDesc.Width = 0;
				swapChainDesc.Height = 0;
				swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
				swapChainDesc.Stereo = false;
				swapChainDesc.SampleDesc.Count = 1;
				swapChainDesc.SampleDesc.Quality = 0;
				swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
				swapChainDesc.BufferCount = 2;
				swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
				swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
				swapChainDesc.Flags = 0;
				// Create DXGI swap chain targeting a window handle (the only Windows 7-compatible option)
				GetDXGIFactory()->CreateSwapChainForHwnd(std::get<0>(GetD3DDevices()), m_hWnd, &swapChainDesc, nullptr, nullptr, &p);
			}
			return p;
		};
	
		GetD2DBackBuffer = [p = CComPtr<ID2D1Bitmap1>(), this]() mutable->CComPtr<ID2D1Bitmap1>&
		{
			if (!p) {
				// Get the back buffer as an IDXGISurface (Direct2D doesn't accept an ID3D11Texture2D directly as a render target)
				CComPtr<IDXGISurface> dxgiBackBuffer;
				GetDXGISwapChain()->GetBuffer(0, IID_PPV_ARGS(&dxgiBackBuffer));
				// Get screen DPI
				FLOAT dpiX, dpiY;
				dpiX = dpiY = static_cast<FLOAT>(::GetDpiForWindow(m_hWnd));

				// Create a Direct2D surface (bitmap) linked to the Direct3D texture back buffer via the DXGI back buffer
				D2D1_BITMAP_PROPERTIES1 bitmapProperties =
					D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
					D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE), dpiX, dpiY);

				GetD2DDeviceContext()->CreateBitmapFromDxgiSurface(dxgiBackBuffer, &bitmapProperties, &p);
			}
			return p;
		};

		GetD2DFactory =[p = CComPtr<ID2D1Factory1>(), this]() mutable->CComPtr<ID2D1Factory1>&
		{
			if (!p) {
				D2D1_FACTORY_OPTIONS options;
				ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));
				if (FAILED(
					::D2D1CreateFactory(
						D2D1_FACTORY_TYPE::D2D1_FACTORY_TYPE_MULTI_THREADED,
						__uuidof(ID2D1Factory1),
						&options,
						reinterpret_cast<void**>(&p)))) {
					throw std::exception(FILE_LINE_FUNC);
				}
			}
			return p;
		};
		GetDWriteFactory = [p = CComPtr<IDWriteFactory1>(), this]() mutable->CComPtr<IDWriteFactory1>&
		{
			if (!p) {
				if (FAILED(
					::DWriteCreateFactory(
						DWRITE_FACTORY_TYPE::DWRITE_FACTORY_TYPE_SHARED,
						__uuidof(IDWriteFactory1),
						reinterpret_cast<IUnknown**>(&p)))) {
					throw std::exception(FILE_LINE_FUNC);
				}
			}
			return p;
		};

		GetWICImagingFactory =  [p = CComPtr<IWICImagingFactory2>(), this]() mutable->CComPtr<IWICImagingFactory2>&
		{
			if (!p) {
				if (FAILED(
					p.CoCreateInstance(
						CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER))) {
					throw std::exception(FILE_LINE_FUNC);
				}
			}
			return p;
		};

	}
	CDirect2DWrite::~CDirect2DWrite() = default;

	CComPtr<ID2D1SolidColorBrush>& CDirect2DWrite::GetColorBrush(const CColorF& color)
	{
		auto iter = m_solidColorBrushMap.find(color);
		if (iter != m_solidColorBrushMap.end()) {
			return iter->second;
		} else {
			CComPtr<ID2D1SolidColorBrush> pBrush;
			if (FAILED(GetD2DDeviceContext()->CreateSolidColorBrush(color, &pBrush))) {
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
		GetD2DDeviceContext()->SetTransform(D2D1::Matrix3x2F::Identity());
		GetD2DDeviceContext()->BeginDraw();

	}

	void CDirect2DWrite::ClearSolid(const SolidFill& fill)
	{
		GetD2DDeviceContext()->Clear(fill.Color);
	}

	void CDirect2DWrite::Resize()
	{
		GetDXGISwapChain() = nullptr;
		GetD2DBackBuffer() = nullptr;

		// Set surface as render target in Direct2D device context
		GetD2DDeviceContext()->SetTarget(GetD2DBackBuffer());

	}

	void CDirect2DWrite::EndDraw()
	{
		HRESULT hr = GetD2DDeviceContext()->EndDraw();
		if (FAILED(hr) || hr == D2DERR_RECREATE_TARGET) {
			Clear();
		}
		// Present (new for Direct2D 1.1)
		DXGI_PRESENT_PARAMETERS parameters = { 0 };
		parameters.DirtyRectsCount = 0;
		parameters.pDirtyRects = nullptr;
		parameters.pScrollRect = nullptr;
		parameters.pScrollOffset = nullptr;
 
		hr = GetDXGISwapChain()->Present1(1, 0, &parameters);
	}


	void CDirect2DWrite::DrawSolidLine(const SolidLine& line, const D2D1_POINT_2F& p0, const D2D1_POINT_2F& p1)
	{
		GetD2DDeviceContext()->DrawLine(LayoutRound(p0), LayoutRound(p1), GetColorBrush(line.Color),line.Width);
	}

	void CDirect2DWrite::DrawSolidLine(const CColorF& color, const FLOAT& width, const D2D1_POINT_2F& p0, const D2D1_POINT_2F& p1)
	{
		GetD2DDeviceContext()->DrawLine(LayoutRound(p0), LayoutRound(p1), GetColorBrush(color), width);
	}


	void CDirect2DWrite::DrawSolidTriangleWave(const SolidLine& line, const D2D1_POINT_2F& startPoint, const D2D1_POINT_2F& endPoint, const FLOAT& amplitude, const FLOAT& period)
	{
		D2D_POINT_2F nextCenterPoint = startPoint;
		D2D_POINT_2F currPoint = startPoint;
		D2D_POINT_2F nextPoint;
		FLOAT radPoint = std::atan2(endPoint.y - startPoint.y, endPoint.x - startPoint.x);
		FLOAT radWave = std::atan2(amplitude, (period / 2.f));
		FLOAT hypotenuseWave = static_cast<FLOAT>(std::sqrt(std::pow(period / 4.f, 2) + std::pow(amplitude / 2.f, 2)));
		int count = 0;
		while (true) {
			nextCenterPoint.x = startPoint.x + std::cos(radPoint) * ((count + 1) * period / 4.f);
			nextCenterPoint.y = startPoint.y + std::sin(radPoint) * ((count + 1) * period / 4.f);

			if ((startPoint.x <= endPoint.x ? nextCenterPoint.x > endPoint.x : nextCenterPoint.x < endPoint.x) ||
				(startPoint.y <= endPoint.y ? nextCenterPoint.y > endPoint.y : nextCenterPoint.y < endPoint.y)) {
				break;
			}

			switch (count % 4) {
				case 0:
					nextPoint = currPoint;
					nextPoint.x += std::cos(radPoint + radWave) * hypotenuseWave;
					nextPoint.y += std::sin(radPoint + radWave) * hypotenuseWave;
					break;
				case 1:
					nextPoint = nextCenterPoint;
					break;
				case 2:
					nextPoint = currPoint;
					nextPoint.x += std::cos(radPoint + radWave) * hypotenuseWave;
					nextPoint.y -= std::sin(radWave - radPoint) * hypotenuseWave;
					break;
				case 3:
					nextPoint = nextCenterPoint;
					break;
				default:
					break;
			}

			DrawSolidLine(line, currPoint, nextPoint);

			currPoint = nextPoint;
			count++;
		}
	}


	void CDirect2DWrite::DrawTextLayout(const FormatF& format, const std::wstring& text, const D2D1_POINT_2F& origin, const D2D1_SIZE_F& size)
	{
		GetD2DDeviceContext()->DrawTextLayout(origin, GetTextLayout(format, text, size), GetColorBrush(format.Color), D2D1_DRAW_TEXT_OPTIONS::D2D1_DRAW_TEXT_OPTIONS_CLIP);
	}

	void CDirect2DWrite::DrawTextLayout(const FormatF& format, const std::wstring& text, const CRectF& rect)
	{
		DrawTextLayout(format, text, rect.LeftTop(), rect.Size());
	}

	void CDirect2DWrite::DrawTextInRect(const FormatF& format, const std::wstring& text, const CRectF& rect)
	{
		FormatF tmpFormat(format);
		tmpFormat.Alignment.TextAlignment = DWRITE_TEXT_ALIGNMENT::DWRITE_TEXT_ALIGNMENT_CENTER;
		tmpFormat.Alignment.ParagraphAlignment = DWRITE_PARAGRAPH_ALIGNMENT::DWRITE_PARAGRAPH_ALIGNMENT_CENTER;

		CSizeF size = CalcTextSize(tmpFormat, text);

		while (size.width > rect.Width() || size.height > rect.Height()) {
			tmpFormat.Font.Size -= 1.0f;
			size = CalcTextSize(tmpFormat, text);
		}

		DrawTextLayout(tmpFormat, text, rect);
	}

	void CDirect2DWrite::DrawTab(const SolidLine& line, const CRectF& rc)
	{
		auto width = rc.Width();
		auto height = rc.Height();
		auto centerPoint = rc.CenterPoint();
		auto deg2rad = [](const int& deg) { return deg * (3.14f / 180); };
		
		auto left = CPointF(rc.left + 0.1f * width, centerPoint.y);
		auto right = CPointF(rc.left + 0.9f * width, centerPoint.y);
		auto top = CPointF(right.x - 0.3f * height * std::cos(deg2rad(45)), centerPoint.y - 0.3f * height * std::sin(deg2rad(45)));
		auto bottom = CPointF(right.x - 0.3f * height * std::cos(deg2rad(45)), centerPoint.y + 0.3f * height * std::sin(deg2rad(45)));

		GetD2DDeviceContext()->DrawLine(left, right, GetColorBrush(line.Color), line.Width);
		GetD2DDeviceContext()->DrawLine(right, top, GetColorBrush(line.Color), line.Width);
		GetD2DDeviceContext()->DrawLine(right, bottom, GetColorBrush(line.Color), line.Width);

	}

	void CDirect2DWrite::DrawLineFeed(const SolidLine& line, const CRectF& rc)
	{
		auto width = rc.Width();
		auto height = rc.Height();
		auto centerPoint = rc.CenterPoint();
		auto deg2rad = [](const int& deg) { return (float)(deg * (3.14f / 180.f)); };

		auto top = CPointF(centerPoint.x, rc.top + 0.1f * height);
		auto bottom = CPointF(centerPoint.x, rc.top + 0.9f * height);
		auto left = CPointF(centerPoint.x - 0.3f * height * std::cos(deg2rad(45)), bottom.y - 0.3f * height * std::cos(deg2rad(45)));
		auto right = CPointF(centerPoint.x + 0.3f * height * std::cos(deg2rad(45)), bottom.y - 0.3f * height * std::cos(deg2rad(45)));

		GetD2DDeviceContext()->DrawLine(top, bottom, GetColorBrush(line.Color), line.Width);
		GetD2DDeviceContext()->DrawLine(bottom, left, GetColorBrush(line.Color), line.Width);
		GetD2DDeviceContext()->DrawLine(bottom, right, GetColorBrush(line.Color), line.Width);
	}

	void CDirect2DWrite::DrawHalfSpace(const SolidLine& line, const CRectF& rc)
	{
		auto width = rc.Width();
		auto height = rc.Height();
		auto centerPoint = rc.CenterPoint();

		D2D1_ELLIPSE ellipse = D2D1::Ellipse(
			centerPoint,
			line.Width,
			line.Width
		);

		GetD2DDeviceContext()->FillEllipse(&ellipse, GetColorBrush(line.Color));
	}

	void CDirect2DWrite::DrawSolidRectangle(const SolidLine& line, const D2D1_RECT_F& rc)
	{
		auto rect = LayoutRound(rc);

		//GetD2DDeviceContext()->DrawRectangle(LayoutRound(rect), GetColorBrush(line.Color), line.Width);
		GetD2DDeviceContext()->DrawLine(CPointF(rect.left, rect.top), CPointF(rect.right, rect.top), GetColorBrush(line.Color), line.Width);
		GetD2DDeviceContext()->DrawLine(CPointF(rect.right, rect.top), CPointF(rect.right, rect.bottom), GetColorBrush(line.Color), line.Width);
		GetD2DDeviceContext()->DrawLine(CPointF(rect.right, rect.bottom), CPointF(rect.left, rect.bottom), GetColorBrush(line.Color), line.Width);
		GetD2DDeviceContext()->DrawLine(CPointF(rect.left, rect.bottom), CPointF(rect.left, rect.top), GetColorBrush(line.Color), line.Width);

	}

	void CDirect2DWrite::DrawSolidRoundedRectangle(const SolidLine& line, const D2D1_RECT_F& rect, FLOAT radiusX, FLOAT radiusY)
	{
		auto roundedrect = D2D1_ROUNDED_RECT{LayoutRound(rect), radiusX, radiusY};

		GetD2DDeviceContext()->DrawRoundedRectangle(roundedrect, GetColorBrush(line.Color), line.Width);
	}

	void CDirect2DWrite::FillSolidRectangle(const SolidFill& fill, const D2D1_RECT_F& rect)
	{
		GetD2DDeviceContext()->FillRectangle(LayoutRound(rect), GetColorBrush(fill.Color));
	}

	void CDirect2DWrite::FillSolidRoundedRectangle(const SolidFill& fill, const D2D1_RECT_F& rect, FLOAT radiusX, FLOAT radiusY)
	{
		auto roundedrect = D2D1_ROUNDED_RECT{LayoutRound(rect), radiusX, radiusY};

		GetD2DDeviceContext()->FillRoundedRectangle(roundedrect, GetColorBrush(fill.Color));
	}

	void CDirect2DWrite::DrawSolidGeometry(const SolidLine& line, CComPtr<ID2D1PathGeometry>& path)
	{
		GetD2DDeviceContext()->DrawGeometry(path, GetColorBrush(line.Color), line.Width);
	}
	void CDirect2DWrite::FillSolidGeometry(const SolidFill& fill, CComPtr<ID2D1PathGeometry>& path)
	{
		GetD2DDeviceContext()->FillGeometry(path, GetColorBrush(fill.Color));
	}

	void CDirect2DWrite::DrawIcon(HICON hIcon, CRectF& rect)
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
		HRESULT hr = GetD2DDeviceContext()->CreateBitmapFromWicBitmap(pWICBitmap, bitmapProps, &pBitmap);
		if (FAILED(hr)) {
			LOG_THIS_1("Failed");
			LOG_THIS_1(fmt::format("{:08X}", (LONG_PTR)hIcon));
			LOG_THIS_1(fmt::format("{:08X}", hr));

			throw std::exception(FILE_LINE_FUNC);
		}

		GetD2DDeviceContext()->DrawBitmap(pBitmap, rect);
	}

	void CDirect2DWrite::DrawBitmap(const CComPtr<ID2D1Bitmap>& pBitmap, const CRectF& rect)
	{
		if (pBitmap) {
			auto bmpRect = CRectF(
				std::round(rect.left),
				std::round(rect.top),
				std::round(rect.right),
				std::round(rect.bottom));
			GetD2DDeviceContext()->DrawBitmap(pBitmap, bmpRect);
		}
	}
	void CDirect2DWrite::DrawBitmap(const CComPtr<ID2D1Bitmap1>& pBitmap, const CRectF& rect)
	{
		if (pBitmap) {
			auto bmpRect = CRectF(
				std::round(rect.left),
				std::round(rect.top),
				std::round(rect.right),
				std::round(rect.bottom));
			GetD2DDeviceContext()->DrawBitmap(pBitmap, bmpRect);
		}
	}

	std::vector<CRectF> CDirect2DWrite::CalcCharRects(const FormatF& format, const std::wstring& text, const CSizeF& size)
	{
		if (text.empty()) {
			return std::vector<CRectF>();
		} else {
			return CalcCharRects(GetTextLayout(format, text, size), text.size());
		}
	}

	std::vector<CRectF> CDirect2DWrite::CalcCharRects(const CComPtr<IDWriteTextLayout>& pTextLayout, const size_t& count)
	{
		std::vector<CRectF> rects;
		for (size_t i = 0; i < count; i++) {
			float x, y;
			DWRITE_HIT_TEST_METRICS tm;
			pTextLayout->HitTestTextPosition(i, false, &x, &y, &tm);
			rects.emplace_back(tm.left, tm.top, tm.left + tm.width, tm.top + tm.height);
		}
		return rects;
	}

	std::vector<CRectF> CDirect2DWrite::CalcCharRects(const CComPtr<IDWriteTextLayout1>& pTextLayout, const size_t& count)
	{
		std::vector<CRectF> rects;
		for (size_t i = 0; i < count; i++) {
			float x, y;
			DWRITE_HIT_TEST_METRICS tm;
			pTextLayout->HitTestTextPosition(i, false, &x, &y, &tm);
			rects.emplace_back(tm.left, tm.top, tm.left + tm.width, tm.top + tm.height);
		}
		return rects;
	}

	void CDirect2DWrite::PushAxisAlignedClip(const CRectF& clipRect, D2D1_ANTIALIAS_MODE antialiasMode)
	{
		auto rect = LayoutRound(clipRect);
		rect.InflateRect(0.5f);
		GetD2DDeviceContext()->PushAxisAlignedClip(rect, antialiasMode);
	}
	
	void CDirect2DWrite::PopAxisAlignedClip()
	{
		GetD2DDeviceContext()->PopAxisAlignedClip();
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
		GetD2DFactory() = nullptr;
		GetDWriteFactory() = nullptr;
		GetWICImagingFactory() = nullptr;
		//m_pHwndRenderTarget = nullptr;
		GetD2DDeviceContext() = nullptr;
		m_solidColorBrushMap.clear();
		m_textFormatMap.clear();
		m_textLayoutMap.clear();
		m_defaultHeightMap.clear();

		m_xPixels2Dips = 0.0f;
		m_yPixels2Dips = 0.0f;

		m_pIconCache->Clear();
	}
