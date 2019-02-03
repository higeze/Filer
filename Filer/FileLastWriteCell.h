#pragma once
#include "TextCell.h"

class CShellFile;

class CFileLastWriteCell:public CTextCell, public std::enable_shared_from_this<CFileLastWriteCell>
{
private:
	mutable boost::signals2::connection m_conDelayUpdateAction;
	mutable boost::signals2::connection m_conLastWriteChanged;

public:
	CFileLastWriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CFileLastWriteCell(void);

	virtual bool IsComparable()const override{return false;}
	virtual std::wstring GetString() override;

private:
	virtual std::shared_ptr<CShellFile> GetShellFile();
};

