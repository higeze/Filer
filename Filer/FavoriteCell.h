#pragma once
#include "FileIconCell.h"

class CShellFile;

class CFavoriteCell:public CFileIconCell
{
private:
	virtual std::wstring GetShortName();
public:
	CFavoriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CFavoriteCell(){}

	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)override;
	virtual std::shared_ptr<CShellFile> GetShellFile()override;
	virtual void OnContextMenu(const ContextMenuEvent& e) override;
};