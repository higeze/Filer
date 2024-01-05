#pragma once
#include "Column.h"
#include "IBindSheet.h"
#include "TextCell.h"
#include "Debug.h"
#include "MyMPL.h"

template<typename T>
class CBindColumn :public CColumn
{
public:
	CBindColumn(CSheet* pSheet)
		:CColumn(pSheet){ }

	template<typename U>
	U& GetItem()
	{
		if (auto p = dynamic_cast<IBindSheet<T>*>(this->m_pSheet)) {
			auto index = GetIndex<AllTag>() - this->m_pSheet->GetFrozenCount<ColTag>();
			return p->GetItemsSource().get_unconst()->at(index);		
		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}

	template<class U>
	const U& GetItem() const
	{
		return const_cast<CBindColumn&>(*this).GetItem<U>();
	}

	std::shared_ptr<CCell>& Cell(CRow* pRow )
	{
		if (pRow->HasCell()) {
			return pRow->Cell(this);
		} else {
			THROW_FILE_LINE_FUNC;
		}
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CTextCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
	}

};

template<typename... V>
class CBindColumn<std::tuple<V...>> : public CColumn
{
public:
	CBindColumn(CSheet* pSheet)
		:CColumn(pSheet){}

	template<typename U>
	U& GetItem()
	{
		if (auto p = dynamic_cast<IBindSheet<std::tuple<V...>>*>(this->m_pSheet)) {
			auto index = GetIndex<AllTag>() - this->m_pSheet->GetFrozenCount<ColTag>();
			return std::get<U>(p->GetItemsSource().get_unconst()->at(index));		
		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}

	template<class U>
	const U& GetItem() const
	{
		return const_cast<CBindColumn&>(*this).GetItem<U>();
	}

	std::shared_ptr<CCell>& Cell(CRow* pRow )
	{
		if (pRow->HasCell()) {
			return pRow->Cell(this);
		} else {
			THROW_FILE_LINE_FUNC;
		}
	}

	std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn)
	{
		return std::make_shared<CTextCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
	}
};

