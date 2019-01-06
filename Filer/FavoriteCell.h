#pragma once
#include "FileIconCell.h"

class CShellFile;

class CFavoriteCell:public CFileIconCell
{
private:
	virtual std::wstring GetShortName();
	virtual UINT GetFormat()const{ return DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL; }

public:
	CFavoriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CFavoriteCell(){}

	virtual void PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)override;
	virtual std::shared_ptr<CShellFile> GetShellFile()override;
};