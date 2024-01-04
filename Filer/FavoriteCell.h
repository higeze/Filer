#pragma once
#include "FileIconCell.h"

class CFavoriteCell:public CFileIconCell
{
	using base = CFileIconCell;
private:
	virtual std::wstring GetShortName();
public:
	using CFileIconCell::CFileIconCell;
	virtual ~CFavoriteCell(){}

	virtual std::wstring GetString() override;
	virtual std::shared_ptr<CShellFile> GetShellFile() override;
	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override;
	virtual void OnContextMenu(const ContextMenuEvent& e) override;
};