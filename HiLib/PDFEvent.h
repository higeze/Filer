#pragma once
#include <tuple>
#include <string>
#include "Direct2DWrite.h"

class CD2DWWindow;
class CDirect2DWrite;
class CPDFViewport;

//For Event in boost::sml, do not use virual, it cause C4789 error.
//If no virtual couldn't resolve C4789, do not inherit anything.

/************/
/* CopyText */
/************/
struct CopyDocTextEvent
{
	CopyDocTextEvent(CD2DWWindow* pWindow, const std::tuple<int, int>& begin, const std::tuple<int, int>& end)
	:WndPtr(pWindow), Begin(begin), End(end) {}

	CD2DWWindow* WndPtr;
	std::tuple<int, int> Begin;
	std::tuple<int, int> End;
};

/**********/
/* Render */
/**********/
struct PDFRenderEventBase
{
	PDFRenderEventBase(CDirect2DWrite* pDirect, CPDFViewport* pViewport)
	:DirectPtr(pDirect), ViewportPtr(pViewport) {}

	CDirect2DWrite* DirectPtr;
	CPDFViewport* ViewportPtr;
};

/*****************/
/* RenderContent */
/*****************/
struct RenderContentEventBase : public PDFRenderEventBase
{
	RenderContentEventBase(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const FLOAT& scale)
	:PDFRenderEventBase(pDirect, pViewport), Scale(scale) {}

	FLOAT Scale;
};

struct RenderDocContentEvent : public RenderContentEventBase
{
	RenderDocContentEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const FLOAT& scale, int begin, int end)
	:RenderContentEventBase(pDirect, pViewport, scale), PageIndexBegin(begin), PageIndexEnd(end){}

	int PageIndexBegin;
	int PageIndexEnd;
};

struct RenderPageContentEvent : public RenderContentEventBase
{
	RenderPageContentEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const FLOAT& scale, const int& index)
	:RenderContentEventBase(pDirect, pViewport, scale),PageIndex(index) {}

	int PageIndex;
};

/**************/
/* RenderFind */
/**************/
struct RenderFindEventBase : public PDFRenderEventBase
{
	RenderFindEventBase(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const std::wstring& find)
	:PDFRenderEventBase(pDirect, pViewport), Find(find) {}

	std::wstring Find;
};

struct RenderDocFindEvent
{
	RenderDocFindEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const std::wstring& find, int begin, int end)
	:DirectPtr(pDirect), ViewportPtr(pViewport), Find(find), PageIndexBegin(begin), PageIndexEnd(end){}

	CDirect2DWrite* DirectPtr;
	CPDFViewport* ViewportPtr;
	std::wstring Find;
	int PageIndexBegin;
	int PageIndexEnd;
};

struct RenderPageFindEvent
{
	RenderPageFindEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const std::wstring& find, const int& index)
	:DirectPtr(pDirect), ViewportPtr(pViewport),  Find(find), PageIndex(index){}

	CDirect2DWrite* DirectPtr;
	CPDFViewport* ViewportPtr;
	std::wstring Find;
	int PageIndex;
};

struct RenderDocFindLineEvent
{
	RenderDocFindLineEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const std::wstring& find, const CRectF& rc)
	:DirectPtr(pDirect), ViewportPtr(pViewport), Find(find), Rect(rc){}

	CDirect2DWrite* DirectPtr;
	CPDFViewport* ViewportPtr;
	std::wstring Find;
	CRectF Rect;
};

struct RenderPageFindLineEvent
{
	RenderPageFindLineEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const std::wstring& find, const CRectF& rc)
	:DirectPtr(pDirect), ViewportPtr(pViewport), Find(find), Rect(rc) {}

	CDirect2DWrite* DirectPtr;
	CPDFViewport* ViewportPtr;
	std::wstring Find;
	CRectF Rect;
};

/**********************/
/* RenderSelectedText */
/**********************/


struct RenderDocSelectedTextEvent : public PDFRenderEventBase
{
	RenderDocSelectedTextEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, const std::tuple<int, int>& selBegin, const std::tuple<int, int>& selEnd)
	:PDFRenderEventBase(pDirect, pViewport), SelectedBegin(selBegin), SelectedEnd(selEnd)  {}

	std::tuple<int, int> SelectedBegin;
	std::tuple<int, int> SelectedEnd;
};

struct RenderPageSelectedTextEvent: public PDFRenderEventBase
{
	RenderPageSelectedTextEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, int index, int selBegin, int selEnd)
	:PDFRenderEventBase(pDirect, pViewport), PageIndex(index), SelectedBegin(selBegin), SelectedEnd(selEnd) {}

	int PageIndex;
	int SelectedBegin;
	int SelectedEnd;
};

/***************/
/* RenderCaret */
/***************/


struct RenderDocCaretEvent : public PDFRenderEventBase
{
	RenderDocCaretEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, int page_index, int char_index)
	:PDFRenderEventBase(pDirect, pViewport),PageIndex(page_index), CharIndex(char_index)   {}

	int PageIndex;
	int CharIndex;
};

struct RenderPageCaretEvent: public PDFRenderEventBase
{
	RenderPageCaretEvent(CDirect2DWrite* pDirect, CPDFViewport* pViewport, int page_index, int char_index)
	:PDFRenderEventBase(pDirect, pViewport),PageIndex(page_index), CharIndex(char_index)   {}

	int PageIndex;
	int CharIndex;
};
