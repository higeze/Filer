#pragma once
#include <fpdfview.h>
#include <fpdf_text.h>
#include "UniqueFpdf.h"
#include "D2DWTypes.h"
#include "MyUniqueHandle.h"

class CUniqueFPdfSchHandle;

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

class CUniqueFPdfTextPage :public std::unique_ptr<std::remove_pointer_t<FPDF_TEXTPAGE>, delete_fpdf_textpage>
{
public:
	CUniqueFPdfTextPage(FPDF_TEXTPAGE p = nullptr)
		:std::unique_ptr<std::remove_pointer_t<FPDF_TEXTPAGE>, delete_fpdf_textpage>(p){}

	int CountChars() const
	{
		FPDF_LOCK;
		return FPDFText_CountChars(get());
	}

	int CountRects(
		int start_index,
		int count) const
	{
		FPDF_LOCK;
		return FPDFText_CountRects(get(), start_index, count);
	}

	int GetText(
		int start_index,
		int count,
		unsigned short* result) const
	{
		FPDF_LOCK;
		return FPDFText_GetText(get(), start_index, count, result);
	}

	int GetCharIndexAtPos(
		double x,
		double y,
		double xTolerance,
		double yTolerance) const
	{
		FPDF_LOCK;
		return FPDFText_GetCharIndexAtPos(get(), x, y, xTolerance, yTolerance);
	}

	FPDF_BOOL GetRect(
		int rect_index,
		double* left,
		double* top,
		double* right,
		double* bottom)
	{
		FPDF_LOCK;
		return FPDFText_GetRect(get(), rect_index, left, top, right, bottom);
	}

	std::vector<CRectF> GetRects();
	std::vector<CRectF> GetRangeRects(int begin, int end);

	CUniqueFPdfSchHandle FindStart(
		FPDF_WIDESTRING findwhat,
		unsigned long flags,
		int start_index);

	std::vector<std::tuple<int, int, std::vector<CRectF>>> SearchResults(FPDF_WIDESTRING findwhat);




};