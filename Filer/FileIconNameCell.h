#pragma once
#include "TextCell.h"

class CShellFile;

class CFileIconNameCell:public CParameterCell, public std::enable_shared_from_this<CFileIconNameCell>
{
protected:
	mutable boost::signals2::connection m_conDelayUpdateAction;
public:
	CFileIconNameCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty);
	virtual ~CFileIconNameCell(){}

	virtual bool IsComparable()const override{return false;}
	virtual std::wstring GetString() override;
	void SetStringCore(const std::wstring& str) override;
	virtual std::shared_ptr<CShellFile> GetShellFile();
	virtual void PaintContent(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint)override;
	virtual d2dw::CSizeF MeasureContentSize(d2dw::CDirect2DWrite* pDirect) override;
	virtual d2dw::CSizeF MeasureContentSizeWithFixedWidth(d2dw::CDirect2DWrite* pDirect) override;
	virtual d2dw::CRectF GetEditRect() const override;
	virtual void OnEdit(const EventArgs& e) override;
	virtual void PaintBackground(d2dw::CDirect2DWrite* pDirect, d2dw::CRectF rcPaint) override;

protected:
	d2dw::CSizeF GetIconSizeF(d2dw::CDirect2DWrite* pDirect)const;
	CSize GetIconSize(d2dw::CDirect2DWrite* pDirect)const;


};