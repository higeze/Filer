#pragma once
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <fpdf_text.h>
#include <fpdf_ppo.h>
#include <fpdf_save.h>
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
