#include "UniqueFPdfBitmap.h"
#include "UniqueFPdfPage.h"


void CUniqueFPdfBitmap::RenderPageBitmap(
        const CUniqueFPdfPage& page,
        int start_x,
        int start_y,
        int size_x,
        int size_y,
        int rotate,
        int flags)
{
    FPDF_LOCK;
    return FPDF_RenderPageBitmap(get(), page.get(), start_x, start_y, size_x, size_y, rotate, flags);
}

void CUniqueFPdfBitmap::RenderPageBitmapWithMatrix(
    const CUniqueFPdfPage& page,
    const FS_MATRIX* matrix,
    const FS_RECTF* clipping,
    int flags)
{
    FPDF_LOCK;
    return FPDF_RenderPageBitmapWithMatrix(get(), page.get(), matrix, clipping, flags);
}