#pragma once
#include "FPDF.h"
#include "D2DWTypes.h"
#include "MyUniqueHandle.h"

class CFPDFPage;
class CFPDFFormHandle;

class CFPDFDocument
{
    friend class CFPDFFormHandle;
private:
    struct delete_fpdf_document
    {
	    void operator()(FPDF_DOCUMENT p)
        { 
            if(p){
                FPDF_LOCK;
                FPDF_CloseDocument(p);
            }
        }
    };
    std::unique_ptr<std::remove_pointer_t<FPDF_DOCUMENT>, delete_fpdf_document> m_p;
public:
    CFPDFDocument(FPDF_DOCUMENT p = nullptr)
        :m_p(p){}

    operator bool() const { return m_p.get(); }

    void LoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password)
    {
        FPDF_LOCK;
        return m_p.reset(FPDF_LoadDocument(file_path, password));
    }

    void CreateNewDocument()
    {
        FPDF_LOCK;
        return m_p.reset(FPDF_CreateNewDocument());
    }

    int GetPageCount() const
    {
        FPDF_LOCK;
        return FPDF_GetPageCount(m_p.get());
    }

    FPDF_BOOL GetFileVersion(int* fileVersion)
    {
        FPDF_LOCK;
        return FPDF_GetFileVersion(m_p.get(), fileVersion);
    }

    void Page_Delete(int page_index)
    {
        FPDF_LOCK;
        return FPDFPage_Delete(m_p.get(), page_index);
    }

    CFPDFPage LoadPage(int page_index);

    FPDF_BOOL ImportPagesByIndex(
        const CFPDFDocument& src_doc,
        const int* page_indices,
        unsigned long length,
        int index)
    {
        FPDF_LOCK;
        return FPDF_ImportPagesByIndex(m_p.get(), src_doc.m_p.get(), page_indices, length, index);
    }

    FPDF_BOOL ImportPages(
        const CFPDFDocument& src_doc,
        FPDF_BYTESTRING pagerange,
        int index)
    {
        FPDF_LOCK;
        return FPDF_ImportPages(m_p.get(), src_doc.m_p.get(), pagerange, index);
    }

    FPDF_BOOL SaveAsCopy(
        FPDF_FILEWRITE* pFileWrite,
        FPDF_DWORD flags)
    {
        FPDF_LOCK;
        return FPDF_SaveAsCopy(m_p.get(), pFileWrite, flags);
    }

    FPDF_BOOL SaveWithVersion(
        FPDF_FILEWRITE* pFileWrite,
        FPDF_DWORD flags,
        int fileVersion)
    {
        FPDF_LOCK;
        return FPDF_SaveWithVersion(m_p.get(), pFileWrite, flags, fileVersion);
    }

    CFPDFFormHandle InitFormFillEnvironment(FPDF_FORMFILLINFO* formInfo);

};