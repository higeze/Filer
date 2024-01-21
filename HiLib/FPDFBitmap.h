#pragma once
#include "FPDF.h"
#include "D2DWTypes.h"
#include "MyUniqueHandle.h"

class CFPDFPage;

class CFPDFBitmap
{
    friend class CFPDFFormHandle;
private:
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
    std::unique_ptr<std::remove_pointer_t<FPDF_BITMAP>, delete_fpdf_bitmap> m_p;

public:
    CFPDFBitmap(FPDF_BITMAP p = nullptr)
        :m_p(p) {}

    operator bool() const { return m_p.get(); }

    bool CreateEx(
        int width,
        int height,
        int format,
        void* first_scan,
        int stride,
        std::function<bool()> cancel = []()->bool { return false; })
    {
        FPDF_LOCK;
        FPDF_RETURN_FALSE_IF_CANCEL;
        m_p.reset(FPDFBitmap_CreateEx(width, height, format, first_scan, stride));
        return true;
    }

    int GetStride() const
    {
        FPDF_LOCK;
        return FPDFBitmap_GetStride(m_p.get());    
    }


    void* GetBuffer() const
    {
        FPDF_LOCK;
        return FPDFBitmap_GetBuffer(m_p.get());
    }

    int GetWidth() const
    {
        FPDF_LOCK;
        return FPDFBitmap_GetWidth(m_p.get());
    }

    int GetHeight() const
    {
        FPDF_LOCK;
        return FPDFBitmap_GetHeight(m_p.get());
    }

    bool FillRect(
        int left,
        int top,
        int width,
        int height,
        FPDF_DWORD color,
        std::function<bool()> cancel = []()->bool { return false; })
    {
        FPDF_LOCK;
        FPDF_RETURN_FALSE_IF_CANCEL;
        FPDFBitmap_FillRect(m_p.get(), left, top, width, height, color);
        return true;
    }
   bool RenderPageBitmap(
        const CFPDFPage& page,
        int start_x,
        int start_y,
        int size_x,
        int size_y,
        int rotate,
        int flags,
        std::function<bool()> cancel = []()->bool { return false; });

    bool RenderPageBitmapWithMatrix(
        const CFPDFPage& page,
        const FS_MATRIX* matrix,
        const FS_RECTF* clipping,
        int flags,
        std::function<bool()> cancel = []()->bool { return false; });

};
