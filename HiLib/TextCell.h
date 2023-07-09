#pragma once
#include "Cell.h"
#include <unordered_map>

class CTextBox;

struct equal_double
//	:public std::binary_function<const double&,const double&,bool>
{
private:
	double m_dEpsilon;
public:
	equal_double(double dEpsilon=1e-10):m_dEpsilon(dEpsilon){}
	bool operator()(const double& dLhs,const double& dRhs)
	{
		double dRelativeEpsilon=(std::max)(abs(dLhs),abs(dRhs))*m_dEpsilon;
		double dDiff=dLhs-dRhs;
		return (*((__int64*) & dDiff) & 0x7FFFFFFFFFFFFFFF) <= (*((__int64*)&dRelativeEpsilon));
	}
};

struct equal_wstring_compare_in_double
//	:public std::binary_function<const std::wstring&,const std::wstring&,bool>
{
private:
	equal_double m_ed;
public:
	equal_wstring_compare_in_double():m_ed(){}
	bool operator()(const std::wstring& lhs,const std::wstring& rhs){
		if( lhs.empty() && rhs.empty() ){
			return true;
		}else if( lhs.empty() || rhs.empty() ){
			return false;
		}else{
			return m_ed(_wtof(lhs.c_str()),_wtof(rhs.c_str()));
		}
	}
};

enum class EditMode
{
	None,
	ReadOnly,
	LButtonDownEdit,
	FocusedSingleClickEdit,
	ExcelLike,
};

class CTextCell:public CCell
{
private:
	EditMode m_editMode = EditMode::LButtonDownEdit;
//protected:
//	std::wstring m_text;


public:
	template<typename... Args>
	CTextCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, Args... args)
		:CCell(pSheet, pRow, pColumn, spProperty, args...)
	{
		m_editMode = ::get(arg<"editmode"_s>(), args..., default_(EditMode::ReadOnly));
		//m_text = ::get(arg<"text"_s>(), args..., default_(std::wstring()));
	}
	virtual ~CTextCell() = default;
	//Accesser
	EditMode GetEditMode()const { return m_editMode; }
	void SetEditMode(const EditMode& value) { m_editMode = value; }

	//virtual std::wstring GetString() override;
	//virtual void SetStringCore(const std::wstring& str) override;


	virtual void PaintContent(CDirect2DWrite* pDirect, CRectF rcPaint) override;

	//virtual CSize MeasureSize(CDC* pDC);
	//virtual CSize MeasureSizeWithFixedWidth(CDC* pDC);
	virtual CSizeF MeasureContentSize(CDirect2DWrite* direct) override;
	virtual CSizeF MeasureContentSizeWithFixedWidth(CDirect2DWrite* direct) override;
	virtual CRectF GetEditRect() const;
	virtual void OnEdit(const Event& e);

	virtual void OnLButtonDown(const LButtonDownEvent& e);
	virtual void OnLButtonDblClk(const LButtonDblClkEvent& e);
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e);
	virtual void OnKeyDown(const KeyDownEvent& e) override;
	virtual void OnChar(const CharEvent& e);

	virtual bool CanSetStringOnEditing()const{return false;}
};

