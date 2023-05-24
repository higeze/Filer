#pragma once
#include <fpdfview.h>
#include <fpdf_text.h>
#include "UniqueFpdf.h"
#include "D2DWTypes.h"
#include "MyUniqueHandle.h"

struct delete_fpdf_schhandle
{
	void operator()(FPDF_SCHHANDLE p)
	{ 
		if(p){
			FPDF_LOCK;
			FPDFText_FindClose(p);
		}
	}
};

class CUniqueFPdfSchHandle :public std::unique_ptr<std::remove_pointer_t<FPDF_SCHHANDLE>, delete_fpdf_schhandle>
{
public:
	CUniqueFPdfSchHandle(FPDF_SCHHANDLE p = nullptr)
		:std::unique_ptr<std::remove_pointer_t<FPDF_SCHHANDLE>, delete_fpdf_schhandle>(p) {}

	FPDF_BOOL FindNext() const
	{
		FPDF_LOCK;
		return FPDFText_FindNext(get());
	}

	int GetSchResultIndex() const
	{
		FPDF_LOCK;
		return FPDFText_GetSchResultIndex(get());
	}

	int GetSchCount() const
	{
		FPDF_LOCK;
		return FPDFText_GetSchCount(get());
	}
};
