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
        pWICBitmapFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeMedianCut));
    FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap( pFormatConverter, NULL, &m_pBitmap ));
}

void CD2DImage::Close() 
{
    m_pBitmap.Release();
}

void CD2DImage::Save(const std::wstring& path) {}