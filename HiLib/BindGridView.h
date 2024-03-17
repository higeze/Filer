#pragma once
#include "GridView.h"
#include "BindRow.h"
#include "BindColumn.h"
#include "BindTextColumn.h"
#include "MyString.h"
#include "MyXmlSerializer.h"
#include "Cursorer.h"
#include "MapRow.h"
#include "MapColumn.h"
#include "Celler.h"
#include "reactive_vector.h"
#include "IBindSheet.h"

#include "Cursorer.h"

//template<typename T> class CBindCursorer;
template<typename T, typename TRow, typename TCol> class CBindGridView;

template<typename T, typename TRow, typename TCol>
class CBindCursorer : public CCursorer
{
	using CCursorer::CCursorer;

public:

	void OnCursor(const std::shared_ptr<CCell>& cell)
	{
		if(!IsCursorTargetCell(cell)){
			return;
		}
	
		UpdateCursor(cell);
		cell->GetGridPtr()->DeselectAll();
		cell->GetRowPtr()->SetIsSelected(true);//Select
		if (auto pGrid = dynamic_cast<CBindGridView<T, TRow, TCol>*>(cell->GetGridPtr())){//dynamic_cast<CBindGridView<T>*>(cell->GetGridPtr())) {
			if (auto pRow = dynamic_cast<TRow*>(cell->GetRowPtr())){//dynamic_cast<CBindRow<T>*>(cell->GetRowPtr())) {
				pGrid->SelectedItem.set(pRow->GetItem<T>());
			}
		}
	}

	void OnCursorDown(const std::shared_ptr<CCell>& cell)
	{
		if (!IsCursorTargetCell(cell)) {
			return;
		}

		UpdateCursor(cell);
		m_isDragPossible = true;
		cell->GetRowPtr()->SetIsSelected(true);//Select
	}

	void OnCursorUp(const std::shared_ptr<CCell>& cell)
	{
		if (!IsCursorTargetCell(cell)) {
			return;
		}

		if (m_isDragPossible) {
			m_isDragPossible = false;
			cell->GetGridPtr()->DeselectAll();
			cell->GetRowPtr()->SetIsSelected(true);//Select
		}
	}

	void OnCursorLeave(const std::shared_ptr<CCell>& cell)
	{
		if (!IsCursorTargetCell(cell)) {
			return;
		}
		if (m_isDragPossible) {
			m_isDragPossible = false;
			//cell->GetGridPtr()->DeselectAll();
			//cell->GetRowPtr()->SetSelected(true);//Select
		}
	}

	void OnCursorCtrl(const std::shared_ptr<CCell>& cell)
	{
		if (!IsCursorTargetCell(cell)) {
			return;
		}
		UpdateCursor(cell);

		cell->GetRowPtr()->SetIsSelected(!cell->GetRowPtr()->GetIsSelected());//Select
	}
};



enum class BindType
{
	Row,
	Column,
};

template<typename T, typename TRow = CBindRow<T>, typename TCol = CBindColumn<T>>
class CBindGridView :public CGridView, public IBindSheet<T>
{
protected:
	BindType m_bindType;
	std::vector<std::shared_ptr<CColumn>> m_initColumns;
	std::vector<std::shared_ptr<CRow>> m_initRows;

public:
	reactive_vector_ptr<T> ItemsSource;
	reactive_property_ptr<T> SelectedItem;
	reactive_vector_ptr<T>& GetItemsSource() override { return ItemsSource; }

public:
	template<typename... TArgs> 
	CBindGridView(
		CD2DWControl* pParentControl,
		const std::shared_ptr<GridViewProperty>& spGridViewProp,
		TArgs... args)
		:CGridView(pParentControl, spGridViewProp),
		ItemsSource(),
		m_bindType(BindType::Row)
	{
		m_spCursorer = std::make_shared<CBindCursorer<T, TRow, TCol>>();

		//TArg...
		m_bindType = ::get(arg<"bindtype"_s>(), args..., default_(BindType::Row));

		ItemsSource = ::get(arg<"itemssource"_s>(), args..., default_(reactive_vector_ptr<T>()));

		m_pHeaderColumn = ::get(arg<"hdrcol"_s>(), args..., default_(nullptr));
		if (m_pHeaderColumn) {
			m_pHeaderColumn->SetSheetPtr(this);
			PushColumn(m_pHeaderColumn);
		}

		m_pNameColumn = ::get(arg<"namecol"_s>(), args..., default_(nullptr));
		if (m_pNameColumn) {
			m_pNameColumn->SetSheetPtr(this);
			PushColumn(m_pNameColumn);
		}

		//m_pHeaderRow = ::get(arg<"hdrrow"_s>(), args..., default_(nullptr));
		//if (m_pHeaderRow){
		//	m_pHeaderRow->SetSheetPtr(this);
		//	PushRow(m_pHeaderRow);
		//}

		m_pNameHeaderRow = ::get(arg<"namerow"_s>(), args..., default_(nullptr));
		if (m_pNameHeaderRow) {
			m_pNameHeaderRow->SetSheetPtr(this);
			PushRow(m_pNameHeaderRow);
		}

		m_pFilterRow = ::get(arg<"fltrow"_s>(), args..., default_(nullptr));
		if (m_pFilterRow) {
			m_pFilterRow->SetSheetPtr(this);
			PushRow(m_pFilterRow);
		}

		m_frozenColumnCount = ::get(arg<"frzcolcnt"_s>(), args..., default_(0));
		m_frozenRowCount = ::get(arg<"frzrowcnt"_s>(), args..., default_(0));

		//std::vector<std::shared_ptr<CColumn>> columns;
		m_initColumns = ::get(arg<"columns"_s>(), args..., default_(m_initColumns));

		//std::vector<std::shared_ptr<CRow>> rows;
		m_initRows = ::get(arg<"rows"_s>(), args..., default_(m_initRows));
	}

	virtual std::shared_ptr<CCell> Cell(const std::shared_ptr<CRow>& spRow, const std::shared_ptr<CColumn>& spColumn)override
	{
		return CGridView::Cell(spRow, spColumn);
	}
	virtual std::shared_ptr<CCell> Cell(const std::shared_ptr<CColumn>& spColumn, const std::shared_ptr<CRow>& spRow)override
	{
		return CGridView::Cell(spRow, spColumn);
	}
	virtual std::shared_ptr<CCell> Cell(const CPointF& pt)override
	{
		return CGridView::Cell(pt);
	}
	std::shared_ptr<CCell> Cell(CRow* pRow, CColumn* pColumn) override
	{
		auto& container = m_allCells.get<rowcol_tag>();
		auto iter = container.find(MAKELONGLONG(pRow, pColumn));
		if (iter == container.end()) {
			if (m_bindType == BindType::Row) {
				/*if (pRow == GetHeaderRowPtr().get()) {
					iter = container.insert(pColumn->HeaderCellTemplate(pRow, pColumn)).first;
				} else */if (pRow == GetNameHeaderRowPtr().get()) {
					iter = container.insert(pColumn->NameHeaderCellTemplate(pRow, pColumn)).first;
				} else if (pRow == GetFilterRowPtr().get()) {
					iter = container.insert(pColumn->FilterCellTemplate(pRow, pColumn)).first;
				} else {
					iter = container.insert(pColumn->CellTemplate(pRow, pColumn)).first;
				}
			} else if (m_bindType == BindType::Column) {
				/*if (pRow == GetHeaderRowPtr().get()) {
					iter = container.insert(pRow->HeaderCellTemplate(pRow, pColumn)).first;
				} else*/ if (pRow == GetNameHeaderRowPtr().get()) {
					iter = container.insert(pRow->NameHeaderCellTemplate(pRow, pColumn)).first;
				} else if (pRow == GetFilterRowPtr().get()) {
					iter = container.insert(pRow->FilterCellTemplate(pRow, pColumn)).first;
				} else {
					iter = container.insert(pRow->CellTemplate(pRow, pColumn)).first;
				}
			}
		}
		return const_cast<std::shared_ptr<CCell>&>(*iter);

	}


	std::vector<int> GetSelectedIndexes() 
	{
		std::vector<int> indexes;
		for (const auto& rowPtr : m_visRows) {
			if (rowPtr->GetIsSelected()) {
				indexes.push_back(rowPtr->GetIndex<AllTag>());
			}
		}
		return indexes;
	}
	/******************/
	/* Window Message */
	/******************/
	void subscribe_detail_row(const notify_container_changed_event_args<std::vector<T>>& e)
	{
		switch (e.action) {
			case notify_container_changed_action::push_back:
				PushRow(std::make_shared<TRow>(this, GetCellProperty()));
				break;
			case notify_container_changed_action::insert:
				InsertRow(e.new_starting_index, std::make_shared<TRow>(this, GetCellProperty()));
				break;
			case notify_container_changed_action::erase:
			{
				auto spRow = m_allRows[e.old_starting_index + m_frozenRowCount];
				m_allCells.get<row_tag>().erase(spRow.get());
				EraseRow(spRow);//TODO Refactor
				m_spCursorer->Clear();
				m_spCeller->Clear();
				break;
			}
			case notify_container_changed_action::reset:
			{
				for (size_t i = m_frozenRowCount; i < m_allRows.size(); i++) {
					m_allCells.get<row_tag>().erase(m_allRows[i].get());
				}
				m_allRows.idx_erase(m_allRows.begin() + m_frozenRowCount, m_allRows.end());
				//m_allCells.clear();
				m_spCursorer->Clear();//TOOD Refactor
				m_spCeller->Clear();
				for (auto& tup : e.new_items) {
					PushRow(std::make_shared<TRow>(this, GetCellProperty()));
				}
				//PostUpdate(Updates::All);
				//SubmitUpdate();
				break;
			}
			default:
				break;
		}
	}

	void subscribe_detail_column(const notify_container_changed_event_args<std::vector<T>>& e)
	{
		switch (e.action) {
			case notify_container_changed_action::push_back:
				this->PushColumn(std::make_shared<TCol>(this));
				break;
			case notify_container_changed_action::insert:
				this->InsertColumn(e.new_starting_index, std::make_shared<TCol>(this));
				break;
			case notify_container_changed_action::erase:
			{
				auto spColumn = m_allCols[e.old_starting_index + m_frozenColumnCount];
				m_allCells.get<col_tag>().erase(spColumn.get());
				EraseColumn(spColumn);
				break;
			}
			case notify_container_changed_action::reset:
				m_allCols.idx_erase(m_allCols.begin() + m_frozenColumnCount, m_allCols.end());
				m_allCells.clear();
				for (auto& tup : e.new_items) {
					PushColumn(std::make_shared<TCol>(this));
				}

				break;
			default:
				break;
		}
	}


	void OnCreate(const CreateEvt& e) override
	{
		CD2DWControl::OnCreate(e);

		switch (m_bindType) {
			case BindType::Row:
			{
				//VectorChanged
				ItemsSource.subscribe([&](const auto& e) 
				{
					//ItemsSource->block_subject();
					this->subscribe_detail_row(e); 
					//ItemsSource->unblock_subject();
				}, shared_from_this());

				//PushColumn
				for (auto& spCol : m_initColumns) {
					spCol->SetSheetPtr(this);
					PushColumn(spCol);
				}

				//PushRow
				for (size_t i = 0; i < ItemsSource->size(); i++) {
					PushRow(std::make_shared<TRow>(this, GetCellProperty()));
				}
			}
			break;
			case BindType::Column:
			{
				//VectorChanged
				ItemsSource.subscribe([this](const auto& e) { this->subscribe_detail_column(e); }, shared_from_this());

				//PushRow
				for (auto& spRow : m_initRows) {
					spRow->SetSheetPtr(this);
					PushRow(spRow);
				}

				//PushColumn
				for (size_t i = 0; i < ItemsSource->size(); i++) {
					PushColumn(std::make_shared<TCol>(this));
				}

			}
			break;
			default:
			break;
		}

		UpdateAll();
		SubmitUpdate();
	}

	/****************/
	/* StateMachine */
	/****************/
};
