#pragma once
#include "FileIconCell.h"

class CShellFile;

class CFavoriteCell:public CFileIconCell
{
private:
	virtual std::wstring GetShortName();
	virtual UINT GetFormat()const { return DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL; }

public:
	CFavoriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty);
	virtual ~CFavoriteCell(){}

	virtual void PaintContent(CDC* pDC, CRect rcPaint)override;
	virtual std::shared_ptr<CShellFile> GetShellFile()const override;
};