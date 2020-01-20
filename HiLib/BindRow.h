#pragma once
#include "Row.h"

template<typename T>
class CBindRow :public CParentRow
{
public:
	CParentRow::CParentRow;
};

//CToDoGridView::CToDoGridView(std::shared_ptr<ToDoGridViewProperty>& spToDoGridViewProp)
//{
//	//Insert rows
//	m_rowNameHeader = std::make_shared<CParentHeaderRow>(this);
//	m_rowFilter = std::make_shared<CParentRow>(this);
//
//	InsertRowNotify(CRow::kMinIndex, m_rowFilter, false);
//	InsertRowNotify(CRow::kMinIndex, m_rowNameHeader, false);
//
//	//Insert columns if not initialized
//
//		InsertColumnNotify(CColumn::kMinIndex, std::make_shared<CParentRowHeaderColumn>(this));
//		m_pNameColumn = std::make_shared<CToDoNameColumn>(this);
//		InsertColumnNotiry(CColumn::kMaxIndex, m_pNameColumn );
//		insertFun(std::make_shared<CFileExtColumn>(this), CColumn::kMaxIndex);
//		insertFun(std::make_shared<CFileSizeColumn>(this, GetFilerGridViewPropPtr()->FileSizeArgsPtr), CColumn::kMaxIndex);
//		insertFun(std::make_shared<CFileLastWriteColumn>(this, GetFilerGridViewPropPtr()->FileTimeArgsPtr), CColumn::kMaxIndex);
//	}
//}
//~CToDoGridView() = default;
