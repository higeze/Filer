#pragma once
#include "Row.h"
#include "IBindSheet.h"
#include "Debug.h"

template<typename TItem>
class CBindRow :public CRow
{
public:
	using CRow::CRow;

	TItem& GetItem()
	{
		if (auto pBindSheet = dynamic_cast<IBindSheet<TItem>*>(m_pSheet)) {
			auto& itemsSource = pBindSheet->GetItemsSource();
			auto index = GetIndex<AllTag>() - m_pSheet->GetFrozenCount<RowTag>();
			return pBindSheet->GetItemsSource()[GetIndex<AllTag>() - m_pSheet->GetFrozenCount<RowTag>()];
		} else {
			throw std::exception(FILE_LINE_FUNC);
		}
	}
};
