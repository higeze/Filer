#pragma once
#include <mutex>
#include <fpdfview.h>
#include <fpdf_edit.h>
#include "UniqueFpdf.h"
#include "D2DWTypes.h"
#include "MyUniqueHandle.h"

class CUniqueFPdfPage;

struct delete_fpdf_bitmap
{
	void operator()(FPDF_BITMAP p)
    { 
        if(p){
            FPDF_LOCK;
            FPDFBitmap_Destroy(p);
        }
    }
};

class CUniqueFPdfBitmap : public std::unique_ptr<std::remove_pointer_t<FPDF_BITMAP>, delete_fpdf_bitmap>
{
public:
    CUniqueFPdfBitmap(FPDF_BITMAP p = nullptr)
        :std::unique_ptr<std::remove_pointer_t<FPDF_BITMAP>, delete_fpdf_bitmap>(p) {}

    void CreateEx(
        int width,
        int height,
        int format,
        void* first_scan,
        int stride)
    {
        FPDF_LOCK;
        return reset(FPDFBitmap_CreateEx(width, height, format, first_scan, stride));
    }

    void FillRect(
        int left,
        int top,
        int width,
        int height,
        FPDF_DWORD color)
    {
        FPDF_LOCK;
        return FPDFBitmap_FillRect(get(), left, top, width, height, color);
    }
    void RenderPageBitmap(
        const CUniqueFPdfPage& page,
        int start_x,
        int start_y,
        int size_x,
        int size_y,
        int rotate,
        int flags);

    void RenderPageBitmapWithMatrix(
        const CUniqueFPdfPage& page,
        const FS_MATRIX* matrix,
        const FS_RECTF* clipping,
        int flags);

};
