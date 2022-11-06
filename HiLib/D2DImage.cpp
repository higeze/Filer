#include "D2DImage.h"

const CComPtr<ID2D1Bitmap> CD2DImage::GetBitmapPtr(CDirect2DWrite* pDirect) const
{
    if (!m_pBitmap || m_isPathChanged || m_pCurDirect != pDirect) {
        IWICBitmapDecoder* pWICBitmapDecoder = NULL;
        FAILED_THROW(pDirect->GetWICImagingFactory()->CreateDecoderFromFilename(
            m_path.c_str()
            , NULL
            , GENERIC_READ
            , WICDecodeMetadataCacheOnLoad
            , &pWICBitmapDecoder
        ));
        IWICBitmapFrameDecode* pWICBitmapFrame = NULL;
        FAILED_THROW(pWICBitmapDecoder->GetFrame( 0, &pWICBitmapFrame ));
        IWICFormatConverter* pFormatConverter = NULL;
        FAILED_THROW(pDirect->GetWICImagingFactory()->CreateFormatConverter( &pFormatConverter ));
        FAILED_THROW(pFormatConverter->Initialize(
            pWICBitmapFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeMedianCut));
        FAILED_THROW(pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap( pFormatConverter, NULL, &m_pBitmap ));
        m_pCurDirect = pDirect;
        m_isPathChanged = false;
    }
    return m_pBitmap;
}

void CD2DImage::Save(const std::wstring& path) {}