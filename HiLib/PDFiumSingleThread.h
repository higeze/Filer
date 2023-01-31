#pragma once
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <fpdf_text.h>
#include <fpdf_ppo.h>
#include <fpdf_save.h>
#include <fpdf_thumbnail.h>
#pragma comment(lib, "pdfium.dll.lib")

#include "ThreadPool.h"

class CThreadPool;

using UNQ_FPDF_DOCUMENT = std::unique_ptr<std::remove_pointer_t<FPDF_DOCUMENT>, std::function<void(FPDF_DOCUMENT)>>;
using UNQ_FPDF_PAGE = std::unique_ptr<std::remove_pointer_t<FPDF_PAGE>, std::function<void(FPDF_PAGE)>>;
using UNQ_FPDF_BITMAP = std::unique_ptr<std::remove_pointer_t<FPDF_BITMAP>, std::function<void(FPDF_BITMAP)>>;
using UNQ_FPDF_TEXTPAGE = std::unique_ptr<std::remove_pointer_t<FPDF_TEXTPAGE>, std::function<void(FPDF_TEXTPAGE)>>;
using UNQ_FPDF_SCHHANDLE = std::unique_ptr<std::remove_pointer_t<FPDF_SCHHANDLE>, std::function<void(FPDF_SCHHANDLE)>>;

class CPDFException : public std::exception
{
private:
    unsigned long m_error;
public:
    CPDFException(unsigned long error, const char* what)
        :std::exception(what), m_error(error) {}
    unsigned long GetError()const { return m_error; }
};

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

class CPDFiumSingleThread
{
private:
    std::unique_ptr<CThreadPool> m_pPool;

public:
    CPDFiumSingleThread()
	    :m_pPool(std::make_unique<CThreadPool>(1)){}

    std::unique_ptr<CThreadPool>& GetThreadPtr() { return m_pPool; }

	template<class F, class... Args>
    auto ThreadRun(F&& f, Args&&... args)
        -> std::invoke_result_t<F, Args...>
    {
        if (std::this_thread::get_id() == GetThreadPtr()->GetThreadId(0)) {
            return f(args...);
        } else {
            auto future = GetThreadPtr()->enqueue(f, args...);
            return future.get();
        }
    }

    void InitLibraryWithConfig(const FPDF_LIBRARY_CONFIG* config)
    {
        return ThreadRun(FPDF_InitLibraryWithConfig, config);
    }
    void DestroyLibrary()
    {
        return ThreadRun(FPDF_DestroyLibrary);
    }
    /*********************/
    /* FPDF_GetLastError */
    /*********************/
public:
    //GetLastError need to be called just after subjected function call in same scope.
    //If split by ThreadRun, GetLastError couldn't return Error.
    //unsigned long GetLastError()
    //{
    //    return ThreadRun(FPDF_GetLastError);
    //}
    /*****************/
    /* FPDF_DOCUMENT */
    /*****************/
private:
    FPDF_DOCUMENT LoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password)
    {
        return ThreadRun(FPDF_LoadDocument, file_path, password);
    }
    FPDF_DOCUMENT CreateDocument()
    {
        return ThreadRun(FPDF_CreateNewDocument);
    }
    FPDF_DOCUMENT LoadDocumentEx(FPDF_STRING file_path, FPDF_BYTESTRING password)
    {
        std::exception_ptr ex_ptr;

        FPDF_DOCUMENT pDoc = ThreadRun([&](FPDF_STRING file_path, FPDF_BYTESTRING password)
        {
            try {
                FPDF_DOCUMENT p = FPDF_LoadDocument(file_path, password);
                if (!p) {
                    auto err = FPDF_GetLastError();

                    switch (err) {
                        case FPDF_ERR_SUCCESS:
                            throw CPDFException(err, "Success");
                            break;
                        case FPDF_ERR_UNKNOWN:
                            throw CPDFException(err, "Unknown error");
                            break;
                        case FPDF_ERR_FILE:
                            throw CPDFException(err, "File not found or could not be opened");
                            break;
                        case FPDF_ERR_FORMAT:
                            throw CPDFException(err, "File not in PDF format or corrupted");
                            break;
                        case FPDF_ERR_PASSWORD:
                            throw CPDFException(err, "Password required or incorrect password");
                            break;
                        case FPDF_ERR_SECURITY:
                            throw CPDFException(err, "Unsupported security scheme");
                            break;
                        case FPDF_ERR_PAGE:
                            throw CPDFException(err, "Page not found or content error");
                            break;
                        default:
                            throw CPDFException(err, std::format("Unknown error %ld", err).c_str());
                    }
                }
                return p;                
            } catch (...) {
                ex_ptr = std::current_exception();
                return FPDF_DOCUMENT();
            }

        }, file_path, password);

        if (ex_ptr != nullptr) {
            std::rethrow_exception(ex_ptr);
        }
        
        return pDoc;
    }

    void CloseDocument(FPDF_DOCUMENT document)
    { 
        return ThreadRun(FPDF_CloseDocument, document); 
    }
public:
    UNQ_FPDF_DOCUMENT UnqLoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password)
    {
        return UNQ_FPDF_DOCUMENT(
                        LoadDocumentEx(file_path, password),
                        [this](FPDF_DOCUMENT p) { if (p) { return CloseDocument(p); } });
    }
    UNQ_FPDF_DOCUMENT UnqCreateDocument()
    {
        return UNQ_FPDF_DOCUMENT(
                        CreateDocument(),
                        [this](FPDF_DOCUMENT p) { if (p) { return CloseDocument(p); } });
    }

    int GetPageCount(FPDF_DOCUMENT document)
    {
        return ThreadRun(FPDF_GetPageCount, document);
    }

    FPDF_BOOL GetFileVersion(FPDF_DOCUMENT doc,
                                   int* fileVersion)
    {
        return ThreadRun(FPDF_GetFileVersion, doc, fileVersion);
    }

    FPDF_BOOL ImportPagesByIndex(FPDF_DOCUMENT dest_doc,
                            FPDF_DOCUMENT src_doc,
                            const int* page_indices,
                            unsigned long length,
                            int index)
    {
        return ThreadRun(FPDF_ImportPagesByIndex, dest_doc, src_doc, page_indices, length, index);
    }

    FPDF_BOOL ImportPages(FPDF_DOCUMENT dest_doc,
                        FPDF_DOCUMENT src_doc,
                        FPDF_BYTESTRING pagerange,
                        int index)
    {
        return ThreadRun(FPDF_ImportPages, dest_doc, src_doc, pagerange, index);
    }

    FPDF_BOOL SaveAsCopy(FPDF_DOCUMENT document,
                        FPDF_FILEWRITE* pFileWrite,
                        FPDF_DWORD flags)
    {
        return ThreadRun(FPDF_SaveAsCopy, document, pFileWrite, flags);
    }

    FPDF_BOOL SaveWithVersion(FPDF_DOCUMENT document,
                     FPDF_FILEWRITE* pFileWrite,
                     FPDF_DWORD flags,
                     int fileVersion)
    {
        return ThreadRun(FPDF_SaveWithVersion, document, pFileWrite, flags, fileVersion);
    }

    /*************/
    /* FPDF_PAGE */
    /*************/
private:
    FPDF_PAGE LoadPage(FPDF_DOCUMENT document, int page_index)
    {
        return ThreadRun(FPDF_LoadPage, document, page_index);
    }
    void ClosePage(FPDF_PAGE page)
    {
        return ThreadRun(FPDF_ClosePage, page);
    }
public:
    UNQ_FPDF_PAGE UnqLoadPage(FPDF_DOCUMENT document, int page_index)
    {
        return UNQ_FPDF_PAGE(
            LoadPage(document, page_index),
            [this](FPDF_PAGE p) { if (p) { return ClosePage(p); } });
    }

    double GetPageWidth(FPDF_PAGE page)
    {
        return ThreadRun(FPDF_GetPageWidth, page);
    }
    double GetPageHeight(FPDF_PAGE page)
    {
        return ThreadRun(FPDF_GetPageHeight, page);
    }

    int Page_GetRotation(FPDF_PAGE page)
    {
        return ThreadRun(FPDFPage_GetRotation, page);
    }

    void Page_SetRotation(FPDF_PAGE page, int rotate)
    {
        return ThreadRun(FPDFPage_SetRotation, page, rotate);
    }

    FPDF_BITMAP Page_GetThumbnailAsBitmap(FPDF_PAGE page)
    {
        return ThreadRun(FPDFPage_GetThumbnailAsBitmap, page);
    }

    UNQ_FPDF_BITMAP Page_GetThumbnailAsUnqBitmap(FPDF_PAGE page)
    {
        return UNQ_FPDF_BITMAP(Page_GetThumbnailAsBitmap(page),
            [this](FPDF_BITMAP p) { if (p) { return Bitmap_Destroy(p); } });
    }

    /***************/
    /* FPDF_BITMAP */
    /***************/
private:
    FPDF_BITMAP Bitmap_CreateEx(int width,
                                int height,
                                int format,
                                void* first_scan,
                                int stride)
    {
        return ThreadRun(FPDFBitmap_CreateEx, width, height, format, first_scan, stride);
    }
    void Bitmap_Destroy(FPDF_BITMAP bitmap)
    {
        return ThreadRun(FPDFBitmap_Destroy, bitmap);
    }
public:
    UNQ_FPDF_BITMAP Bitmap_UnqCreateEx(int width,
                                        int height,
                                        int format,
                                        void* first_scan,
                                        int stride)
    {
        return UNQ_FPDF_BITMAP(
            Bitmap_CreateEx(width, height, format, first_scan, stride),
            [this](FPDF_BITMAP p) { if (p) { return Bitmap_Destroy(p); } });
    }

    void Bitmap_FillRect(FPDF_BITMAP bitmap,
                        int left,
                        int top,
                        int width,
                        int height,
       
                 FPDF_DWORD color)
    {
        return ThreadRun(FPDFBitmap_FillRect, bitmap, left, top, width, height, color);
    }
    void RenderPage(HDC dc,
                    FPDF_PAGE page,
                    int start_x,
                    int start_y,
                    int size_x,
                    int size_y,
                    int rotate,
                    int flags)
    {
        return ThreadRun(FPDF_RenderPage, dc, page, start_x, start_y, size_x, size_y, rotate, flags);
    }
    void RenderPageBitmap(FPDF_BITMAP bitmap,
                                FPDF_PAGE page,
                                int start_x,
                                int start_y,
                                int size_x,
                                int size_y,
                                int rotate,
                                int flags)
    {
        return ThreadRun(FPDF_RenderPageBitmap, bitmap, page, start_x, start_y, size_x, size_y, rotate, flags);
    }

    void RenderPageBitmapWithMatrix(FPDF_BITMAP bitmap,
                                FPDF_PAGE page,
                                const FS_MATRIX* matrix,
                                const FS_RECTF* clipping,
                                int flags)
    {
        return ThreadRun(FPDF_RenderPageBitmapWithMatrix, bitmap, page, matrix, clipping, flags);
    }

    /*****************/
    /* FPDF_TEXTPAGE */
    /*****************/
private:
    FPDF_TEXTPAGE Text_LoadPage(FPDF_PAGE pPage)
    {
        return ThreadRun(FPDFText_LoadPage, pPage);
    }

    void Text_ClosePage(FPDF_TEXTPAGE pPage)
    {
        return ThreadRun(FPDFText_ClosePage, pPage);
    }
public:
    UNQ_FPDF_TEXTPAGE Text_UnqLoadPage(FPDF_PAGE pPage)
    {
        return UNQ_FPDF_TEXTPAGE(
            Text_LoadPage(pPage),
            [this](FPDF_TEXTPAGE p) { if (p) { return Text_ClosePage(p); } });
    }

    /******************/
    /* FPDF_SCHHANDLE */
    /******************/
private:
    FPDF_SCHHANDLE Text_FindStart(FPDF_TEXTPAGE text_page,
                       FPDF_WIDESTRING findwhat,
                       unsigned long flags,
                       int start_index)
    {
        return ThreadRun(FPDFText_FindStart, text_page, findwhat, flags, start_index);
    }
    void Text_FindClose(FPDF_SCHHANDLE handle)
    {
        return ThreadRun(FPDFText_FindClose, handle);
    }
public:
    UNQ_FPDF_SCHHANDLE Text_UnqFindStart(FPDF_TEXTPAGE text_page,
                                        FPDF_WIDESTRING findwhat,
                                        unsigned long flags,
                                        int start_index)
    {
        return UNQ_FPDF_SCHHANDLE(
            Text_FindStart(text_page, findwhat, flags, start_index),
            [this](FPDF_SCHHANDLE p) { if (p) { return Text_FindClose(p); } });
    }

    FPDF_BOOL Text_FindNext(FPDF_SCHHANDLE handle)
    {
        return ThreadRun(FPDFText_FindNext, handle);
    }

    int Text_GetSchResultIndex(FPDF_SCHHANDLE handle)
    {
        return ThreadRun(FPDFText_GetSchResultIndex, handle);
    }
    int Text_GetSchCount(FPDF_SCHHANDLE handle)
    {
        return ThreadRun(FPDFText_GetSchCount, handle);
    }
    int Text_CountChars(FPDF_TEXTPAGE text_page)
    {
        return ThreadRun(FPDFText_CountChars, text_page);
    }
    int Text_CountRects(FPDF_TEXTPAGE text_page,
                        int start_index,
                        int count)
    {
        return ThreadRun(FPDFText_CountRects, text_page, start_index, count);
    }
    int Text_GetText(FPDF_TEXTPAGE text_page, int start_index, int count, unsigned short* result)
    {
        return ThreadRun(FPDFText_GetText, text_page, start_index, count, result);
    }

    int Text_GetCharIndexAtPos(FPDF_TEXTPAGE text_page, double x, double y, double xTolerance, double yTolerance)
    {
        return ThreadRun(FPDFText_GetCharIndexAtPos, text_page, x, y, xTolerance, yTolerance);
    }

    FPDF_BOOL Text_GetRect(FPDF_TEXTPAGE text_page,
                            int rect_index,
                            double* left,
                            double* top,
                            double* right,
                            double* bottom)
    {
        return ThreadRun(FPDFText_GetRect, text_page, rect_index, left, top, right, bottom);
    }






};

#include "Debug.h"
#include "MyWin32.h"

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include "MyCom.h"

class CPDFiumMultiThread
{
private:
    struct PDFObject
    {
        UNQ_FPDF_DOCUMENT Doc;
        std::vector<UNQ_FPDF_PAGE> Pages;
        std::vector<UNQ_FPDF_TEXTPAGE> TextPages;
        std::vector<UNQ_FPDF_SCHHANDLE> SchHandles;

    };
private:
	// need to keep track of threads so we can join them
	std::vector<std::thread> workers;

    // the single task queue
	std::queue< std::function<void(PDFObject&)> > tasks;
	// specific task queue
    std::unordered_map<std::thread::id, std::queue<std::function<void(PDFObject&)>>> spetasks;
    // 


    //std::unordered_map<std::thread::id, UNQ_FPDF_DOCUMENT> docs;
    //std::unordered_map<std::thread::id, std::vector<UNQ_FPDF_PAGE>> pages;
    //std::unordered_map<std::thread::id, std::vector<UNQ_FPDF_TEXTPAGE>> textpages;
    //std::unordered_map<std::thread::id, std::vector<UNQ_FPDF_SCHHANDLE>> schhandles;


    // active thread count
	std::atomic<int> activeCount = 0;

	// synchronization
	std::mutex queue_mutex;
	std::condition_variable condition;
	bool stop;

public:
	CPDFiumMultiThread(size_t = std::thread::hardware_concurrency());
	~CPDFiumMultiThread();

	int GetTotalTheadCount() { return workers.size(); }
	int GetActiveThreadCount() { return activeCount.load(); }
	std::thread::id GetThreadId(int n) { return workers[n].get_id(); }

// add new work item to the pool
	template<class F, class... Args>
	auto single_enqueue(F&& f, Args&&... args)
		-> std::future<typename std::invoke_result_t<F, PDFObject&, Args...>>
	{
		using return_type = typename std::invoke_result_t<F, PDFObject&, Args...>;

		auto task = std::make_shared< std::packaged_task<return_type(PDFObject&)> >(
			std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			// don't allow enqueueing after stopping the pool
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			tasks.emplace([task](PDFObject& pdf) { (*task)(pdf); });
		}
		condition.notify_one();
		return res;
	}

    template<class F, class... Args>
	auto specific_enqueue(std::thread::id id, F&& f, Args&&... args)
		-> std::future<typename std::invoke_result_t<F, PDFObject&, Args...>>
	{
		using return_type = typename std::invoke_result_t<F, PDFObject&, Args...>;

		auto task = std::make_shared< std::packaged_task<return_type(PDFObject&)> >(
			std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...));

		std::future<return_type> res = task->get_future();
		{
			std::unique_lock<std::mutex> lock(queue_mutex);

			// don't allow enqueueing after stopping the pool
			if (stop)
				throw std::runtime_error("enqueue on stopped ThreadPool");

			spetasks[id].emplace([task](PDFObject& pdf) { (*task)(pdf); });
		}
		condition.notify_one();
		return res;
	}

    template<class F, class... Args>
	auto all_enqueue(F&& f, Args&&... args)
		-> std::vector<std::future<typename std::invoke_result_t<F, PDFObject&, Args...>>>
	{
		using return_type = typename std::invoke_result_t<F, PDFObject&, Args...>;

        std::vector<std::future<typename std::invoke_result_t<F, PDFObject&, Args...>>> futures;
        std::for_each(workers.begin(), workers.end(),
            [&](const std::thread& th) {

		    auto task = std::make_shared< std::packaged_task<return_type(PDFObject&)> >(
			    std::bind(std::forward<F>(f), std::placeholders::_1, std::forward<Args>(args)...));

		        futures.emplace_back(task->get_future());
		        {
			        std::unique_lock<std::mutex> lock(queue_mutex);

			        // don't allow enqueueing after stopping the pool
                    if (stop) {
                        throw std::runtime_error("enqueue on stopped ThreadPool");
                    }

                    spetasks[std::this_thread::get_id()].emplace([task](PDFObject& pdf) { (*task)(pdf); });
		        }
            });
		condition.notify_all();
		return futures;
	}

	template<class F, class... Args>
    auto single_run(F&& f, Args&&... args)
        -> std::invoke_result_t<F, PDFObject&, Args...>
    {
        if (std::any_of(workers.cbegin(), workers.cend(), [](const std::thread& th)->bool{ return std::this_thread::get_id() == th.get_id(); })){
            auto future = specific_enqueue(std::this_thread::get_id(), f, args...);
            return future.get();
        } else {
            auto future = single_enqueue(f, args...);
            return future.get();
        }
    }

	template<class F, class... Args>
    auto specific_run(std::thread::id id, F&& f, Args&&... args)
        -> std::invoke_result_t<F, PDFObject&, Args...>
    {
        if (std::this_thread::get_id() == id){
            auto future = specific_enqueue(id, f, args...);
            return future.get();
        } else {
            auto future = specific_enqueue(id, f, args...);
            return future.get();
        }
    }


	template<class F, class... Args,  std::enable_if_t<!std::is_void_v<std::invoke_result_t<F, PDFObject&, Args...>>, std::nullptr_t> = nullptr>
    auto all_run(F&& f, Args&&... args)
        -> std::vector<std::invoke_result_t<F, PDFObject&, Args...>>
    {
        if (std::any_of(workers.cbegin(), workers.cend(), [](const std::thread& th)->bool{ return std::this_thread::get_id() == th.get_id(); })){
            throw std::exception("Cannot call this function from thread.");
        } else {
            auto futures = all_enqueue(f, args...);
            std::vector<std::invoke_result_t<F, PDFObject&, Args...>> results;
            std::for_each(futures.begin(), futures.end(),
                [&](auto& ftr) {
                    results.emplace_back(ftr.get());
                });
            return results;
        }
    }

	template<class F, class... Args,  std::enable_if_t<std::is_void_v<std::invoke_result_t<F, PDFObject&, Args...>>, std::nullptr_t> = nullptr>
    auto all_run(F&& f, Args&&... args)
        -> void
    {
        if (std::any_of(workers.cbegin(), workers.cend(), [](const std::thread& th)->bool{ return std::this_thread::get_id() == th.get_id(); })){
            throw std::exception("Cannot call this function from thread.");
        } else {
            auto futures = all_enqueue(f, args...);
            std::for_each(futures.begin(), futures.end(),
                [](auto& ftr) {
                    ftr.get();
                });
        }
    }

    /****************/
    /* INIT DESTROY */
    /****************/
public:
    void InitLibraryWithConfig(const FPDF_LIBRARY_CONFIG* config)
    {
        single_run([](PDFObject&, const FPDF_LIBRARY_CONFIG* config) { return FPDF_InitLibraryWithConfig(config); }, config);
    }
    void DestroyLibrary()
    {
        single_run([](PDFObject&, void*){ return FPDF_DestroyLibrary(); }, nullptr);
    }

    /*****************/
    /* FPDF_DOCUMENT */
    /*****************/
public:
    void LoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password)
    {
        std::exception_ptr ex_ptr;
        all_run([&](PDFObject& pdf, FPDF_STRING file_path, FPDF_BYTESTRING password)->void 
        {
            try {
                //DOCUMENT
                FPDF_DOCUMENT p = FPDF_LoadDocument(file_path, password);
                if (!p) {
                    auto err = FPDF_GetLastError();

                    switch (err) {
                        case FPDF_ERR_SUCCESS:
                            throw CPDFException(err, "Success");
                            break;
                        case FPDF_ERR_UNKNOWN:
                            throw CPDFException(err, "Unknown error");
                            break;
                        case FPDF_ERR_FILE:
                            throw CPDFException(err, "File not found or could not be opened");
                            break;
                        case FPDF_ERR_FORMAT:
                            throw CPDFException(err, "File not in PDF format or corrupted");
                            break;
                        case FPDF_ERR_PASSWORD:
                            throw CPDFException(err, "Password required or incorrect password");
                            break;
                        case FPDF_ERR_SECURITY:
                            throw CPDFException(err, "Unsupported security scheme");
                            break;
                        case FPDF_ERR_PAGE:
                            throw CPDFException(err, "Page not found or content error");
                            break;
                        default:
                            throw CPDFException(err, std::format("Unknown error %ld", err).c_str());
                    }
                }
                pdf.Doc = std::move(UNQ_FPDF_DOCUMENT(p, [](FPDF_DOCUMENT p) { FPDF_CloseDocument(p); }));
                //PAGES, TEXTPAGES
                int count = FPDF_GetPageCount(pdf.Doc.get());
                for (auto i = 0; i < count; i++) {
                    auto pPage = FPDF_LoadPage(pdf.Doc.get(), i);
                    pdf.Pages.emplace_back(pPage, [](FPDF_PAGE p) { FPDF_ClosePage(p); });
                    pdf.TextPages.emplace_back(FPDFText_LoadPage(pPage), [](FPDF_TEXTPAGE p) { FPDFText_ClosePage(p); });
                }

            } catch (...) {
                ex_ptr = std::current_exception();
                pdf.Doc = nullptr;
            }

        }, file_path, password);
        
        if (ex_ptr != nullptr) {
            std::rethrow_exception(ex_ptr);
        }
    }

    void CreateDocument()
    {
        all_run([](PDFObject& pdf, void*) {
            pdf.Doc = std::move(UNQ_FPDF_DOCUMENT(FPDF_CreateNewDocument(), [](FPDF_DOCUMENT p) { FPDF_CloseDocument(p); })); }, nullptr);
    }

    int GetPageCount()
    {
        return single_run([&](PDFObject& pdf, void*) {
            return FPDF_GetPageCount(pdf.Doc.get());
        }, nullptr);
    }

    FPDF_BOOL GetFileVersion(FPDF_DOCUMENT doc,
                                   int* fileVersion)
    {
        return single_run([&](PDFObject& pdf, int* fileVersion) {
            return FPDF_GetFileVersion(pdf.Doc.get(), fileVersion);
        }, fileVersion);
    }

    FPDF_BOOL ImportPagesByIndex(FPDF_DOCUMENT src_doc,
                            const int* page_indices,
                            unsigned long length,
                            int index)
    {
        std::vector<FPDF_BOOL> results = all_run([&](
            PDFObject& pdf,
            FPDF_DOCUMENT src_doc,
            const int* page_indices,
            unsigned long length,
            int index) {
            return FPDF_ImportPagesByIndex(pdf.Doc.get(), src_doc, page_indices, length, index);
        },src_doc, page_indices, length, index);

        return FPDF_BOOL(std::all_of(results.cbegin(), results.cend(), [](const FPDF_BOOL& result)->bool { return result; }));
    }

    FPDF_BOOL ImportPages(FPDF_DOCUMENT src_doc,
                        FPDF_BYTESTRING pagerange,
                        int index)
    {
        std::vector<FPDF_BOOL> results = all_run([&](PDFObject& pdf,
            FPDF_DOCUMENT src_doc,
            FPDF_BYTESTRING pagerange,
            int index) {
            return FPDF_ImportPages(pdf.Doc.get(), src_doc, pagerange, index);
        }, src_doc, pagerange, index);
        return FPDF_BOOL(std::all_of(results.cbegin(), results.cend(), [](const FPDF_BOOL& result)->bool{ return result; }));
    }

    FPDF_BOOL SaveAsCopy(FPDF_FILEWRITE* pFileWrite,
                        FPDF_DWORD flags)
    {
        return single_run([&](PDFObject& pdf,
            FPDF_FILEWRITE* pFileWrite,
            FPDF_DWORD flags) {
            return FPDF_SaveAsCopy(pdf.Doc.get(), pFileWrite, flags);
        }, pFileWrite, flags);
    }

    FPDF_BOOL SaveWithVersion(FPDF_FILEWRITE* pFileWrite,
                     FPDF_DWORD flags,
                     int fileVersion)
    {
        return single_run([&](PDFObject& pdf,
            FPDF_FILEWRITE* pFileWrite,
            FPDF_DWORD flags,
            int fileVersion) {
            return FPDF_SaveWithVersion(pdf.Doc.get(), pFileWrite, flags, fileVersion);
        }, pFileWrite, flags, fileVersion);
    }
    /*************/
    /* FPDF_PAGE */
    /*************/
//private:
//    FPDF_PAGE LoadPage(FPDF_DOCUMENT document, int page_index)
//    {
//        return ThreadRun(FPDF_LoadPage, document, page_index);
//    }
//    void ClosePage(FPDF_PAGE page)
//    {
//        return ThreadRun(FPDF_ClosePage, page);
//    }
//public:
//    UNQ_FPDF_PAGE UnqLoadPage(FPDF_DOCUMENT document, int page_index)
//    {
//        return UNQ_FPDF_PAGE(
//            LoadPage(document, page_index),
//            [this](FPDF_PAGE p) { if (p) { return ClosePage(p); } });
//    }
//
    double GetPageWidth(int page_index)
    {
        return single_run([&](PDFObject& pdf, int page_index) {
            return FPDF_GetPageWidth(pdf.Pages[page_index].get());
        },page_index);
    }
    double GetPageHeight(int page_index)
    {
        return single_run([&](PDFObject& pdf, int page_index) {
            return FPDF_GetPageHeight(pdf.Pages[page_index].get());
        },page_index);
    }

    int Page_GetRotation(int page_index)
    {
        return single_run([&](PDFObject& pdf, int page_index) {
            return FPDFPage_GetRotation(pdf.Pages[page_index].get());
        }, page_index);
    }

    void Page_SetRotation(int page_index, int rotate)
    {
        return all_run([&](PDFObject& pdf, int page_index, int rotate) {
            return FPDFPage_SetRotation(pdf.Pages[page_index].get(), rotate);
        }, page_index, rotate);
    }

    //FPDF_BITMAP Page_GetThumbnailAsBitmap(int page_index)
    //{
    //    return single_run([&](PDFObject& pdf, int page_index) {
    //        return FPDFPage_GetThumbnailAsBitmap(pdf.Pages[page_index].get());
    //    }, page_index);
    //}

    UNQ_FPDF_BITMAP Page_GetThumbnailAsUnqBitmap(int page_index)
    {
        std::thread::id id = std::this_thread::get_id();
        return single_run([&](PDFObject& pdf, int page_index) {
            return UNQ_FPDF_BITMAP(FPDFPage_GetThumbnailAsBitmap(pdf.Pages[page_index].get()),
                [this, id](FPDF_BITMAP p) { specific_run(id,
                [](PDFObject& pdf, FPDF_BITMAP p) {
                return FPDFBitmap_Destroy(p);
            }, p); });
            }, page_index);
    }

    /***************/
    /* FPDF_BITMAP */
    /***************/
//private:
//    FPDF_BITMAP Bitmap_CreateEx(int width,
//                                int height,
//                                int format,
//                                void* first_scan,
//                                int stride)
//    {
//        return ThreadRun(FPDFBitmap_CreateEx, width, height, format, first_scan, stride);
//    }
//    void Bitmap_Destroy(FPDF_BITMAP bitmap)
//    {
//        return ThreadRun(FPDFBitmap_Destroy, bitmap);
//    }
public:
    UNQ_FPDF_BITMAP Bitmap_UnqCreateEx(int width,
                                        int height,
                                        int format,
                                        void* first_scan,
                                        int stride)
    {
        std::thread::id id = std::this_thread::get_id();
        return UNQ_FPDF_BITMAP(
            FPDFBitmap_CreateEx(width, height, format, first_scan, stride),
            [this, id](FPDF_BITMAP p) { specific_run(id,
            [](PDFObject& pdf, FPDF_BITMAP p) {
            if (p) { return FPDFBitmap_Destroy(p); } }, p);
        });
    }

    //void Bitmap_FillRect(FPDF_BITMAP bitmap,
    //                    int left,
    //                    int top,
    //                    int width,
    //                    int height,
    //   
    //             FPDF_DWORD color)
    //{
    //    return ThreadRun(FPDFBitmap_FillRect, bitmap, left, top, width, height, color);
    //}
    //void RenderPage(HDC dc,
    //                FPDF_PAGE page,
    //                int start_x,
    //                int start_y,
    //                int size_x,
    //                int size_y,
    //                int rotate,
    //                int flags)
    //{
    //    return ThreadRun(FPDF_RenderPage, dc, page, start_x, start_y, size_x, size_y, rotate, flags);
    //}
    //void RenderPageBitmap(FPDF_BITMAP bitmap,
    //                            FPDF_PAGE page,
    //                            int start_x,
    //                            int start_y,
    //                            int size_x,
    //                            int size_y,
    //                            int rotate,
    //                            int flags)
    //{
    //    return ThreadRun(FPDF_RenderPageBitmap, bitmap, page, start_x, start_y, size_x, size_y, rotate, flags);
    //}

    //void RenderPageBitmapWithMatrix(FPDF_BITMAP bitmap,
    //                            FPDF_PAGE page,
    //                            const FS_MATRIX* matrix,
    //                            const FS_RECTF* clipping,
    //                            int flags)
    //{
    //    return ThreadRun(FPDF_RenderPageBitmapWithMatrix, bitmap, page, matrix, clipping, flags);
    //}

    /*****************/
    /* FPDF_TEXTPAGE */
    /*****************/
//private:
//    FPDF_TEXTPAGE Text_LoadPage(FPDF_PAGE pPage)
//    {
//        return ThreadRun(FPDFText_LoadPage, pPage);
//    }
//
//    void Text_ClosePage(FPDF_TEXTPAGE pPage)
//    {
//        return ThreadRun(FPDFText_ClosePage, pPage);
//    }
//public:
//    UNQ_FPDF_TEXTPAGE Text_UnqLoadPage(FPDF_PAGE pPage)
//    {
//        return UNQ_FPDF_TEXTPAGE(
//            Text_LoadPage(pPage),
//            [this](FPDF_TEXTPAGE p) { if (p) { return Text_ClosePage(p); } });
//    }
    int Text_CountChars(int index)
    {
        return single_run([](PDFObject& pdf, int index) {return FPDFText_CountChars(pdf.TextPages[index].get()); }, index);
    }
    int Text_CountRects(int index,
                        int start_index,
                        int count)
    {
        return single_run([](PDFObject& pdf, int index, int start_index, int count) {
            return FPDFText_CountRects(pdf.TextPages[index].get(), start_index, count); }, index, start_index, count);
    }
    int Text_GetText(int index, int start_index, int count, unsigned short* result)
    {
        return single_run([](PDFObject& pdf, int index, int start_index, int count, unsigned short* result) {
            return FPDFText_GetText(pdf.TextPages[index].get(), start_index, count, result); }, index, start_index, count, result);
    }

    int Text_GetCharIndexAtPos(int index, double x, double y, double xTolerance, double yTolerance)
    {
        return single_run([](PDFObject& pdf, int index, double x, double y, double xTolerance, double yTolerance) {
            return FPDFText_GetCharIndexAtPos(pdf.TextPages[index].get(), x, y, xTolerance, yTolerance); }, index, x, y, xTolerance, yTolerance);
    }

    FPDF_BOOL Text_GetRect(int index,
                            int rect_index,
                            double* left,
                            double* top,
                            double* right,
                            double* bottom)
    {
        return single_run([](PDFObject& pdf, int index, int rect_index, double* left, double* top, double* right, double* bottom) {
            return FPDFText_GetRect(pdf.TextPages[index].get(), rect_index, left, top, right, bottom); }, index, rect_index, left, top, right, bottom);
    }

    /******************/
    /* FPDF_SCHHANDLE */
    /******************/
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





