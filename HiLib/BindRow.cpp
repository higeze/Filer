#include "BindRow.h"
#include "BindGridView.h"

any_tuple& CBindRow::GetTupleItems()
{
	if (auto p = dynamic_cast<CBindGridView2*>(this->m_pSheet)) {
		auto& itemsSource = p->ItemsSource;
		auto index = GetIndex<AllTag>() - this->m_pSheet->GetFrozenCount<RowTag>();
		return itemsSource.get_unconst()->at(index);		
	} else {
		throw std::exception(FILE_LINE_FUNC);
	}
}

const any_tuple& CBindRow::GetTupleItems() const
{
	return const_cast<CBindRow&>(*this).GetTupleItems();
}
