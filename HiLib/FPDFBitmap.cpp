#include "FPDFBitmap.h"
#include "FPDFPage.h"


bool CFPDFBitmap::RenderPageBitmap(
    const CFPDFPage& page,
    int start_x,
    int start_y,
    int size_x,
    int size_y,
    int rotate,
    int flags,
    std::function<bool()> cancel)
{
    FPDF_LOCK;
    FPDF_RETURN_FALSE_IF_CANCEL;
    FPDF_RenderPageBitmap(m_p.get(), page.m_p.get(), start_x, start_y, size_x, size_y, rotate, flags);
    return true;
}

bool CFPDFBitmap::RenderPageBitmapWithMatrix(
    const CFPDFPage& page,
    const FS_MATRIX* matrix,
    const FS_RECTF* clipping,
    int flags,
    std::function<bool()> cancel)
{
    FPDF_LOCK;
    FPDF_RETURN_FALSE_IF_CANCEL;
    FPDF_RenderPageBitmapWithMatrix(m_p.get(), page.m_p.get(), matrix, clipping, flags);
    return true;
}