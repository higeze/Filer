#include "D2DImage.h"

const CComPtr<IWICBitmapDecoder>& CD2DImage::GetDecoder() const
{
    if (!m_optDecoder.has_value()) {
        CComPtr<IWICBitmapDecoder> pDecoder;
        FAILED_THROW(GetFactory()->CreateDecoderFromFilename(
            GetPath().c_str()
            , NULL
            , GENERIC_READ
            , WICDecodeMetadataCacheOnLoad
            , &pDecoder
        ));
        m_optDecoder.emplace(pDecoder);
    }
    return m_optDecoder.value();
}

const CComPtr<IWICBitmapFrameDecode>& CD2DImage::GetFrameDecode() const
{
    if (!m_optFrameDecode.has_value()) {
        CComPtr<IWICBitmapFrameDecode> pFrameDecode = NULL;
        FAILED_THROW(GetDecoder()->GetFrame( 0, &pFrameDecode ));
        m_optFrameDecode.emplace(pFrameDecode);
    }
    return m_optFrameDecode.value();
}

const CSizeU& CD2DImage::GetSizeU() const
{
	if (!m_optSizeU.has_value()) {
        UINT width, height;
        FAILED_THROW(GetFrameDecode()->GetSize(&width, &height));
        m_optSizeU.emplace(width, height);
	}
	return m_optSizeU.value();
}

const CSizeF CD2DImage::GetSizeF() const
{
    return CSizeU2CSizeF(GetSizeU());
}

const CComPtr<IWICImagingFactory2>& CD2DImage::GetFactory() const
{
	if (!m_optFactory.has_value()) {
        CComPtr<IWICImagingFactory2> pFactory;
        FAILED_THROW(pFactory.CoCreateInstance(
            CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER));
        m_optFactory.emplace(pFactory);
	}
    return m_optFactory.value();
}
//
//const CComPtr<IWICBitmapScaler>& CD2DImage::GetScaler() const
//{
//    if (!m_optScaler.has_value()) {
//        CComPtr<IWICBitmapScaler> pScaler;
//        FAILED_THROW(GetFactory()->CreateBitmapScaler(&pScaler));
//        m_optScaler.emplace(pScaler);
//    }
//    return m_optScaler.value();
//}
//
//const CComPtr<IWICBitmapClipper>& CD2DImage::GetClipper() const
//{
//    if (!m_optClipper.has_value()) {
//        CComPtr<IWICBitmapClipper> pClipper;
//        FAILED_THROW(GetFactory()->CreateBitmapClipper(&pClipper));
//        m_optClipper.emplace(pClipper);
//    }
//    return m_optClipper.value();
//}
//
//const CComPtr<IWICFormatConverter>& CD2DImage::GetFormatConverter() const
//{
//    if (!m_optFormatConverter.has_value()) {
//        CComPtr<IWICFormatConverter> pFormatConverter;
//        FAILED_THROW(GetFactory()->CreateFormatConverter(&pFormatConverter));
//        m_optFormatConverter.emplace(pFormatConverter);
//    }
//    return m_optFormatConverter.value();
//}
//

CComPtr<ID2D1Bitmap1> CD2DImage::GetBitmap(const CComPtr<ID2D1DeviceContext>& pContext, const FLOAT& scale) const
{
    CSizeU size(GetSizeU());
    CComPtr<IWICBitmapScaler> pScaler;
    FAILED_THROW(GetFactory()->CreateBitmapScaler(&pScaler));
    FAILED_THROW(pScaler->Initialize(
        GetFrameDecode(), 
        static_cast<UINT>(std::round(size.width * scale)),
        static_cast<UINT>(std::round(size.height * scale)),
        WICBitmapInterpolationModeNearestNeighbor));
    
    CComPtr<IWICFormatConverter> pFormatConverter;
    FAILED_THROW(GetFactory()->CreateFormatConverter( &pFormatConverter ));
    FAILED_THROW(pFormatConverter->Initialize(
        pScaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeCustom));

	CComPtr<ID2D1Bitmap1> m_pBitmap;
    FAILED_THROW(pContext->CreateBitmapFromWicBitmap(
        pFormatConverter, 
        nullptr, //D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE, D2D1::PixelFormat(DXGI_FORMAT_BC1_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
        &m_pBitmap ));

    return m_pBitmap;

    //CComPtr<IWICBitmapDecoder> pWICBitmapDecoder = NULL;
    //FAILED_THROW(pDirect->GetWICImagingFactory()->CreateDecoderFromFilename(
    //    m_path.c_str()
    //    , NULL
    //    , GENERIC_READ
    //    , WICDecodeMetadataCacheOnLoad
    //    , &pWICBitmapDecoder
    //));
    //CComPtr<IWICBitmapFrameDecode> pWICBitmapFrame = NULL;
    //FAILED_THROW(pWICBitmapDecoder->GetFrame( 0, &pWICBitmapFrame ));
    //UINT width, height;
    //FAILED_THROW(pWICBitmapFrame->GetSize(&width, &height))
    //CComPtr<IWICBitmapScaler> pScaler = NULL;
    //FAILED_THROW(pDirect->GetWICImagingFactory()->CreateBitmapScaler(&pScaler));
    //FAILED_THROW(pScaler->Initialize(
    //    pWICBitmapFrame, 
    //    width / 10,
    //    height / 10,
    //    WICBitmapInterpolationModeNearestNeighbor));

    //CComPtr<IWICFormatConverter> pFormatConverter = NULL;
    //FAILED_THROW(pDirect->GetWICImagingFactory()->CreateFormatConverter( &pFormatConverter ));
    //FAILED_THROW(pFormatConverter->Initialize(
    //    pScaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeCustom));

    //FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(
    //    pFormatConverter, 
    //    nullptr, //D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE, D2D1::PixelFormat(DXGI_FORMAT_BC1_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
    //    &m_pBitmap ));

}

CComPtr<ID2D1Bitmap1> CD2DImage::GetClipBitmap(const CComPtr<ID2D1DeviceContext>& pContext, const FLOAT& scale, const CRectU& rcClip, std::function<bool()> cancel) const
{
    CSizeU size(GetSizeU());

    //Scale
    if (cancel && cancel()) { return nullptr; }
    CComPtr<IWICBitmapScaler> pScaler;
    FAILED_THROW(GetFactory()->CreateBitmapScaler(&pScaler));
    FAILED_THROW(pScaler->Initialize(
        GetFrameDecode(), 
        static_cast<UINT>(std::round(size.width * scale)),
        static_cast<UINT>(std::round(size.height * scale)),
        WICBitmapInterpolationModeNearestNeighbor));

    //Clip
    if (cancel && cancel()) { return nullptr; }
    CComPtr<IWICBitmapClipper> pClipper;
    FAILED_THROW(GetFactory()->CreateBitmapClipper(&pClipper));
    WICRect rcwicClip{
        .X = static_cast<INT>(rcClip.left),
        .Y = static_cast<INT>(rcClip.top),
        .Width = static_cast<INT>(rcClip.Width()),
        .Height = static_cast<INT>(rcClip.Height())};
    FAILED_THROW(pClipper->Initialize(
        pScaler, 
        &rcwicClip));
    //Convert
    if (cancel && cancel()) { return nullptr; }
    CComPtr<IWICFormatConverter> pFormatConverter;
    FAILED_THROW(GetFactory()->CreateFormatConverter( &pFormatConverter ));
    FAILED_THROW(pFormatConverter->Initialize(
        pClipper, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeCustom));
    //D2D
    if (cancel && cancel()) { return nullptr; }
    CComPtr<ID2D1Bitmap1> m_pBitmap;
    FAILED_THROW(pContext->CreateBitmapFromWicBitmap(
        pFormatConverter, 
        nullptr, //D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE, D2D1::PixelFormat(DXGI_FORMAT_BC1_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
        &m_pBitmap ));

    return m_pBitmap;
}

void CD2DImage::Open(const std::wstring& path)
{
    CShellFile::Load(path);

    m_optDecoder.reset();
    m_optFrameDecode.reset();
    m_optSizeU.reset();
    //CComPtr<IWICBitmapDecoder> pWICBitmapDecoder = NULL;
    //FAILED_THROW(pDirect->GetWICImagingFactory()->CreateDecoderFromFilename(
    //    m_path.c_str()
    //    , NULL
    //    , GENERIC_READ
    //    , WICDecodeMetadataCacheOnLoad
    //    , &pWICBitmapDecoder
    //));
    //CComPtr<IWICBitmapFrameDecode> pWICBitmapFrame = NULL;
    //FAILED_THROW(pWICBitmapDecoder->GetFrame( 0, &pWICBitmapFrame ));
    //UINT width, height;
    //FAILED_THROW(pWICBitmapFrame->GetSize(&width, &height))
    //CComPtr<IWICBitmapScaler> pScaler = NULL;
    //FAILED_THROW(pDirect->GetWICImagingFactory()->CreateBitmapScaler(&pScaler));
    //FAILED_THROW(pScaler->Initialize(
    //    pWICBitmapFrame, 
    //    width / 10,
    //    height / 10,
    //    WICBitmapInterpolationModeNearestNeighbor));

    //CComPtr<IWICFormatConverter> pFormatConverter = NULL;
    //FAILED_THROW(pDirect->GetWICImagingFactory()->CreateFormatConverter( &pFormatConverter ));
    //FAILED_THROW(pFormatConverter->Initialize(
    //    pScaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeCustom));

    //FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(
    //    pFormatConverter, 
    //    nullptr, //D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE, D2D1::PixelFormat(DXGI_FORMAT_BC1_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
    //    &m_pBitmap ));
}

void CD2DImage::Close() 
{
    CShellFile::ResetOpts();

    m_optDecoder.reset();
    m_optFrameDecode.reset();
    m_optSizeU.reset();
}

void CD2DImage::Save(const std::wstring& path) {}