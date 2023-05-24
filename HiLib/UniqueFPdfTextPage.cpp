#include "UniqueFPdfTextPage.h"
#include "UniqueFPdfSchHandle.h"

std::vector<CRectF> CUniqueFPdfTextPage::GetRects()
{
	FPDF_LOCK;
	std::vector<CRectF> rects;
	int charCount = CountChars();
	for (auto i = 0; i < charCount; i++) {
		int rect_count = CountRects(i, 1);
		if (rect_count == 1) {
			double left, top, right, bottom = 0.f;
			GetRect(0, &left, &top, &right, &bottom);
			rects.emplace_back(
				static_cast<FLOAT>(left),
				static_cast<FLOAT>(top),
				static_cast<FLOAT>(right),
				static_cast<FLOAT>(bottom));
		}
	}
	return rects;
}
std::vector<CRectF> CUniqueFPdfTextPage::GetRangeRects(int begin, int end)
{
	FPDF_LOCK;
	int rect_count = CountRects(begin, end - begin);
	std::vector<CRectF> rects;
	for (auto i = 0; i < rect_count; i++) {
		double left, top, right, bottom;
		GetRect(0, &left, &top, &right, &bottom);
		rects.emplace_back(
			static_cast<FLOAT>(left),
			static_cast<FLOAT>(top),
			static_cast<FLOAT>(right),
			static_cast<FLOAT>(bottom));
	}
	return rects;
}

CUniqueFPdfSchHandle CUniqueFPdfTextPage::FindStart(
	FPDF_WIDESTRING findwhat,
    unsigned long flags,
    int start_index)
{
	FPDF_LOCK;
	return CUniqueFPdfSchHandle(FPDFText_FindStart(get(), findwhat, flags, start_index));
}

std::vector<std::tuple<int, int, std::vector<CRectF>>> CUniqueFPdfTextPage::SearchResults(FPDF_WIDESTRING findwhat)
{
	FPDF_LOCK;
	std::vector<std::tuple<int, int, std::vector<CRectF>>> results;

	CUniqueFPdfSchHandle schHdl(FindStart(findwhat, 0, 0));
	while (schHdl.FindNext()) {
		int index = schHdl.GetSchResultIndex();
		int ch_count = schHdl.GetSchCount();
		int rc_count = CountRects(index, ch_count);
		std::vector<CRectF> rects;
		for (int i = 0; i < rc_count; i++) {
			double left, top, right, bottom;
			GetRect(
				i,
				&left,
				&top,
				&right,
				&bottom);
			rects.emplace_back(
				static_cast<FLOAT>(left),
				static_cast<FLOAT>(top),
				static_cast<FLOAT>(right),
				static_cast<FLOAT>(bottom));
		}
		results.emplace_back(index, ch_count, rects);
	}
	return results;
}