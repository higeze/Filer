#pragma once

#include "TextCell.h"

class CFileSizeCell:public CTextCell
{
public:
	CFileSizeCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty);
	virtual ~CFileSizeCell(void){}

	bool operator<(const CCell& rhs)const;
	bool operator>(const CCell& rhs)const;

	virtual bool IsComparable()const override{return false;}
	virtual string_type GetSortString()const override;
	virtual UINT GetFormat()const override{ return DT_RIGHT|DT_TOP|DT_NOPREFIX|DT_WORDBREAK|DT_EDITCONTROL; }
	virtual string_type GetString()const override;
};

