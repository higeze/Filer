#include "PDFiumMultiThread.h"
#include <boost/algorithm/string.hpp>
#include "se_exception.h"

/*************/
/* PDFObject */
/*************/

void CPDFiumMultiThread::PDFObject::Clear()
{
	Doc.reset();
	Pages.clear();
	TextPages.clear();
}

void CPDFiumMultiThread::PDFObject::UpdatePages()
{
	Pages.clear();
	TextPages.clear();
	int count = Doc.GetPageCount();
	for (auto i = 0; i < count; i++) {
		CUniqueFPdfPage page(Doc.LoadPage(i));
		CUniqueFPdfTextPage textPage(page.LoadTextPage());
		Pages.emplace_back(std::move(page));
		TextPages.emplace_back(std::move(textPage));
	}
}

unsigned long CPDFiumMultiThread::PDFObject::LoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password)
{
	std::lock_guard<std::mutex> lock(Mutex);
    
	Clear();
    Doc.LoadDocument(file_path, password);
    unsigned long err = FPDF_GetLastError();
    if (!Doc) {
        return err;
	} else {
		UpdatePages();
		return FPDF_ERR_SUCCESS;
	}
}

unsigned long CPDFiumMultiThread::PDFObject::CreateDocument()
{
	Clear();
	Doc.CreateNewDocument();
	unsigned long err = FPDF_GetLastError();
	if (!Doc) {
		return err;
	} else {
		return FPDF_ERR_SUCCESS;
	}
}

int CPDFiumMultiThread::PDFObject::GetPageCount()
{
	return Doc.GetPageCount();
}

FPDF_BOOL CPDFiumMultiThread::PDFObject::GetFileVersion(int* fileVersion)
{
	return Doc.GetFileVersion(fileVersion);
}

float CPDFiumMultiThread::PDFObject::GetPageWidthF(int page_index)
{
	return Pages[page_index].GetPageWidthF();
}
float CPDFiumMultiThread::PDFObject::GetPageHeightF(int page_index)
{
	return Pages[page_index].GetPageHeightF();
}

int CPDFiumMultiThread::PDFObject::Page_GetRotation(int page_index)
{
	return Pages[page_index].GetRotation();
}

void CPDFiumMultiThread::PDFObject::Page_SetRotation(int page_index, int rotate)
{
	return Pages[page_index].SetRotation(rotate);
}

int CPDFiumMultiThread::PDFObject::Text_CountChars(int index)
{
    return TextPages[index].CountChars();
}

int CPDFiumMultiThread::PDFObject::Text_GetText(int index, int start_index, int count, unsigned short* result)
{
	return TextPages[index].GetText(start_index, count, result);
}

int CPDFiumMultiThread::PDFObject::Text_GetCharIndexAtPos(int index, double x, double y, double xTolerance, double yTolerance)
{
	return TextPages[index].GetCharIndexAtPos(x, y, xTolerance, yTolerance);
}

std::vector<CRectF> CPDFiumMultiThread::PDFObject::Text_GetRects(int index)
{
	return TextPages[index].GetRects();
}

std::vector<CRectF> CPDFiumMultiThread::PDFObject::Text_GetRangeRects(int index, int begin, int end)
{
	return TextPages[index].GetRangeRects(begin, end);
}

UHBITMAP CPDFiumMultiThread::PDFObject::Bitmap_GetPageBitmap(
	const int index,
	HDC hDC,
	const FLOAT& scale)
{
	return Pages[index].GetBitmap(hDC, scale);
}

UHBITMAP CPDFiumMultiThread::PDFObject::Bitmap_GetPageClippedBitmap(
	const int index,
	HDC hDC,
	const CRectF& rectInPage,
	const FLOAT& scale)
{
	return Pages[index].GetClippedBitmap(hDC, scale, rectInPage);
}

FPDF_BOOL CPDFiumMultiThread::PDFObject::ImportPagesByIndex(
	const CUniqueFPdfDocument& src_doc,
	const int* page_indices,
	unsigned long length,
	int index)
{		
	FPDF_BOOL ret = Doc.ImportPagesByIndex(src_doc, page_indices, length, index);
	if (ret) {
		UpdatePages();
	}
	return ret;
}

FPDF_BOOL CPDFiumMultiThread::PDFObject::ImportPages(
	const CUniqueFPdfDocument& src_doc,
	FPDF_BYTESTRING pagerange,
	int index)
{
	FPDF_BOOL ret = Doc.ImportPages(src_doc, pagerange, index);
	if (ret) {
		UpdatePages();
	}
	return ret;
}

FPDF_BOOL CPDFiumMultiThread::PDFObject::SaveAsCopy(FPDF_FILEWRITE* pFileWrite,
	FPDF_DWORD flags)
{
	return Doc.SaveAsCopy(pFileWrite, flags);
}

FPDF_BOOL CPDFiumMultiThread::PDFObject::SaveWithVersion(FPDF_FILEWRITE* pFileWrite,
	FPDF_DWORD flags,
	int fileVersion)
{
	return Doc.SaveWithVersion(pFileWrite, flags, fileVersion);
}

std::vector<std::tuple<int, int, std::vector<CRectF>>> CPDFiumMultiThread::PDFObject::Text_FindResults(int page_index,
	const std::wstring& find_string)
{
	auto find = boost::trim_copy(find_string);
	if (!find.empty()) {

		FPDF_WIDESTRING text = reinterpret_cast<FPDF_WIDESTRING>(find.c_str());
		return TextPages[page_index].SearchResults(text);
	} else {
		return std::vector<std::tuple<int, int, std::vector<CRectF>>>();
	}
}

/**********************/
/* CPDFiumMultiThread */
/**********************/

// the constructor just launches some amount of workers
CPDFiumMultiThread::CPDFiumMultiThread(size_t threads)
	: stop(false), spetasks(threads), Pdf(pdf_mutex)
{
	threads = 1;
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
	for (std::thread& worker : workers) {
		worker.join();
	}
}