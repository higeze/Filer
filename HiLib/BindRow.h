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
	CBindRow(CSheet* pSheet, std::shared_ptr<CellProperty> spProperty)
		:CRow(pSheet, spProperty){ }

	template<typename U>
	U& GetItem()
	{
		if (auto p = dynamic_cast<IBindSheet<T>*>(this->m_pSheet)) {
			auto index = this->GetIndex<AllTag>() - this->m_pSheet->GetFrozenCount<RowTag>();
			return p->GetItemsSource().get_unconst()->at(index);		
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

template<typename... V>
class CBindRow<std::tuple<V...>> : public CRow
{
public:
	CBindRow(CSheet* pSheet, std::shared_ptr<CellProperty> spProperty)
		:CRow(pSheet, spProperty){}

	template<typename U>
	U& GetItem()
	{
		if (auto p = dynamic_cast<IBindSheet<std::tuple<V...>>*>(this->m_pSheet)) {
			auto index = this->GetIndex<AllTag>() - this->m_pSheet->GetFrozenCount<RowTag>();
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
