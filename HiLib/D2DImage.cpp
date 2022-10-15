#include "D2DImage.h"

//void CD2DImage::Open(const std::wstring& path)
//{
//IWICBitmapDecoder* pWICBitmapDecoder = NULL;
//FAILED_THROW(m_pDirect->GetWICImagingFactory()->CreateDecoderFromFilename(
//    m_path.c_str()
//    , NULL
//    , GENERIC_READ
//    , WICDecodeMetadataCacheOnLoad
//    , &pWICBitmapDecoder
//));
//IWICBitmapFrameDecode* pWICBitmapFrame = NULL;
//FAILED_THROW(pWICBitmapDecoder->GetFrame( 0, &pWICBitmapFrame ));
//IWICFormatConverter* pFormatConverter = NULL;
//FAILED_THROW(m_pDirect->GetWICImagingFactory()->CreateFormatConverter( &pFormatConverter ));
//FAILED_THROW(pFormatConverter->Initialize(
//    pWICBitmapFrame, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeMedianCut));
//FAILED_THROW(m_pDirect->GetD2DDeviceContext()->CreateBitmapFromWicBitmap( pFormatConverter, NULL, &m_pBitmap ));
//
//}
//
//void CD2DImage::Save(const std::wstring& path) {}