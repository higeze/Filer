#include "D2DImage.h"


CD2DImage::CD2DImage(const std::wstring& path)
    :CShellFile(path), Dummy(std::make_shared<int>(0)), Rotate(WICBitmapTransformOptions::WICBitmapTransformRotate0)
{}

const CComPtr<IWICBitmapSource>& CD2DImage::GetBitmapSourcePtr() const
{
    if (!m_pBitmapSource) {
        CComPtr<IWICBitmapDecoder> pDecoder;
        FAILED_THROW(CWICImagingFactory::GetInstance()->CreateDecoderFromFilename(
            GetPath().c_str(),
            NULL,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            &pDecoder
        ));

        CComPtr<IWICBitmapFrameDecode> pFrameDecode;
        FAILED_THROW(pDecoder->GetFrame(0, &pFrameDecode));

        CComPtr<IWICBitmapFlipRotator> pFlipRotator;
        FAILED_THROW(CWICImagingFactory::GetInstance()->CreateBitmapFlipRotator(&pFlipRotator));
        FAILED_THROW(pFlipRotator->Initialize(
            pFrameDecode,
            *Rotate
        ));

        m_pBitmapSource = pFlipRotator; 
    }
    return m_pBitmapSource;
}

const CSizeU& CD2DImage::GetSizeU() const
{
	if (!m_optSizeU.has_value()) {
        UINT width, height;
        FAILED_THROW(GetBitmapSourcePtr()->GetSize(&width, &height));
        m_optSizeU.emplace(width, height);
	}
	return m_optSizeU.value();
}

const CSizeF CD2DImage::GetSizeF() const
{
    return CSizeU2CSizeF(GetSizeU());
}

CComPtr<IWICBitmapSource> CD2DImage::GetBitmap(const FLOAT& scale, std::function<bool()> cancel) const
{
    CSizeU size(GetSizeU());

    //Scale
    if (cancel && cancel()) { return nullptr; }
    CComPtr<IWICBitmapScaler> pScaler;
    FAILED_THROW(CWICImagingFactory::GetInstance()->CreateBitmapScaler(&pScaler));
    FAILED_THROW(pScaler->Initialize(
        GetBitmapSourcePtr(), 
        static_cast<UINT>(std::round(size.width * scale)),
        static_cast<UINT>(std::round(size.height * scale)),
        WICBitmapInterpolationModeNearestNeighbor));

    pScaler->GetSize(&size.width, &size.height);
    
    //Convert
    if (cancel && cancel()) { return nullptr; }
    CComPtr<IWICFormatConverter> pFormatConverter;
    FAILED_THROW(CWICImagingFactory::GetInstance()->CreateFormatConverter( &pFormatConverter ));
    FAILED_THROW(pFormatConverter->Initialize(
        pScaler, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeCustom));

    return CComPtr<IWICBitmapSource>(pFormatConverter);
}

CComPtr<IWICBitmapSource> CD2DImage::GetClipBitmap(const FLOAT& scale, const CRectU& rcClip, std::function<bool()> cancel) const
{
    CSizeU size(GetSizeU());

    //Scale
    if (cancel && cancel()) { return nullptr; }
    CComPtr<IWICBitmapScaler> pScaler;
    FAILED_THROW(CWICImagingFactory::GetInstance()->CreateBitmapScaler(&pScaler));
    FAILED_THROW(pScaler->Initialize(
        GetBitmapSourcePtr(), 
        static_cast<UINT>(std::round(size.width * scale)),
        static_cast<UINT>(std::round(size.height * scale)),
        WICBitmapInterpolationModeNearestNeighbor));

    //Clip
    if (cancel && cancel()) { return nullptr; }
    CComPtr<IWICBitmapClipper> pClipper;
    FAILED_THROW(CWICImagingFactory::GetInstance()->CreateBitmapClipper(&pClipper));
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
    FAILED_THROW(CWICImagingFactory::GetInstance()->CreateFormatConverter( &pFormatConverter ));
    FAILED_THROW(pFormatConverter->Initialize(
        pClipper, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 1.0f, WICBitmapPaletteTypeCustom));

    return CComPtr<IWICBitmapSource>(pFormatConverter);
}

void CD2DImage::Open(const std::wstring& path)
{
	Rotate.subscribe([this](auto value) 
	{
        ReleaseComPtrs();
        m_optSizeU.reset();
	},Dummy);

    CShellFile::Load(path);

    ReleaseComPtrs();
    m_optSizeU.reset();
}

void CD2DImage::Close() 
{
    CShellFile::ResetOpts();

    ReleaseComPtrs();
    m_optSizeU.reset();
}

void CD2DImage::Save(const std::wstring& path) {}