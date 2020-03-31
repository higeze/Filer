#pragma once
#include "TextCell.h"
#include "BindGridView.h"
#include "BindRow.h"
#include "BindTextColumn.h"

enum class EditMode
{
	None,
	LButtonDownEdit,
	FocusedSingleClickEdit,
};


template<typename TItem>
class CBindTextCell :public CEditableCell
{
private:
	EditMode m_editMode = EditMode::LButtonDownEdit;
public:
	//CBindTextCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	//	:CEditableCell(pSheet, pRow, pColumn, spProperty, pMenu){}
	//struct Args
	//{
	//	CSheet* SheetPtr;
	//	CRow* RowPtr;
	//	CColumn* ColPtr;
	//	std::shared_ptr<CellProperty> PropPtr;
	//	EditMode EditMode = EditMode::LButtonDownEdit;
	//};


	template<typename... Args>
	CBindTextCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:CEditableCell(pSheet, pRow, pColumn, spProperty)
	{
		m_editMode = ::get(arg<"editmode"_s>(), args...);
	}

	virtual ~CBindTextCell() = default;

	EditMode GetEditMode()const { return m_editMode; }
	void SetEditMode(const EditMode& value)const { m_editMode = value; }

	virtual std::wstring GetString() override
	{
		auto pBindRow = static_cast<CBindRow<TItem>*>(m_pRow);
		auto pBindColumn = static_cast<CBindTextColumn<TItem>*>(m_pColumn);
		return pBindColumn->GetGetter()(pBindRow->GetItem());
	}

	virtual void SetStringCore(const std::wstring& str) override
	{
		auto pBindRow = static_cast<CBindRow<TItem>*>(m_pRow);
		auto pBindColumn = static_cast<CBindTextColumn<TItem>*>(m_pColumn);
		pBindColumn->GetSetter()(pBindRow->GetItem(), str);
	}

	virtual void OnLButtonDown(const LButtonDownEvent& e) override
	{
		if (m_editMode == EditMode::LButtonDownEdit) {
			OnEdit(e);
		}
	}

	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e)
	{
		if (m_editMode == EditMode::FocusedSingleClickEdit) {
			if (GetDoubleFocused()) {
				OnEdit(e);
			}
		}

	}



};