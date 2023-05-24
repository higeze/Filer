#include "UniqueFPdfPage.h"
#include "UniqueFPdfBitmap.h"
#include "UniqueFpdfTextPage.h"
#include "Debug.h"

UHBITMAP CUniqueFPdfPage::GetBitmap(HDC hDC, const float& scale)
{
	CSizeF sz(GetPageWidthF() * scale, GetPageHeightF() * scale);

	BITMAPINFOHEADER bmih
	{
		.biSize = sizeof(BITMAPINFOHEADER),
		.biWidth = static_cast<LONG>(std::round(sz.width)),
		.biHeight = -static_cast<LONG>(std::round(sz.height)),
		.biPlanes = 1,
		.biBitCount = 32,
		.biCompression = BI_RGB,
		.biSizeImage = 0,
	};

	void* bitmapBits = nullptr;

	UHBITMAP phBmp(::CreateDIBSection(hDC, reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0));
	FALSE_THROW(phBmp);

	CUniqueFPdfBitmap fpdfBmp;
	fpdfBmp.CreateEx(bmih.biWidth, -bmih.biHeight, FPDFBitmap_BGRx, bitmapBits, ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3));
    FALSE_THROW(fpdfBmp);

	fpdfBmp.FillRect(0, 0, bmih.biWidth, -bmih.biHeight, 0xFFFFFFFF); // Fill white
	fpdfBmp.RenderPageBitmap(*this, 0, 0, bmih.biWidth, -bmih.biHeight, 0, FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH | FPDF_RENDER_LIMITEDIMAGECACHE);

	return phBmp;
}

UHBITMAP CUniqueFPdfPage::GetClippedBitmap(HDC hDC, const float& scale, const CRectF& rectInPage)
{
    CRectU scaledRectInPage = CRectF2CRectU(rectInPage * scale);
    BITMAPINFOHEADER bmih
    {
        .biSize = sizeof(BITMAPINFOHEADER),
        .biWidth = static_cast<LONG>(scaledRectInPage.Width()),
        .biHeight = -static_cast<LONG>(scaledRectInPage.Height()),
        .biPlanes = 1,
        .biBitCount = 32,
        .biCompression = BI_RGB,
        .biSizeImage = 0,
    };

    if (bmih.biWidth == 0 || bmih.biHeight == 0) {
        return nullptr;
	} else {

		void* bitmapBits = nullptr;

		UHBITMAP phBmp(::CreateDIBSection(hDC, reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0));
		FALSE_THROW(phBmp);

		CUniqueFPdfBitmap fpdfBmp;
		fpdfBmp.CreateEx(bmih.biWidth, -bmih.biHeight, FPDFBitmap_BGRx, bitmapBits, ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3));
		FALSE_THROW(fpdfBmp);

		fpdfBmp.FillRect(0, 0, bmih.biWidth, -bmih.biHeight, 0xFFFFFFFF); // Fill white
		FS_MATRIX mat{scale, 0.f, 0.f, scale, -static_cast<float>(scaledRectInPage.left), -static_cast<float>(scaledRectInPage.top)};
		FS_RECTF rcf{0, 0, static_cast<float>(bmih.biWidth), static_cast<float>(-bmih.biHeight)};
		fpdfBmp.RenderPageBitmapWithMatrix(*this, &mat, &rcf, FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH | FPDF_RENDER_LIMITEDIMAGECACHE);

		return phBmp;
	}
}

CUniqueFPdfTextPage CUniqueFPdfPage::LoadTextPage() const
{
	FPDF_LOCK;
	return CUniqueFPdfTextPage(FPDFText_LoadPage(get()));
}