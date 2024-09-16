#pragma once
#include "FPDF.h"
#include "D2DWTypes.h"
#include "MyUniqueHandle.h"

class CFPDFTextPage;
class CFPDFBitmap;
class CFPDFFormHandle;

class CFPDFPage
{
    friend class CFPDFBitmap;
    friend class CFPDFFormHandle;
private:
    struct delete_fpdf_page
    {
	    void operator()(FPDF_PAGE& p)
        { 
            if(p){
                FPDF_LOCK;
                FPDF_ClosePage(p);
                p = nullptr;
            }
        }
    };
    std::unique_ptr<std::remove_pointer_t<FPDF_PAGE>, delete_fpdf_page> m_p;

public:
    CFPDFPage(FPDF_PAGE p = nullptr)
        :m_p(p){}

    operator bool() const { return m_p.get(); }

    float GetPageWidthF() const
    {
        FPDF_LOCK;
        return FPDF_GetPageWidthF(m_p.get());
    }
    double GetPageWidth() const
    {
        FPDF_LOCK;
        return FPDF_GetPageWidth(m_p.get());
    }
    float GetPageHeightF() const
    {
        FPDF_LOCK;
        return FPDF_GetPageHeightF(m_p.get());
    }
    double GetPageHeight() const
    {
        FPDF_LOCK;
        return FPDF_GetPageHeight(m_p.get());
    }
    int GetRotation() const
    {
        FPDF_LOCK;
        return FPDFPage_GetRotation(m_p.get());
    }
    void SetRotation(int rotate)
    {
        FPDF_LOCK;
        return FPDFPage_SetRotation(m_p.get(), rotate);
    }

    FPDF_BITMAP GetThumbnailAsBitmap();

    CFPDFTextPage LoadTextPage() const;

	void OnAfterLoadPage(const CFPDFFormHandle& form);
    void OnBeforeClosePage(const CFPDFFormHandle& form);
    void DoPageAAction(const CFPDFFormHandle& form, int aaType);
};
