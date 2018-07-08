#pragma once
#include "Cell.h"

class CShellFile;

class CFileIconCell:public CCell, public std::enable_shared_from_this<CFileIconCell>
{
private:
	boost::signals2::connection m_conIconChanged;
public:
	CFileIconCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty);
	virtual ~CFileIconCell();

	virtual bool IsComparable()const override{return false;}
	virtual void PaintContent(CDC* pDC, CRect rcPaint)override;
	virtual CSize MeasureContentSize(CDC* pDC) override;
	virtual CSize MeasureContentSizeWithFixedWidth(CDC* pDC) override;
	virtual std::shared_ptr<CShellFile> GetShellFile();
};