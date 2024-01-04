#include "BindColumn.h"
#include "BindGridView.h"
#include "Row.h"
#include "TextCell.h"


any_tuple& CBindColumn::GetTupleItems()
{
	if(auto p = dynamic_cast<CBindGridView2*>(this->m_pSheet)){
		auto& itemsSource = p->ItemsSource;
		auto index = GetIndex<AllTag>() - this->m_pSheet->GetFrozenCount<ColTag>();

		return itemsSource.get_unconst()->at(index);

	} else {
		throw std::exception(FILE_LINE_FUNC);
	}
}

const any_tuple& CBindColumn::GetTupleItems() const
{
	return const_cast<CBindColumn&>(*this).GetTupleItems();
}

std::shared_ptr<CCell>& CBindColumn::Cell(CRow* pRow )
{
	if (pRow->HasCell()) {
		return pRow->Cell(this);
	} else {
		THROW_FILE_LINE_FUNC;
	}
}

std::shared_ptr<CCell> CBindColumn::CellTemplate(CRow* pRow, CColumn* pColumn)
{
	return std::make_shared<CTextCell>(m_pSheet,pRow,pColumn,m_pSheet->GetCellProperty());
}

