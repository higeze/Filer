#pragma once
#include "FileIconCell.h"

class CLauncherCell:public CFileIconCell
{
	using base = CFileIconCell;
private:
	virtual std::wstring GetShortName();
public:
	using CFileIconCell::CFileIconCell;
	virtual ~CLauncherCell() = default;

	virtual std::shared_ptr<CShellFile> GetShellFile() override;
	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override;
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e) override;
	virtual void OnContextMenu(const ContextMenuEvent& e) override;
};