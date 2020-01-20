#pragma once
#include "Cell.h"
#include "BindGridView.h"
#include "BindTextColumn.h"

template<typename T>
class CBindTextCell :public CTextCell
{
private:
public:
	CBindCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, CMenu* pMenu = nullptr)
		:CTextCell(pSheet, pRow, pColumn, spProperty, pMenu){}
	virtual ~CBindCell() = default:

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<T>*>(m_pRow);
		auto pBindColumn = static_cast<CBindTextColumn<T>*>(m_pColumn);
		auto pBindGrid = static_cast<CBindGridView<T>*>(m_pSheet);
		pBindColumnm->GetGetter()(m_pSheet->GetItemsSource()[pBindRow->GetIndex<AllTag>()]);
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<CBindRow<T>*>(m_pRow);
		auto pBindColumn = static_cast<CBindTextColumn<T>*>(m_pColumn);
		auto pBindGrid = static_cast<CBindGridView<T>*>(m_pSheet);
		pBindColumnm->GetSetter()(m_pSheet->GetItemsSource()[pBindRow->GetIndex<AllTag>()], str);
	}

};