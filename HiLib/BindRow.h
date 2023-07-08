#pragma once
#include "Row.h"
#include "Sheet.h"
#include "Debug.h"
#include "IBindSheet.h"
#include "BindGridView.h"

#include "reactive_vector.h"

template<typename... TItems>
class CBindRow :public CRow
{
public:
	CBindRow(CSheet* pSheet)
		:CRow(pSheet){ }

	std::tuple<TItems...>& GetTupleItems()
	{
		if (auto pBindSheet = dynamic_cast<IBindSheet2<TItems...>*>(this->m_pSheet)) {
			auto& itemsSource = pBindSheet->GetItemsSource();
			auto index = GetIndex<AllTag>() - this->m_pSheet->GetFrozenCount<RowTag>();
			return itemsSource->get_unconst()[index];		
		} if(auto pBindSheet = dynamic_cast<IBindSheet<TItems...>*>(this->m_pSheet)){
			auto& itemsSource = pBindSheet->GetItemsSource();
			auto index = GetIndex<AllTag>() - this->m_pSheet->GetFrozenCount<RowTag>();
			return itemsSource[index];
		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}

	template<typename TItem> 
	TItem& GetItem()
	{
		return std::get<TItem>(GetTupleItems());
	}
};

template<typename... TItems>
class CBindRow2 :public CRow
{
public:
	CBindRow2(CSheet* pSheet)
		:CRow(pSheet){ }

	std::tuple<TItems...>& GetTupleItems()
	{
		if (auto pBindSheet = dynamic_cast<IBindSheet2<TItems...>*>(this->m_pSheet)) {
			auto& itemsSource = pBindSheet->GetItemsSource();
			auto index = GetIndex<AllTag>() - this->m_pSheet->GetFrozenCount<RowTag>();
			return itemsSource.get_unconst()[index];		
		} if(auto pBindSheet = dynamic_cast<IBindSheet<TItems...>*>(this->m_pSheet)){
			auto& itemsSource = pBindSheet->GetItemsSource();
			auto index = GetIndex<AllTag>() - this->m_pSheet->GetFrozenCount<RowTag>();
			return itemsSource[index];
		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}

	template<typename TItem> TItem& GetItem()
	{
		return std::get<TItem>(GetTupleItems());
	}
};
