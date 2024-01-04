#pragma once
#include "TextCell.h"
#include "CellProperty.h"
#include "ShellFile.h"

class CFileLastWriteCell:public CTextCell
{
private:
	mutable sigslot::connection m_conDelayUpdateAction;
	mutable sigslot::connection m_conLastWriteChanged;

public:
	CFileLastWriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty)
	{
	}

	virtual ~CFileLastWriteCell()
	{
		m_conDelayUpdateAction.disconnect();
		m_conLastWriteChanged.disconnect();
	}

	virtual std::wstring GetString() override;

private:
	virtual std::shared_ptr<CShellFile> GetShellFile();
};

