#pragma once
#include "TextCell.h"

class IInplaceEdit;
/**
 *  Filter Cell
 */
class CFilterCell:public CEditableCell
{
private :
	//static std::unique_ptr<boost::asio::io_service> m_pFilterIosv;
	//static std::unique_ptr<boost::asio::io_service::work> m_pFilterWork;
	//static std::unique_ptr<boost::asio::deadline_timer> m_pFilterTimer;

public:
	/**
	 *  Constructor
	 */
	CFilterCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, CMenu* pMenu = nullptr);
	/**
	 *  Destructor
	 */
	virtual ~CFilterCell();
	/**
	 *  Get string from column property
	 */
	virtual std::wstring GetString() override;
	/**
	 *  Set string to column property
	 */
	virtual void SetString(const std::wstring& str)override;
	virtual void SetStringCore(const std::wstring& str)override;
	/**
	 *  Override to show message when cell is empty
	 */
	virtual void PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)override;
	/**
	 *  Comparable or not
	 */
	virtual bool IsComparable()const override
	{
		return false;
	}

};