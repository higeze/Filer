#pragma once

#include "TextCell.h"

class CFileExtCell:public CTextCell
{
public:
	CFileExtCell::CFileExtCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty, arg<"editmode"_s>() = EditMode::FocusedSingleClickEdit)
	{
	}
	virtual ~CFileExtCell(void){}

	virtual std::wstring GetString() override;
	void SetStringCore(const std::wstring& str) override;
	virtual bool CanSetStringOnEditing()const override { return false; }

};

