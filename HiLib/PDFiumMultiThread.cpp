#include "PDFiumMultiThread.h"
#include <boost/algorithm/string.hpp>
#include "se_exception.h"
#include "MyUniqueHandle.h"

/*************/
/* PDFObject */
/*************/

unsigned long CPDFiumMultiThread::PDFObject::LoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password)
{
	std::lock_guard<std::mutex> lock(Mutex);
    //DOCUMENT
    Doc = std::move(UNQ_FPDF_DOCUMENT(FPDF_LoadDocument(file_path, password)));
    unsigned long err = FPDF_GetLastError();
    if (!Doc) {
        return err;
	} else {
		//PAGES, TEXTPAGES
		int count = FPDF_GetPageCount(Doc.get());
		for (auto i = 0; i < count; i++) {
			auto pPage = FPDF_LoadPage(Doc.get(), i);
			Pages.emplace_back(pPage);
			TextPages.emplace_back(FPDFText_LoadPage(pPage));
		}
		return FPDF_ERR_SUCCESS;
	}
}

unsigned long CPDFiumMultiThread::PDFObject::CreateDocument()
{
	Doc = std::move(UNQ_FPDF_DOCUMENT(FPDF_CreateNewDocument()));
	unsigned long err = FPDF_GetLastError();
	if (!Doc) {
		return err;
	} else {
		return FPDF_ERR_SUCCESS;
	}
}

int CPDFiumMultiThread::PDFObject::GetPageCount()
{
	return FPDF_GetPageCount(Doc.get());
}

FPDF_BOOL CPDFiumMultiThread::PDFObject::GetFileVersion(int* fileVersion)
{
	return FPDF_GetFileVersion(Doc.get(), fileVersion);
}

float CPDFiumMultiThread::PDFObject::GetPageWidthF(int page_index)
{
	return FPDF_GetPageWidthF(Pages[page_index].get());
}
float CPDFiumMultiThread::PDFObject::GetPageHeightF(int page_index)
{
	return FPDF_GetPageHeightF(Pages[page_index].get());
}

int CPDFiumMultiThread::PDFObject::Page_GetRotation(int page_index)
{
	return FPDFPage_GetRotation(Pages[page_index].get());
}

void CPDFiumMultiThread::PDFObject::Page_SetRotation(int page_index, int rotate)
{
	return FPDFPage_SetRotation(Pages[page_index].get(), rotate);
}

int CPDFiumMultiThread::PDFObject::Text_CountChars(int index)
{
    return FPDFText_CountChars(TextPages[index].get());
}
int CPDFiumMultiThread::PDFObject::Text_CountRects(int index,
                    int start_index,
                    int count)
{
    return FPDFText_CountRects(TextPages[index].get(), start_index, count);
}
int CPDFiumMultiThread::PDFObject::Text_GetText(int index, int start_index, int count, unsigned short* result)
{
	return FPDFText_GetText(TextPages[index].get(), start_index, count, result);
}

int CPDFiumMultiThread::PDFObject::Text_GetCharIndexAtPos(int index, double x, double y, double xTolerance, double yTolerance)
{
	return FPDFText_GetCharIndexAtPos(TextPages[index].get(), x, y, xTolerance, yTolerance);
}

FPDF_BOOL CPDFiumMultiThread::PDFObject::Text_GetRect(int index,
                        int rect_index,
                        double* left,
                        double* top,
                        double* right,
                        double* bottom)
{
	return FPDFText_GetRect(TextPages[index].get(), rect_index, left, top, right, bottom);
}

CComPtr<ID2D1Bitmap1> CPDFiumMultiThread::PDFObject::Bitmap_GetPageBitmap(const int index,
	HDC hDC,
	CComPtr<ID2D1DeviceContext>& pDC,
	CComPtr<IWICImagingFactory2>& pFactory,
	const FLOAT& scale)
{
	CSizeF sz(FPDF_GetPageWidthF(this->Pages[index].get()) * scale, FPDF_GetPageHeightF(this->Pages[index].get()) * scale);
	BITMAPINFOHEADER bmih
	{
		.biSize = sizeof(BITMAPINFOHEADER),
		.biWidth = static_cast<LONG>(sz.width),
		.biHeight = -static_cast<LONG>(sz.height),
		.biPlanes = 1,
		.biBitCount = 32,
		.biCompression = BI_RGB,
		.biSizeImage = 0,
	};

	void* bitmapBits = nullptr;

	UHBITMAP phBmp(::CreateDIBSection(hDC, reinterpret_cast<const BITMAPINFO*>(&bmih), DIB_RGB_COLORS, &bitmapBits, nullptr, 0));
	FALSE_THROW(phBmp);

	UNQ_FPDF_BITMAP pFpdfBmp(FPDFBitmap_CreateEx(bmih.biWidth, -bmih.biHeight, FPDFBitmap_BGRx, bitmapBits, ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3)));
    FALSE_THROW(pFpdfBmp);

	FPDFBitmap_FillRect(pFpdfBmp.get(), 0, 0, bmih.biWidth, -bmih.biHeight, 0xFFFFFFFF); // Fill white
	FPDF_RenderPageBitmap(pFpdfBmp.get(), this->Pages[index].get(),0,0,bmih.biWidth, -bmih.biHeight, FPDFPage_GetRotation(this->Pages[index].get()), FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);

    CComPtr<IWICBitmap> pWICBmp;
	FAILED_THROW(pFactory->CreateBitmapFromHBITMAP(phBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBmp));

	CComPtr<ID2D1Bitmap1> pD2DBmp;
	FAILED_THROW(pDC->CreateBitmapFromWicBitmap(pWICBmp, &pD2DBmp));
	
	return pD2DBmp;
}

CComPtr<ID2D1Bitmap1> CPDFiumMultiThread::PDFObject::Bitmap_GetPageClippedBitmap(const int index,
	HDC hDC,
	CComPtr<ID2D1DeviceContext>& pDC,
	CComPtr<IWICImagingFactory2>& pFactory,
	const CRectF& rectInPage,
	const FLOAT& scale)
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

		UNQ_FPDF_BITMAP pFpdfBmp(FPDFBitmap_CreateEx(bmih.biWidth, -bmih.biHeight, FPDFBitmap_BGRx, bitmapBits, ((((bmih.biWidth * bmih.biBitCount) + 31) & ~31) >> 3)));
		FALSE_THROW(pFpdfBmp);

		FPDFBitmap_FillRect(pFpdfBmp.get(), 0, 0, bmih.biWidth, -bmih.biHeight, 0xFFFFFFFF); // Fill white
		FS_MATRIX mat{scale, 0.f, 0.f, scale, -static_cast<float>(scaledRectInPage.left), -static_cast<float>(scaledRectInPage.top)};
		FS_RECTF rcf{0, 0, static_cast<float>(bmih.biWidth), static_cast<float>(-bmih.biHeight)};
		FPDF_RenderPageBitmapWithMatrix(pFpdfBmp.get(), this->Pages[index].get(), &mat, &rcf, FPDF_ANNOT | FPDF_LCD_TEXT | FPDF_NO_CATCH);

		BITMAP bmp;
		::GetObject(phBmp.get(), sizeof(BITMAP), LPVOID(&bmp));

		CComPtr<IWICBitmap> pWICBmp;
		FAILED_THROW(pFactory->CreateBitmapFromHBITMAP(phBmp.get(), nullptr, WICBitmapIgnoreAlpha, &pWICBmp));

		CComPtr<ID2D1Bitmap1> pD2DBmp;
		FAILED_THROW(pDC->CreateBitmapFromWicBitmap(pWICBmp, &pD2DBmp));

		return pD2DBmp;
	}
}

FPDF_BOOL CPDFiumMultiThread::PDFObject::ImportPagesByIndex(FPDF_DOCUMENT src_doc,
	const int* page_indices,
	unsigned long length,
	int index)
{
	return FPDF_ImportPagesByIndex(Doc.get(), src_doc, page_indices, length, index);
}

FPDF_BOOL CPDFiumMultiThread::PDFObject::ImportPages(FPDF_DOCUMENT src_doc,
	FPDF_BYTESTRING pagerange,
	int index)
{
	return FPDF_ImportPages(Doc.get(), src_doc, pagerange, index);
}

FPDF_BOOL CPDFiumMultiThread::PDFObject::SaveAsCopy(FPDF_FILEWRITE* pFileWrite,
	FPDF_DWORD flags)
{
	return FPDF_SaveAsCopy(Doc.get(), pFileWrite, flags);
}

FPDF_BOOL CPDFiumMultiThread::PDFObject::SaveWithVersion(FPDF_FILEWRITE* pFileWrite,
	FPDF_DWORD flags,
	int fileVersion)
{
	return FPDF_SaveWithVersion(Doc.get(), pFileWrite, flags, fileVersion);
}

std::vector<std::tuple<int, int, std::vector<CRectF>>> CPDFiumMultiThread::PDFObject::Text_FindResults(int index,
	const std::wstring& find_string)
{
	auto find = boost::trim_copy(find_string);
	std::vector<std::tuple<int, int, std::vector<CRectF>>> results;
	if (find.empty()) {
	} else {
		FPDF_WIDESTRING text = reinterpret_cast<FPDF_WIDESTRING>(find.c_str());
		UNQ_FPDF_SCHHANDLE pSchHdl(FPDFText_FindStart(TextPages[index].get(), text, 0, 0));
		while (FPDFText_FindNext(pSchHdl.get())) {
			int index = FPDFText_GetSchResultIndex(pSchHdl.get());
			int ch_count = FPDFText_GetSchCount(pSchHdl.get());
			int rc_count = FPDFText_CountRects(TextPages[index].get(), index, ch_count);
			std::vector<CRectF> rects;
			for (int i = 0; i < rc_count; i++) {
				double left, top, right, bottom;
				FPDFText_GetRect(
					TextPages[index].get(),
					i,
					&left,
					&top,
					&right,
					&bottom);
				rects.emplace_back(
					static_cast<FLOAT>(left),
					static_cast<FLOAT>(top),
					static_cast<FLOAT>(right),
					static_cast<FLOAT>(bottom));
			}
			results.emplace_back(index, ch_count, rects);
		}
	}
	return results;
}

/**********************/
/* CPDFiumMultiThread */
/**********************/

// the constructor just launches some amount of workers
CPDFiumMultiThread::CPDFiumMultiThread(size_t threads)
	: stop(false), spetasks(threads)
{
	for (size_t i = 0; i < threads; ++i) {
		workers.emplace_back([this]
		{
			 //CoInitialize
			 CCoInitializer coinit;
			 //Catch SEH exception as CEH
			 scoped_se_translator se_trans;
			 //ID
			 std::thread::id id = std::this_thread::get_id();
			 //PDF
			 PDFObject pdf(pdf_mutex);
			 for (;;) {
				 std::function<void(PDFObject&)> task;
				 {
					 std::unique_lock<std::mutex> lock(this->queue_mutex);
					 this->condition.wait(lock,
						 [this, id] { return this->stop || !this->tasks.empty() || !this->spetasks[id].empty(); });
					 if (this->stop && this->tasks.empty() && this->spetasks[id].empty()) {
						 return;
					 }else if (!spetasks[id].empty()) {
						 task = std::move(this->spetasks[id].front());
						 this->spetasks[id].pop();
					 } else if (!tasks.empty()) {
						 task = std::move(this->tasks.front());
						 this->tasks.pop();
					 }
				 }

				 activeCount++;
				 try {
					 task(pdf);
				 }
				 catch (std::exception& ex) {
					 std::string msg = fmt::format(
						 "What:{}\r\n"
						 "Last Error:{}\r\n",
						 ex.what(), GetLastErrorString());

					 ::MessageBoxA(nullptr, msg.c_str(), "Exception in Thread Pool", MB_ICONWARNING);
				 }
				 catch (...) {
					 std::string msg = fmt::format(
						 "Last Error:{}",
						 GetLastErrorString());

					 MessageBoxA(nullptr, msg.c_str(), "Unknown Exception in Thread Pool", MB_ICONWARNING);
				 }
				 activeCount--;
			 }
		});
	}
}

// the destructor joins all threads
CPDFiumMultiThread::~CPDFiumMultiThread()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (std::thread &worker : workers)
		worker.join();
}