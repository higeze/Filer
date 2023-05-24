#pragma once
#include <fpdfview.h>
#include <fpdf_edit.h>
#include <fpdf_save.h>
#include <fpdf_ppo.h>
#include "UniqueFpdf.h"
#include "D2DWTypes.h"
#include "MyUniqueHandle.h"

class CUniqueFPdfPage;

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

class CUniqueFPdfDocument :public  std::unique_ptr<std::remove_pointer_t<FPDF_DOCUMENT>, delete_fpdf_document>
{
public:
    CUniqueFPdfDocument(FPDF_DOCUMENT p = nullptr)
        :std::unique_ptr<std::remove_pointer_t<FPDF_DOCUMENT>, delete_fpdf_document>(p){}

    void LoadDocument(FPDF_STRING file_path, FPDF_BYTESTRING password)
    {
        FPDF_LOCK;
        return reset(FPDF_LoadDocument(file_path, password));
    }

    void CreateNewDocument()
    {
        FPDF_LOCK;
        return reset(FPDF_CreateNewDocument());
    }

    int GetPageCount() const
    {
        FPDF_LOCK;
        return FPDF_GetPageCount(get());
    }

    FPDF_BOOL GetFileVersion(int* fileVersion)
    {
        FPDF_LOCK;
        return FPDF_GetFileVersion(get(), fileVersion);
    }

    CUniqueFPdfPage LoadPage(int page_index);

    FPDF_BOOL ImportPagesByIndex(
        const CUniqueFPdfDocument& src_doc,
        const int* page_indices,
        unsigned long length,
        int index)
    {
        FPDF_LOCK;
        return FPDF_ImportPagesByIndex(get(), src_doc.get(), page_indices, length, index);
    }

    FPDF_BOOL ImportPages(
        const CUniqueFPdfDocument& src_doc,
        FPDF_BYTESTRING pagerange,
        int index)
    {
        FPDF_LOCK;
        return FPDF_ImportPages(get(), src_doc.get(), pagerange, index);
    }

    FPDF_BOOL SaveAsCopy(
        FPDF_FILEWRITE* pFileWrite,
        FPDF_DWORD flags)
    {
        FPDF_LOCK;
        return FPDF_SaveAsCopy(get(), pFileWrite, flags);
    }

    FPDF_BOOL SaveWithVersion(
        FPDF_FILEWRITE* pFileWrite,
        FPDF_DWORD flags,
        int fileVersion)
    {
        FPDF_LOCK;
        return FPDF_SaveWithVersion(get(), pFileWrite, flags, fileVersion);
    }



};