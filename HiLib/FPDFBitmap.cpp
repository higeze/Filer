#include "FPDFBitmap.h"
#include "FPDFPage.h"

FPDF_BOOL Pause(struct _IFSDK_PAUSE* pThis)
{
    auto cancel_ptr = reinterpret_cast<std::function<bool()>*>(pThis->user);
    return cancel_ptr->operator()();
}



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

    //IFSDK_PAUSE pause;
    //pause.version = 1;
    //pause.user = reinterpret_cast<void*>(&cancel);
    //pause.NeedToPauseNow = Pause;

    //int ret = FPDF_RenderPageBitmap_Start(m_p.get(), page.m_p.get(), start_x, start_y, size_x, size_y, rotate, flags, &pause);
    //if (ret == FPDF_RENDER_FAILED) {
    //    return false;
    //}
    //else if (ret == FPDF_RENDER_TOBECONTINUED) {
    //    ret = FPDF_RenderPage_Continue(page.m_p.get(), &pause);
    //}

    //if (ret == FPDF_RENDER_DONE) {
    //    FPDF_RenderPage_Close(page.m_p.get());
    //}
    //else {
    //    return false;
    //}
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