#pragma once
#include "Row.h"
#include "IBindSheet.h"
#include "BindGridView.h"
#include "Debug.h"
#include "MyMPL.h"

template<typename T>
class CBindRow :public CRow
{
public:
	CBindRow(CGridView* pSheet)
		:CRow(pSheet){ }

	template<typename U>
	typename std::enable_if_t<std::is_same_v<T, U>, U&>
	GetItem()
	{
		if (auto p = dynamic_cast<IBindSheet<T>*>(this->m_pGrid)) {
			auto index = this->GetIndex<AllTag>() - this->m_pGrid->GetFrozenCount<RowTag>();
			return p->GetItemsSource().get_unconst()->at(index);		
		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}

	template<typename U>
	typename std::enable_if_t<!std::is_same_v<T, U> && is_tuple_v<T>, U&>
	GetItem()
	{
		if (auto p = dynamic_cast<IBindSheet<T>*>(this->m_pGrid)) {
			auto index = this->GetIndex<AllTag>() - this->m_pGrid->GetFrozenCount<RowTag>();
			return std::get<U>(p->GetItemsSource().get_unconst()->at(index));		
		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}

	template<class U>
	const U& GetItem() const
	{
		return const_cast<CBindRow&>(*this).GetItem<U>();
	}
};