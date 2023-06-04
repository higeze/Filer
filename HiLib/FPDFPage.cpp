#include "FPdfPage.h"
#include "FPdfBitmap.h"
#include "FpdfTextPage.h"
#include "Debug.h"

UHBITMAP CFPDFPage::GetBitmap(HDC hDC, const float& scale, std::function<bool()> cancel)
{
	do {
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
		FALSE_BREAK(phBmp);

		CFPDFBitmap fpdfBmp;
		FALSE_BREAK(fpdfBmp.CreateEx(bmih.biWidth, -bmih.biHeight, FPDFBitmap_BGRx, bitmapBits, ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3), cancel));
		FALSE_BREAK(fpdfBmp);

		FALSE_BREAK(fpdfBmp.FillRect(0, 0, bmih.biWidth, -bmih.biHeight, 0xFFFFFFFF, cancel)); // Fill white
		FALSE_BREAK(fpdfBmp.RenderPageBitmap(*this, 0, 0, bmih.biWidth, -bmih.biHeight, 0, FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH | FPDF_RENDER_LIMITEDIMAGECACHE, cancel));

		return phBmp;
	} while (1);

	return nullptr;
}

UHBITMAP CFPDFPage::GetClippedBitmap(HDC hDC, const float& scale, const CRectF& rectInPage, std::function<bool()> cancel)
{
	do {
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

		TRUE_BREAK(bmih.biWidth == 0 || bmih.biHeight == 0);

		void* bitmapBits = nullptr;

		UHBITMAP phBmp(::CreateDIBSection(hDC, reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0));
		FALSE_BREAK(phBmp);

		CFPDFBitmap fpdfBmp;
		fpdfBmp.CreateEx(bmih.biWidth, -bmih.biHeight, FPDFBitmap_BGRx, bitmapBits, ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3));
		FALSE_BREAK(fpdfBmp);

		FALSE_BREAK(fpdfBmp.FillRect(0, 0, bmih.biWidth, -bmih.biHeight, 0xFFFFFFFF)); // Fill white
		FS_MATRIX mat{scale, 0.f, 0.f, scale, -static_cast<float>(scaledRectInPage.left), -static_cast<float>(scaledRectInPage.top)};
		FS_RECTF rcf{0, 0, static_cast<float>(bmih.biWidth), static_cast<float>(-bmih.biHeight)};
		FALSE_BREAK(fpdfBmp.RenderPageBitmapWithMatrix(*this, &mat, &rcf, FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH | FPDF_RENDER_LIMITEDIMAGECACHE));

		return phBmp;
	} while (1);

	return nullptr;
}

CFPDFTextPage CFPDFPage::LoadTextPage() const
{
	FPDF_LOCK;
	return CFPDFTextPage(FPDFText_LoadPage(m_p.get()));
}