#pragma once

#include "TextCell.h"
class CShellFile;

class CFileSizeCell:public CTextCell, public std::enable_shared_from_this<CFileSizeCell>
{
private:
	mutable boost::signals2::connection m_conDelayUpdateAction;

public:
	CFileSizeCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CFileSizeCell(void);

	virtual bool IsComparable()const override{return false;}
	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite* pDirect) override;
	virtual std::wstring GetString() override;
	virtual std::wstring GetSortString() override;
//	virtual UINT GetFormat()const override{ return DT_RIGHT|DT_TOP|DT_NOPREFIX|DT_WORDBREAK|DT_EDITCONTROL; }
	
private:
	virtual std::shared_ptr<CShellFile> GetShellFile() ;
};

