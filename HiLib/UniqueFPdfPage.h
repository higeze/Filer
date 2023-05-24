#pragma once
#include <fpdfview.h>
#include <fpdf_edit.h>
#include "UniqueFpdf.h"
#include "D2DWTypes.h"
#include "MyUniqueHandle.h"

class CUniqueFPdfTextPage;

struct delete_fpdf_page
{
	void operator()(FPDF_PAGE p)
    { 
        if(p){
            FPDF_LOCK;
            FPDF_ClosePage(p);
        }
    }
};

class CUniqueFPdfPage:public std::unique_ptr<std::remove_pointer_t<FPDF_PAGE>, delete_fpdf_page>
{
public:
    CUniqueFPdfPage(FPDF_PAGE p = nullptr)
        :std::unique_ptr<std::remove_pointer_t<FPDF_PAGE>, delete_fpdf_page>(p){}
    float GetPageWidthF() const
    {
        FPDF_LOCK;
        return FPDF_GetPageWidthF(get());
    }
    double GetPageWidth() const
    {
        FPDF_LOCK;
        return FPDF_GetPageWidth(get());
    }
    float GetPageHeightF() const
    {
        FPDF_LOCK;
        return FPDF_GetPageHeightF(get());
    }
    double GetPageHeight() const
    {
        FPDF_LOCK;
        return FPDF_GetPageHeight(get());
    }
    int GetRotation() const
    {
        FPDF_LOCK;
        return FPDFPage_GetRotation(get());
    }
    void SetRotation(int rotate)
    {
        FPDF_LOCK;
        return FPDFPage_SetRotation(get(), rotate);
    }

    UHBITMAP GetBitmap(HDC hDC, const float& scale);
    UHBITMAP GetClippedBitmap(HDC hDC, const float& scale, const CRectF& rectInPage);

    CUniqueFPdfTextPage LoadTextPage() const;

};
