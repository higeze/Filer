#pragma once
#include <memory>
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <fpdf_text.h>
#include <fpdf_ppo.h>
#include <fpdf_save.h>
#include <fpdf_thumbnail.h>
#pragma comment(lib, "pdfium.dll.lib")

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#include <d2d1_1.h>
#include <D2d1_1helper.h>
#include <dwrite_1.h>
#include <d3d11_1.h>
#include <wincodec.h>

#include "atlcomcli.h"

#include "Debug.h"
#include "MyWin32.h"
#include "MyCom.h"
#include "D2DWTypes.h"
#include "MyUniqueHandle.h"


#include "UniqueFPdfDocument.h"
#include "UniqueFPdfPage.h"
#include "UniqueFPdfTextPage.h"

/********************/
/* CPDFiumFileWrite */
/********************/
class CPDFiumFileWrite : public FPDF_FILEWRITE
{
private:
	FILE* m_pFile = nullptr;
public:
	CPDFiumFileWrite(const std::string& path)
	{
		fopen_s(&m_pFile, path.c_str(), "wb");

		version = 1;
		WriteBlock = [](FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)->int
		{
			return fwrite(pData, 1, size, static_cast<CPDFiumFileWrite*>(pThis)->m_pFile);
		};
	}
	CPDFiumFileWrite(const std::wstring& path)
	{
		_wfopen_s(&m_pFile, path.c_str(), L"wb");

		version = 1;
		WriteBlock = [](FPDF_FILEWRITE* pThis, const void* pData, unsigned long size)->int
		{
			return fwrite(pData, 1, size, static_cast<CPDFiumFileWrite*>(pThis)->m_pFile);
		};
	}

	virtual ~CPDFiumFileWrite()
	{
		fclose(m_pFile);
	}
};

/**********************/
/* CPDFiumMultiThread */
/**********************/
class CPDFiumMultiThread
{
private:
    /*************/
    /* PDFObject */
    /*************/
    struct PDFObject
    {
        PDFObject(std::mutex& pdf_mutex):Mutex(pdf_mutex){}
        std::mutex& Mutex;
        CUniqueFPdfDocument Doc;
        //UNQ_FPDF_DOCUMENT& GetDocPtr() { return Doc; }
        std::vector<CUniqueFPdfPage> Pages;
        std::vector<CUniqueFPdfTextPage> TextPages;
        //std::vector<UNQ_FPDF_SCHHANDLE> SchHandles;
        void Clear();
        void UpdatePages();
        unsigned long LoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password);
        unsigned long CreateDocument();
        int GetPageCount();
        FPDF_BOOL GetFileVersion(int* fileVersion);
        float GetPageWidthF(int page_index);
        float GetPageHeightF(int page_index);
        int Page_GetRotation(int page_index);
        void Page_SetRotation(int page_index, int rotate);
        int Text_CountChars(int index);
        //Text_CountRects & Text_GetRect should be called at same sequence. Not available for multithread case.
        //int Text_CountRects(int index,
        //    int start_index,
        //    int count);
        int Text_GetText(int index, int start_index, int count, unsigned short* result);

        int Text_GetCharIndexAtPos(int index, double x, double y, double xTolerance, double yTolerance);

        //FPDF_BOOL Text_GetRect(int index,
        //    int rect_index,
        //    double* left,
        //    double* top,
        //    double* right,
        //    double* bottom);

        std::vector<CRectF> Text_GetRects(int index);
        std::vector<CRectF> Text_GetRangeRects(int index, int begin, int end);

        UHBITMAP Bitmap_GetPageBitmap(
            const int index,
            HDC hDC,
            const FLOAT& scale);

        //CComPtr<ID2D1Bitmap1> Bitmap_GetPageBitmap(const int index,
        //    HDC hDC,
        //    CComPtr<ID2D1DeviceContext>& pDC,
        //    CComPtr<IWICImagingFactory2>& pFactory,
        //    const FLOAT& scale);

        UHBITMAP Bitmap_GetPageClippedBitmap(
            const int index,
            HDC hDC,
            const CRectF& rectInPage,
            const FLOAT& scale);

        //CComPtr<ID2D1Bitmap1> Bitmap_GetPageClippedBitmap(const int index,
        //    HDC hDC,
        //    CComPtr<ID2D1DeviceContext>& pDC,
        //    CComPtr<IWICImagingFactory2>& pFactory,
        //    const CRectF& rectInPage,
        //    const FLOAT& scale);

        FPDF_BOOL ImportPagesByIndex(
            const CUniqueFPdfDocument& src_doc,
            const int* page_indices,
            unsigned long length,
            int index);
        FPDF_BOOL ImportPages(
            const CUniqueFPdfDocument& src_doc,
            FPDF_BYTESTRING pagerange,
            int index);

        FPDF_BOOL SaveAsCopy(
            FPDF_FILEWRITE* pFileWrite,
            FPDF_DWORD flags);

        FPDF_BOOL SaveWithVersion(
            FPDF_FILEWRITE* pFileWrite,
            FPDF_DWORD flags,
            int fileVersion);

        std::vector<std::tuple<int, int, std::vector<CRectF>>> Text_FindResults(int index,
            const std::wstring& find_string);


    };

private:
	// need to keep track of threads so we can join them
	std::vector<std::thread> workers;
    // the single task queue
	std::queue< std::function<void(PDFObject&)> > tasks;
	// specific task queue
    std::unordered_map<std::thread::id, std::queue<std::function<void(PDFObject&)>>> spetasks;
    // active thread count
	std::atomic<int> activeCount = 0;
	// synchronization
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;
    // mutex for pdf
    std::mutex pdf_mutex;
public:
	CPDFiumMultiThread(size_t = 3/*std::thread::hardware_concurrency()*/);
	~CPDFiumMultiThread();

	int GetTotalThreadCount() const { return workers.size(); }
	int GetActiveThreadCount() const { return activeCount.load(); }
    int GetQueuedTaskCount() const { return tasks.size(); }
    std::vector<int> GetQueuedSpecificTaskCounts() const
    {
        std::vector<int> result;
        std::transform(spetasks.cbegin(), spetasks.cend(), std::back_inserter(result),
            [](const std::pair<std::thread::id, std::queue<std::function<void(PDFObject&)>>>& pair) {
            return pair.second.size();
        });
        return result;
    }

	//template<class F, class... Args>
	//auto single_enqueue(F&& f, Args&&... args)
	//	-> std::future<typename std::invoke_result_t<F, PDFObject&, Args...>>
	//{
	//	using return_type = typename std::invoke_result_t<F, PDFObject&, Args...>;

	//	auto task = std::make_shared< std::packaged_task<return_type(PDFObject&)> >(
	//		std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)
	//		);

	//	std::future<return_type> res = task->get_future();
	//	{
	//		std::unique_lock<std::mutex> lock(queue_mutex);

	//		// don't allow enqueueing after stopping the pool
 //           if (stop) {
 //               THROW_FILE_LINE_FUNC;
 //           }

	//		tasks.emplace([task](PDFObject& pdf) { (*task)(pdf); });
	//	}
	//	condition.notify_one();
	//	return res;
	//}

 //   template<class F, class... Args>
	//auto specific_enqueue(std::thread::id id, F&& f, Args&&... args)
	//	-> std::future<typename std::invoke_result_t<F, PDFObject&, Args...>>
	//{
	//	using return_type = typename std::invoke_result_t<F, PDFObject&, Args...>;

	//	auto task = std::make_shared< std::packaged_task<return_type(PDFObject&)> >(
	//		std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...));

	//	std::future<return_type> res = task->get_future();
	//	{
	//		std::unique_lock<std::mutex> lock(queue_mutex);

	//		// don't allow enqueueing after stopping the pool
 //           if (stop) {
 //               THROW_FILE_LINE_FUNC;
 //           }

	//		spetasks[id].emplace([task](PDFObject& pdf) { (*task)(pdf); });
	//	}
	//	condition.notify_one();
	//	return res;
	//}

 //   template<class F, class... Args>
	//auto all_enqueue(F&& f, Args&&... args)
	//	-> std::vector<std::future<typename std::invoke_result_t<F, PDFObject&, Args...>>>
	//{
	//	using return_type = typename std::invoke_result_t<F, PDFObject&, Args...>;

 //       std::vector<std::future<typename std::invoke_result_t<F, PDFObject&, Args...>>> futures;
 //       std::for_each(workers.begin(), workers.end(),
 //           [&](const std::thread& th) {

	//	    auto task = std::make_shared< std::packaged_task<return_type(PDFObject&)> >(
	//		    std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...));

	//	        futures.emplace_back(task->get_future());
	//	        {
	//		        std::unique_lock<std::mutex> lock(queue_mutex);

	//		        // don't allow enqueueing after stopping the pool
 //                   if (stop) {
 //                       THROW_FILE_LINE_FUNC;
 //                   }

 //                   spetasks[th.get_id()].emplace([task](PDFObject& pdf) { (*task)(pdf); });
	//	        }
 //           });
	//	condition.notify_all();
	//	return futures;
	//}

	//template<class F, class... Args>
 //   auto single_run(F&& f, Args&&... args)
 //       -> std::invoke_result_t<F, PDFObject&, Args...>
 //   {
 //       if (std::any_of(workers.cbegin(), workers.cend(), [](const std::thread& th)->bool{ return std::this_thread::get_id() == th.get_id(); })){
 //           auto future = specific_enqueue(std::this_thread::get_id(), std::forward<F>(f), std::forward<Args>(args)...);
 //           return future.get();
 //       } else {
 //           auto future = single_enqueue(std::forward<F>(f), std::forward<Args>(args)...);
 //           return future.get();
 //       }
 //   }

	//template<class F, class... Args>
 //   auto specific_run(std::thread::id id, F&& f, Args&&... args)
 //       -> std::invoke_result_t<F, PDFObject&, Args...>
 //   {
 //       if (std::this_thread::get_id() == id){
 //           auto future = specific_enqueue(id,  std::forward<F>(f), std::forward<Args>(args)...);
 //           return future.get();
 //       } else {
 //           auto future = specific_enqueue(id, std::forward<F>(f) ,std::forward<Args>(args)...);
 //           return future.get();
 //       }
 //   }


	//template<class F, class... Args,  std::enable_if_t<!std::is_void_v<std::invoke_result_t<F, PDFObject&, Args...>>, std::nullptr_t> = nullptr>
 //   auto all_run(F&& f, Args&&... args)
 //       -> std::vector<std::invoke_result_t<F, PDFObject&, Args...>>
 //   {
 //       if (std::any_of(workers.cbegin(), workers.cend(), [](const std::thread& th)->bool{ return std::this_thread::get_id() == th.get_id(); })){
 //           throw std::exception("Cannot call this function from thread.");
 //       } else {
 //           auto futures = all_enqueue(std::forward<F>(f), std::forward<Args>(args)...);
 //           std::vector<std::invoke_result_t<F, PDFObject&, Args...>> results;
 //           std::for_each(futures.begin(), futures.end(),
 //               [&](auto& ftr) {
 //                   results.emplace_back(ftr.get());
 //               });
 //           return results;
 //       }
 //   }

	//template<class F, class... Args,  std::enable_if_t<std::is_void_v<std::invoke_result_t<F, PDFObject&, Args...>>, std::nullptr_t> = nullptr>
 //   auto all_run(F&& f, Args&&... args)
 //       -> void
 //   {
 //       if (std::any_of(workers.cbegin(), workers.cend(), [](const std::thread& th)->bool{ return std::this_thread::get_id() == th.get_id(); })){
 //           throw std::exception("Cannot call this function from thread.");
 //       } else {
 //           auto futures = all_enqueue(std::forward<F>(f), std::forward<Args>(args)...);
 //           std::for_each(futures.begin(), futures.end(),
 //               [](auto& ftr) {
 //                   ftr.get();
 //               });
 //       }
 //   }

    /*****************/
    /* FPDF_DOCUMENT */
    /*****************/
PDFObject Pdf;
public:
    unsigned long LoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password)
    {
        return Pdf.LoadDocument(file_path, password);
        //return all_run(std::bind(&PDFObject::LoadDocument, std::placeholders::_1, file_path, password))[0];
    }

    unsigned long  CreateDocument()
    {
        return Pdf.CreateDocument();
        //return all_run(std::bind(&PDFObject::CreateDocument, std::placeholders::_1))[0];
    }

    //UNQ_FPDF_DOCUMENT& GetDocPtr()
    //{
    //    return single_run(std::bind(&PDFObject::GetDocPtr, std::placeholders::_1));
    //}

    int GetPageCount()
    {
        return Pdf.GetPageCount();
        //return single_run(std::bind(&PDFObject::GetPageCount, std::placeholders::_1));
    }

    FPDF_BOOL GetFileVersion(int* fileVersion)
    {
        return Pdf.GetFileVersion(fileVersion);
        //return single_run(std::bind(&PDFObject::GetFileVersion, std::placeholders::_1, fileVersion));
    }

    FPDF_BOOL ImportPagesByIndex(CPDFiumMultiThread& src_doc,
                            const int* page_indices,
                            unsigned long length,
                            int index)
    {//TODOTODO
        return 0;
        //std::vector<FPDF_BOOL> results = all_run(std::bind(&PDFObject::ImportPagesByIndex, std::placeholders::_1, src_doc.GetDocPtr().get(), page_indices, length, index));
        //return FPDF_BOOL(std::all_of(results.cbegin(), results.cend(), [](const FPDF_BOOL& result)->bool { return result; }));
    }

    FPDF_BOOL ImportPages(CPDFiumMultiThread& src_doc,
                        FPDF_BYTESTRING pagerange,
                        int index)
    {//TODOTODO
        return 0;
        //std::vector<FPDF_BOOL> results = all_run(std::bind(&PDFObject::ImportPages, std::placeholders::_1, src_doc.GetDocPtr().get(), pagerange, index));
        //return FPDF_BOOL(std::all_of(results.cbegin(), results.cend(), [](const FPDF_BOOL& result)->bool{ return result; }));
    }

    FPDF_BOOL SaveAsCopy(FPDF_FILEWRITE* pFileWrite,
                        FPDF_DWORD flags)
    {
        return Pdf.SaveAsCopy(pFileWrite, flags);
        //return single_run(std::bind(&PDFObject::SaveAsCopy, std::placeholders::_1, pFileWrite, flags));
    }

    FPDF_BOOL SaveWithVersion(FPDF_FILEWRITE* pFileWrite,
                     FPDF_DWORD flags,
                     int fileVersion)
    {
        return Pdf.SaveWithVersion(pFileWrite, flags, fileVersion);
        //return single_run(std::bind(&PDFObject::SaveWithVersion, std::placeholders::_1, pFileWrite, flags, fileVersion));
    }
    /*************/
    /* FPDF_PAGE */
    /*************/
    float GetPageWidthF(int page_index)
    {
        return Pdf.GetPageWidthF(page_index);
        //return single_run(std::bind(&PDFObject::GetPageWidthF, std::placeholders::_1, page_index));
    }
    float GetPageHeightF(int page_index)
    {
        return Pdf.GetPageHeightF(page_index);
        //return single_run(std::bind(&PDFObject::GetPageHeightF, std::placeholders::_1, page_index));
    }

    int Page_GetRotation(int page_index)
    {
        return Pdf.Page_GetRotation(page_index);
        //return single_run(std::bind(&PDFObject::Page_GetRotation, std::placeholders::_1, page_index));
    }

    void Page_SetRotation(int page_index, int rotate)
    {
        return Pdf.Page_SetRotation(page_index, rotate);
        //return single_run(std::bind(&PDFObject::Page_SetRotation, std::placeholders::_1, page_index, rotate));
    }

    //FPDF_BITMAP Page_GetThumbnailAsBitmap(int page_index)
    //{
    //    return single_run([&](PDFObject& pdf, int page_index) {
    //        return FPDFPage_GetThumbnailAsBitmap(pdf.Pages[page_index].get());
    //    }, page_index);
    //}

    //UNQ_FPDF_BITMAP Page_GetThumbnailAsUnqBitmap(int page_index)
    //{
    //    std::thread::id id = std::this_thread::get_id();
    //    return single_run([&](PDFObject& pdf, int page_index) {
    //        return UNQ_FPDF_BITMAP(FPDFPage_GetThumbnailAsBitmap(pdf.Pages[page_index].get()),
    //            [this, id](FPDF_BITMAP p) { specific_run(id,
    //            [](PDFObject& pdf, FPDF_BITMAP p) {
    //            return FPDFBitmap_Destroy(p);
    //        }, p); });
    //        }, page_index);
    //}

    /***************/
    /* FPDF_BITMAP */
    /***************/
public:
    UHBITMAP Bitmap_GetPageBitmap(
        const int index,
        HDC hDC, 
        const FLOAT& scale)
    {
        return Pdf.Bitmap_GetPageBitmap(index, hDC, scale);
        //return single_run(std::bind(&PDFObject::Bitmap_GetPageBitmap, std::placeholders::_1, index, hDC, scale));
    }

    //CComPtr<ID2D1Bitmap1> Bitmap_GetPageBitmap(const int index,
    //    HDC hDC, 
    //    CComPtr<ID2D1DeviceContext>& pDC,
    //    CComPtr<IWICImagingFactory2>& pFactory,
    //    const FLOAT& scale)
    //{
    //    return single_run(std::bind(&PDFObject::Bitmap_GetPageBitmap, std::placeholders::_1, index, hDC, pDC, pFactory, scale));
    //}

    UHBITMAP Bitmap_GetPageClippedBitmap(
        const int index,
        HDC hDC, 
        const CRectF& rectInPage, 
        const FLOAT& scale)
    {
        return Pdf.Bitmap_GetPageClippedBitmap(index, hDC, rectInPage, scale);
        //return single_run(std::bind(&PDFObject::Bitmap_GetPageClippedBitmap, std::placeholders::_1, index, hDC, rectInPage, scale));
    }

    //CComPtr<ID2D1Bitmap1> Bitmap_GetPageClippedBitmap(const int index,
    //    HDC hDC, 
    //    CComPtr<ID2D1DeviceContext>& pDC,
    //    CComPtr<IWICImagingFactory2>& pFactory,
    //    const CRectF& rectInPage, 
    //    const FLOAT& scale)
    //{
    //    return single_run(std::bind(&PDFObject::Bitmap_GetPageClippedBitmap, std::placeholders::_1, index, hDC, pDC, pFactory, rectInPage, scale));
    //}

    /*****************/
    /* FPDF_TEXTPAGE */
    /*****************/
    int Text_CountChars(int index)
    {
        return Pdf.Text_CountChars(index);
        //return single_run(std::bind(&PDFObject::Text_CountChars, std::placeholders::_1, index));
    }
    //int Text_CountRects(int index,
    //                    int start_index,
    //                    int count)
    //{
    //    return single_run(std::bind(&PDFObject::Text_CountRects, std::placeholders::_1, index, start_index, count));
    //}
    int Text_GetText(int index, int start_index, int count, unsigned short* result)
    {
        return Pdf.Text_GetText(index, start_index, count, result);
        //return single_run(std::bind(&PDFObject::Text_GetText, std::placeholders::_1, index, start_index, count, result));
    }

    int Text_GetCharIndexAtPos(int index, double x, double y, double xTolerance, double yTolerance)
    {
        return Pdf.Text_GetCharIndexAtPos(index, x, y, xTolerance, yTolerance);
        //return single_run(std::bind(&PDFObject::Text_GetCharIndexAtPos, std::placeholders::_1, index, x, y, xTolerance, yTolerance));
    }

    //FPDF_BOOL Text_GetRect(int index,
    //                        int rect_index,
    //                        double* left,
    //                        double* top,
    //                        double* right,
    //                        double* bottom)
    //{
    //    return single_run(std::bind(&PDFObject::Text_GetRect, std::placeholders::_1, index, rect_index, left, top, right, bottom));
    //}

    std::vector<CRectF> Text_GetRects(int index)
    {
        return Pdf.Text_GetRects(index);
        //return single_run(std::bind(&PDFObject::Text_GetRects, std::placeholders::_1, index));
    }

    std::vector<CRectF> Text_GetRangeRects(int index, int begin, int end)
    {
        return Pdf.Text_GetRangeRects(index, begin, end);
        //return single_run(std::bind(&PDFObject::Text_GetRangeRects, std::placeholders::_1, index, begin, end));
    }

    /******************/
    /* FPDF_SCHHANDLE */
    /******************/
    std::vector<std::tuple<int, int, std::vector<CRectF>>> Text_FindResults(int index,
        const std::wstring& find_string)
    {
        return Pdf.Text_FindResults(index, find_string);
        //return single_run(std::bind(&PDFObject::Text_FindResults, std::placeholders::_1, index, find_string));
    }

//private:
//    FPDF_SCHHANDLE Text_FindStart(FPDF_TEXTPAGE text_page,
//                       FPDF_WIDESTRING findwhat,
//                       unsigned long flags,
//                       int start_index)
//    {
//        return ThreadRun(FPDFText_FindStart, text_page, findwhat, flags, start_index);
//    }
//    void Text_FindClose(FPDF_SCHHANDLE handle)
//    {
//        return ThreadRun(FPDFText_FindClose, handle);
//    }
//public:
//    UNQ_FPDF_SCHHANDLE Text_UnqFindStart(FPDF_TEXTPAGE text_page,
//                                        FPDF_WIDESTRING findwhat,
//                                        unsigned long flags,
//                                        int start_index)
//    {
//        return UNQ_FPDF_SCHHANDLE(
//            Text_FindStart(text_page, findwhat, flags, start_index),
//            [this](FPDF_SCHHANDLE p) { if (p) { return Text_FindClose(p); } });
//    }
//
//    FPDF_BOOL Text_FindNext(FPDF_SCHHANDLE handle)
//    {
//        return ThreadRun(FPDFText_FindNext, handle);
//    }
//
//    int Text_GetSchResultIndex(FPDF_SCHHANDLE handle)
//    {
//        return ThreadRun(FPDFText_GetSchResultIndex, handle);
//    }
//    int Text_GetSchCount(FPDF_SCHHANDLE handle)
//    {
//        return ThreadRun(FPDFText_GetSchCount, handle);
//    }




};





