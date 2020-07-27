#pragma once
#include "GridView.h"
#include "BindRow.h"
#include "observable.h"
#include "MyString.h"
#include "MyXmlSerializer.h"
#include "IBindSheet.h"
#include "Cursorer.h"


template<typename... TItems>
class CBindGridView :public CGridView
{
protected:
	std::shared_ptr<observable_vector<std::tuple<TItems...>>> m_spItemsSource;

public:
	template<typename... TArgs> 
	CBindGridView(std::shared_ptr<GridViewProperty>& spGridViewProp,
				  std::shared_ptr<observable_vector<std::tuple<TItems...>>> spItemsSource = nullptr,
				  TArgs... args)
		:CGridView(spGridViewProp), m_spItemsSource(spItemsSource)
	{
		//ItemsSource
		if (!m_spItemsSource) {
			m_spItemsSource = std::make_shared<observable_vector<std::tuple<TItems...>>>();
		}
		//VectorChanged
		auto& itemsSource = GetItemsSource();
		auto funVectorChanged =
			[this](const NotifyVectorChangedEventArgs<std::tuple<TItems...>>& e)->void {
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
					for (const auto& colPtr : m_allCols) {
						if (auto pMapCol = std::dynamic_pointer_cast<CMapColumn>(colPtr)) {
							pMapCol->Erase(const_cast<CRow*>(spRow.get()));
						}
					}
					EraseRow(spRow);
					break;
				}
				case NotifyVectorChangedAction::Reset:
					m_allRows.idx_erase(m_allRows.begin() + m_frozenRowCount, m_allRows.end());
					break;
				default:
					break;
			}
		};

		if (itemsSource.VectorChanged) {
			itemsSource.VectorChanged =
				[existingFun = itemsSource.VectorChanged, newFun = funVectorChanged](const NotifyVectorChangedEventArgs<std::tuple<TItems...>>& e)->void {
				existingFun(e);
				newFun(e);
			};
		} else {
			itemsSource.VectorChanged = funVectorChanged;
		}

		//TArg...
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

		std::vector<std::shared_ptr<CColumn>> columns;
		columns = ::get(arg<"columns"_s>(), args..., default_(columns));
		for (auto& spCol : columns) {
			spCol->SetSheetPtr(this);
			PushColumn(spCol);
		}

		//PushNewRow
		for (auto& tup : itemsSource) {
			PushRow(std::make_shared<CBindRow<TItems...>>(this));
		}
	}

	observable_vector<std::tuple<TItems...>>& GetItemsSource() { return *m_spItemsSource; }
	//std::vector<std::tuple<TItems...>>& GetSelectedItems() { return m_funSelItems(); }

	/******************/
	/* Window Message */
	/******************/

	/****************/
	/* StateMachine */
	/****************/
};