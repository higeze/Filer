#pragma once
#include "Band.h"
#include "named_arguments.h"
#include "UIElement.h"

class CCell;
class Sheet;
class CColumn;
struct CellProperty;
struct RowTag;

class CRow:public CBand
{
public:
	typedef RowTag Tag;
protected:
	bool m_isVirtualMeasureValid = false;
public:
	template<typename... Args>
	CRow(CGridView* pSheet, Args... args)
		:CBand(pSheet)
	{
		m_isMinLengthFit = ::get(arg<"isminfit"_s>(), args..., default_(true));
		m_isMaxLengthFit = ::get(arg<"ismaxfit"_s>(), args..., default_(true));
		m_minLength = ::get(arg<"minheight"_s>(), args..., default_(2.f));
		m_maxLength = ::get(arg<"maxheight"_s>(), args..., default_(100.f));
	}
	virtual ~CRow(){}

	virtual bool HasCell()const { return false; }
	virtual std::shared_ptr<CCell>& Cell(CColumn* pCol) { throw std::exception("Not implemented"); }

	//Start/End	
	virtual FLOAT GetStart() override;
	virtual FLOAT GetEnd() override;
	//Top/Bottom
	virtual FLOAT GetTop(){ return GetStart(); }
	virtual void SetTop(const FLOAT top, bool notify = true) { SetStart(top, notify); }
	virtual FLOAT GetBottom() { return GetEnd(); }
	virtual CRectF GetRectInWnd();

	//Length
	virtual FLOAT GetLength() override;
	virtual FLOAT GetVirtualLength() override;
	virtual FLOAT GetFitLength() override;
	//Height
	virtual FLOAT GetHeight() { return GetLength(); }
	virtual FLOAT GetVirtualHeight() { return GetVirtualLength(); }
	virtual FLOAT GetFitHeight() { return GetFitLength(); }
	virtual void SetHeight(const FLOAT height, bool notify = true){ return SetLength(height, notify); }
	virtual void FitHeight() { FitLength(); }
	virtual void BoundHeight(){ BoundLength(); }


	virtual void SetIsVisible(const bool& bVisible, bool notify = true)override;
	virtual void SetIsSelected(const bool& bSelected);
	virtual void OnCellPropertyChanged(CCell* pCell, const wchar_t* name) override;
	virtual void OnPropertyChanged(const wchar_t* name) override;
//	virtual SizingType GetSizingType()const override { return SizingType::None; }

	virtual std::shared_ptr<CCell> NameHeaderCellTemplate(CRow* pRow, CColumn* pColumn) { return nullptr; }
	virtual std::shared_ptr<CCell> HeaderCellTemplate(CRow* pRow, CColumn* pColumn) { return nullptr; }
	virtual std::shared_ptr<CCell> FilterCellTemplate(CRow* pRow, CColumn* pColumn) { return nullptr; }
	virtual std::shared_ptr<CCell> CellTemplate(CRow* pRow, CColumn* pColumn) { return nullptr; };

	//virtual void RenderBackground(CDirect2DWrite* pDirect, const CRectF& rc);
	//virtual void RenderHighlight(CDirect2DWrite* pDirect, const CRectF& rc);
	//virtual void OnPaint(const PaintEvent& e);
};

class CHeaderRow:public CRow
{
public:
	using CRow::CRow;
	virtual bool IsTrackable()const override { return true; }
};
