#pragma once
#include "GridView.h"
#include "BindRow.h"
#include "BindColumn.h"
#include "ReactiveProperty.h"
#include "MyString.h"
#include "MyXmlSerializer.h"
#include "Cursorer.h"
#include "MapRow.h"
#include "MapColumn.h"

#include "IBindSheet.h"


template<typename... TItems>
class CBindGridView :public CGridView, public IBindSheet<TItems...>
{
protected:
	std::shared_ptr<ReactiveVectorProperty<std::tuple<TItems...>>> m_spItemsSource;

	std::vector<std::shared_ptr<CColumn>> m_initColumns;
	std::vector<std::shared_ptr<CRow>> m_initRows;


public:
	template<typename... TArgs> 
	CBindGridView(
		CD2DWControl* pParentControl,
		const std::shared_ptr<GridViewProperty>& spGridViewProp,
		const std::shared_ptr<ReactiveVectorProperty<std::tuple<TItems...>>> spItemsSource = nullptr,
		TArgs... args)
		:CGridView(pParentControl, spGridViewProp), m_spItemsSource(spItemsSource)
	{
		//TArg...
		m_bindType = ::get(arg<"bindtype"_s>(), args..., default_(BindType::Row));

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

		m_pHeaderRow = ::get(arg<"hdrrow"_s>(), args..., default_(nullptr));
		if (m_pHeaderRow){
			m_pHeaderRow->SetSheetPtr(this);
			PushRow(m_pHeaderRow);
		}

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

		//ItemsSource
		if (!m_spItemsSource) {
			m_spItemsSource = std::make_shared<ReactiveVectorProperty<std::tuple<TItems...>>>();
		}
	}

	ReactiveVectorProperty<std::tuple<TItems...>>& GetItemsSource() override { return *(this->m_spItemsSource); }
	void SetItemsSource(const ReactiveVectorProperty<std::tuple<TItems...>>& items) { this->m_spItemsSource->set(items); }
	std::vector<std::tuple<TItems...>>& GetSelectedItems() { return this->m_funSelItems(); }

	/******************/
	/* Window Message */
	/******************/
	void subscribe_detail_row(const NotifyVectorChangedEventArgs<std::tuple<TItems...>>& e)
	{
		switch (e.Action) {
			case NotifyVectorChangedAction::Add:
				PushRow(std::make_shared<CBindRow<TItems...>>(this));
				break;
			case NotifyVectorChangedAction::Insert:
				InsertRow(e.NewStartingIndex, std::make_shared<CBindRow<TItems...>>(this));
				break;
			case NotifyVectorChangedAction::Remove:
			{
				auto spRow = m_allRows[e.OldStartingIndex + m_frozenRowCount];
				m_allCells.get<row_tag>().erase(spRow.get());
				EraseRow(spRow);
				break;
			}
			case NotifyVectorChangedAction::Reset:
				m_allRows.idx_erase(m_allRows.begin() + m_frozenRowCount, m_allRows.end());
				m_allCells.clear();
				for (auto& tup : e.NewItems) {
					PushRow(std::make_shared<CBindRow<TItems...>>(this));
				}

				break;
			default:
				break;
		}
	}

	void subscribe_detail_column(const NotifyVectorChangedEventArgs<std::tuple<TItems...>>& e)
	{
		switch (e.Action) {
			case NotifyVectorChangedAction::Add:
				this->PushColumn(std::make_shared<CBindColumn<TItems...>>(this));
				break;
			case NotifyVectorChangedAction::Insert:
				this->InsertColumn(e.NewStartingIndex, std::make_shared<CBindColumn<TItems...>>(this));
				break;
			case NotifyVectorChangedAction::Remove:
			{
				auto spColumn = m_allCols[e.OldStartingIndex + m_frozenColumnCount];
				m_allCells.get<col_tag>().erase(spColumn.get());
				EraseColumn(spColumn);
				break;
			}
			case NotifyVectorChangedAction::Reset:
				m_allCols.idx_erase(m_allCols.begin() + m_frozenColumnCount, m_allCols.end());
				m_allCells.clear();
				for (auto& tup : e.NewItems) {
					PushColumn(std::make_shared<CBindColumn<TItems...>>(this));
				}

				break;
			default:
				break;
		}
	}


	void OnCreate(const CreateEvt& e) override
	{
		switch (m_bindType) {
			case BindType::Row:
			{
				//VectorChanged
				ReactiveVectorProperty<std::tuple<TItems...>>& itemsSource = GetItemsSource();
				itemsSource.Subscribe([&](const auto& e) { this->subscribe_detail_row(e); });

				//PushColumn
				for (auto& spCol : m_initColumns) {
					spCol->SetSheetPtr(this);
					PushColumn(spCol);
				}

				//PushRow
				for (auto& tup : itemsSource) {
					PushRow(std::make_shared<CBindRow<TItems...>>(this));
				}
			}
			break;
			case BindType::Column:
			{
				//VectorChanged
				ReactiveVectorProperty<std::tuple<TItems...>>& itemsSource = GetItemsSource();
				itemsSource.Subscribe([&](const auto& e) { this->subscribe_detail_column(e); });

				//PushRow
				for (auto& spRow : m_initRows) {
					spRow->SetSheetPtr(this);
					PushRow(spRow);
				}

				//PushColumn
				for (auto& tup : itemsSource) {
					PushColumn(std::make_shared<CBindColumn<TItems...>>(this));
				}

			}
			break;
			default:
			break;
		}

		CD2DWControl::OnCreate(e);
	}

	/****************/
	/* StateMachine */
	/****************/
};

