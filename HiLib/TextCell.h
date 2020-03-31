#pragma once
#include "Cell.h"
#include <unordered_map>

class D2DTextbox;

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

class CTextCell:public CCell
{
public:
	CTextCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, CMenu* pMenu = nullptr)
		:CCell(pSheet, pRow, pColumn, spProperty, pMenu)
	{
		m_isWrappable = true;
	}
	virtual ~CTextCell();

	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override;
	virtual void PaintLine(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override;
	virtual void PaintBackground(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override;

	//virtual CSize MeasureSize(CDC* pDC);
	//virtual CSize MeasureSizeWithFixedWidth(CDC* pDC);
	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite* direct) override;
	virtual d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* direct) override;
	virtual d2dw::CRectF GetEditRect() const;
	virtual void OnEdit(const EventArgs& e);
	virtual void OnKillFocus(const KillFocusEvent& e) override;
	virtual bool CanSetStringOnEditing()const{return true;}

	//virtual UINT GetFormat()const
	//{
	//	if (GetRowSizingType() == SizingType::Depend && GetColSizingType() == SizingType::Depend) {
	//		return DT_LEFT | DT_TOP | DT_NOPREFIX | DT_EDITCONTROL;
	//	} else {
	//		return DT_LEFT | DT_TOP | DT_NOPREFIX | DT_WORDBREAK | DT_EDITCONTROL;
	//	}
	//}

	virtual bool IsComparable()const;
	virtual Compares EqualCell(CCell* pCell, std::function<void(CCell*, Compares)> action);
	virtual Compares EqualCell(CEmptyCell* pCell, std::function<void(CCell*, Compares)> action);
	virtual Compares EqualCell(CTextCell* pCell, std::function<void(CCell*, Compares)> action);
	virtual Compares EqualCell(CSheetCell* pCell, std::function<void(CCell*, Compares)> action);
};

class CStringCell:public CTextCell
{
protected:
	std::wstring m_string;
public:
	CStringCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty, std::wstring str,CMenu* pMenu=nullptr)
		:CTextCell(pSheet,pRow, pColumn,spProperty,pMenu),m_string(str){}
	virtual ~CStringCell(){}
	virtual std::wstring GetString();
	virtual void SetStringCore(const std::wstring& str);
};

class CEditableCell:public CTextCell
{
public:
	CEditableCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn,std::shared_ptr<CellProperty> spProperty,CMenu* pMenu=nullptr)
		:CTextCell(pSheet,pRow, pColumn,spProperty,pMenu){}
	virtual ~CEditableCell(){}
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
};

class CEditableStringCell:public CStringCell
{
public:
	CEditableStringCell(CSheet* pSheet,CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, std::wstring str,CMenu* pMenu=nullptr)
		:CStringCell(pSheet,pRow,pColumn,spProperty,str,pMenu){}
	virtual ~CEditableStringCell(){}
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
};

class CEditableNoWrapStringCell :public CEditableStringCell
{
public:
	CEditableNoWrapStringCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, std::wstring str, CMenu* pMenu = nullptr)
		:CEditableStringCell(pSheet, pRow, pColumn, spProperty, str, pMenu){}

	virtual ~CEditableNoWrapStringCell() {}
//	virtual UINT GetFormat()const override { return DT_LEFT | DT_TOP | DT_NOPREFIX | DT_EDITCONTROL; }
};

class CParameterCell:public CEditableCell
{
private:
	bool m_bFirstFocus;
public:
	CParameterCell(CSheet* pSheet=nullptr, CRow* pRow=nullptr, CColumn* pColumn=nullptr,std::shared_ptr<CellProperty> spProperty=nullptr,CMenu* pMenu=nullptr)
		:CEditableCell(pSheet,pRow, pColumn,spProperty,pMenu),m_bFirstFocus(false){}
	virtual ~CParameterCell(){}
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonSnglClk(const LButtonSnglClkEvent& e) override;
	virtual void OnKillFocus(const KillFocusEvent& e) override;

	virtual bool CanSetStringOnEditing()const override{return false;}

};

