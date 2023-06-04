#pragma once
#include "FPDF.h"
#include "D2DWTypes.h"
#include "MyUniqueHandle.h"

class CFPDFSchHandle;

class CFPDFTextPage
{
private:
	struct delete_fpdf_textpage
	{
		void operator()(FPDF_TEXTPAGE p)
		{ 
			if(p){
				FPDF_LOCK;
				FPDFText_ClosePage(p);
			}
		}
	};
	std::unique_ptr<std::remove_pointer_t<FPDF_TEXTPAGE>, delete_fpdf_textpage> m_p;
public:
	CFPDFTextPage(FPDF_TEXTPAGE p = nullptr)
		:m_p(p){}

	operator bool() const { return m_p.get(); }

	int CountChars() const
	{
		FPDF_LOCK;
		return FPDFText_CountChars(m_p.get());
	}

	int CountRects(
		int start_index,
		int count) const
	{
		FPDF_LOCK;
		return FPDFText_CountRects(m_p.get(), start_index, count);
	}

	int GetText(
		int start_index,
		int count,
		unsigned short* result) const
	{
		FPDF_LOCK;
		return FPDFText_GetText(m_p.get(), start_index, count, result);
	}

	int GetCharIndexAtPos(
		double x,
		double y,
		double xTolerance,
		double yTolerance) const
	{
		FPDF_LOCK;
		return FPDFText_GetCharIndexAtPos(m_p.get(), x, y, xTolerance, yTolerance);
	}

	FPDF_BOOL GetRect(
		int rect_index,
		double* left,
		double* top,
		double* right,
		double* bottom)
	{
		FPDF_LOCK;
		return FPDFText_GetRect(m_p.get(), rect_index, left, top, right, bottom);
	}

	std::vector<CRectF> GetRects();
	std::vector<CRectF> GetRangeRects(int begin, int end);

	CFPDFSchHandle FindStart(
		FPDF_WIDESTRING findwhat,
		unsigned long flags,
		int start_index);

	std::vector<std::tuple<int, int, std::vector<CRectF>>> SearchResults(FPDF_WIDESTRING findwhat);




};