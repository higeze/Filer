#pragma once
#include "MapColumn.h"
#include "CellProperty.h"

/**
 *  CParentRowHeaderColumn
 *  Most Left side column which show Row number and act as Row header
 */
class CRowIndexColumn:public CMapColumn
{
protected:

public:
	CRowIndexColumn(CGridView* pSheet = nullptr)
		:CMapColumn(pSheet, arg<"isminfit"_s>() = true, arg<"ismaxfit"_s>() = true){}

	virtual ~CRowIndexColumn() = default;

	virtual CColumn& ShallowCopy(const CColumn& column)override
	{
		CMapColumn::ShallowCopy(column);
		return *this;
	}
	virtual CRowIndexColumn* CloneRaw()const{return new CRowIndexColumn(*this);}
	std::shared_ptr<CRowIndexColumn> Clone()const{return std::shared_ptr<CRowIndexColumn>(CloneRaw());}

	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> HeaderHeaderCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn);
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn);

	virtual bool IsTrackable()const override { return true; }

	//virtual void RenderBackground(CDirect2DWrite* pDirect, const CRectF& rc);

};
