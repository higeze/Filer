#pragma once
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <fpdf_text.h>
#pragma comment(lib, "pdfium.dll.lib")

#include "ThreadPool.h"

class CThreadPool;

using UNQ_FPDF_DOCUMENT = std::unique_ptr<std::remove_pointer_t<FPDF_DOCUMENT>, std::function<void(FPDF_DOCUMENT)>>;
using UNQ_FPDF_PAGE = std::unique_ptr<std::remove_pointer_t<FPDF_PAGE>, std::function<void(FPDF_PAGE)>>;
using UNQ_FPDF_BITMAP = std::unique_ptr<std::remove_pointer_t<FPDF_BITMAP>, std::function<void(FPDF_BITMAP)>>;
using UNQ_FPDF_TEXTPAGE = std::unique_ptr<std::remove_pointer_t<FPDF_TEXTPAGE>, std::function<void(FPDF_TEXTPAGE)>>;
using UNQ_FPDF_SCHHANDLE = std::unique_ptr<std::remove_pointer_t<FPDF_SCHHANDLE>, std::function<void(FPDF_SCHHANDLE)>>;

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

    /*****************/
    /* FPDF_DOCUMENT */
    /*****************/
private:
    FPDF_DOCUMENT LoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password)
    {
        return ThreadRun(FPDF_LoadDocument, file_path, password);
    }
    void CloseDocument(FPDF_DOCUMENT document)
    { 
        return ThreadRun(FPDF_CloseDocument, document); 
    }
public:
    UNQ_FPDF_DOCUMENT UnqLoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password)
    {
        return UNQ_FPDF_DOCUMENT(
            LoadDocument(file_path, password),
            [this](FPDF_DOCUMENT p) { if (p) { return CloseDocument(p); } });
    }

    int GetPageCount(FPDF_DOCUMENT document)
    {
        return ThreadRun(FPDF_GetPageCount, document);
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
