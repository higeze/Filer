#pragma once

#include "TextCell.h"
#include "FileSizeColumn.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "BindRow.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "GridView.h"
#include <fmt/format.h>
#include <sigslot/signal.hpp>

class CFileSizeCell:public CTextCell
{
private:
	mutable sigslot::connection m_conDelayUpdateAction;

public:


	CFileSizeCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty)
	{
	}

	virtual ~CFileSizeCell()
	{
		m_conDelayUpdateAction.disconnect();
	}


	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override;
	virtual std::wstring GetString() override;
	virtual std::wstring GetSortString() override;

	
private:
	virtual std::shared_ptr<CShellFile> GetShellFile();
};

