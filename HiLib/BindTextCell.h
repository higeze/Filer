#pragma once
#include "TextCell.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindTextColumn.h"

template<typename... TItems>
class CBindTextCell :public CTextCell
{
public:
	template<typename... Args>
	CBindTextCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:CTextCell(pSheet, pRow, pColumn, spProperty, args...){}

	virtual ~CBindTextCell() = default;

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		auto pBindColumn = static_cast<CBindTextColumn<TItems...>*>(m_pColumn);
		return pBindColumn->GetGetter()(pBindRow->GetTupleItems());
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<CBindRow<TItems...>*>(m_pRow);
		auto pBindColumn = static_cast<CBindTextColumn<TItems...>*>(m_pColumn);
		pBindColumn->GetSetter()(pBindRow->GetTupleItems(), str);
	}
};