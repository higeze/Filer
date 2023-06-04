#pragma once
#include "FPDF.h"
#include "D2DWTypes.h"
#include "MyUniqueHandle.h"

class CFPDFSchHandle
{
private:
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
	std::unique_ptr<std::remove_pointer_t<FPDF_SCHHANDLE>, delete_fpdf_schhandle> m_p;
public:
	CFPDFSchHandle(FPDF_SCHHANDLE p = nullptr)
		:m_p(p) {}
	  
	operator bool() const { return m_p.get(); }

	FPDF_BOOL FindNext() const
	{
		FPDF_LOCK;
		return FPDFText_FindNext(m_p.get());
	}

	int GetSchResultIndex() const
	{
		FPDF_LOCK;
		return FPDFText_GetSchResultIndex(m_p.get());
	}

	int GetSchCount() const
	{
		FPDF_LOCK;
		return FPDFText_GetSchCount(m_p.get());
	}
};
