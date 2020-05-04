#pragma once
#include "Row.h"
#include "IBindSheet.h"
#include "Debug.h"
#include "BindGridView.h"


template<typename... TItems>
class CBindRow :public CRow
{
public:
	CBindRow(CSheet* pSheet)
		:CRow(pSheet){ }

	std::tuple<TItems...>& GetTupleItems()
	{
		if (auto pBindSheet = dynamic_cast<CBindGridView<TItems...>*>(m_pSheet)) {
			auto& itemsSource = pBindSheet->GetItemsSource();
			auto index = GetIndex<AllTag>() - m_pSheet->GetFrozenCount<RowTag>();
			return itemsSource[index];
		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}

	template<typename TItem> TItem& GetItem()
	{
		return std::get<TItem>(GetTupleItems());
	}

	//virtual std::shared_ptr<CShellFile> GetFilePointer() { return std::get<std::shared_ptr<CShellFile>>(GetItem()); }
	//virtual void SetFilePointer(const std::shared_ptr<CShellFile>& spFile) { std::get<std::shared_ptr<CShellFile >>(GetItem()) = spFile; }
};

//template<typename TItem>
//class CBindRowTest :public CRow
//{
//public:
//	using CRow::CRow;
//
//	TItem& GetTupleItems()
//	{
//		if (auto pBindSheet = dynamic_cast<IBindSheet<TItem>*>(m_pSheet)) {
//			auto& itemsSource = pBindSheet->GetItemsSource();
//			auto index = GetIndex<AllTag>() - m_pSheet->GetFrozenCount<RowTag>();
//			return pBindSheet->GetItemsSource()[GetIndex<AllTag>() - m_pSheet->GetFrozenCount<RowTag>()];
//		} else {
//			throw std::exception(FILE_LINE_FUNC);
//		}
//	}
//};
