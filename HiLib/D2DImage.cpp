#include "D2DImage.h"

void CD2DImage::Open(CDirect2DWrite* pDirect, const std::wstring& path)
{
    m_path = path;
    CComPtr<IWICBitmapDecoder> pWICBitmapDecoder = NULL;
    FAILED_THROW(pDirect->GetWICImagingFactory()->CreateDecoderFromFilename(
        m_path.c_str()
        , NULL
        , GENERIC_READ
        , WICDecodeMetadataCacheOnLoad
        , &pWICBitmapDecoder
    ));
    CComPtr<IWICBitmapFrameDecode> pWICBitmapFrame = NULL;
    FAILED_THROW(pWICBitmapDecoder->GetFrame( 0, &pWICBitmapFrame ));
    CComPtr<IWICFormatConverter> pFormatConverter = NULL;
    FAILED_THROW(pDirect->GetWICImagingFactory()->CreateFormatConverter( &pFormatConverter ));
    FAILED_THROW(pFormatConverter->Initialize(
        pWICBitmapFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeCustom));
    FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap(
        pFormatConverter, 
        nullptr, //D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE, D2D1::PixelFormat(DXGI_FORMAT_BC1_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
        &m_pBitmap ));
}

void CD2DImage::Close() 
{
    m_pBitmap.Release();
}

void CD2DImage::Save(const std::wstring& path) {}