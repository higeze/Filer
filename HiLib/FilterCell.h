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
	CFilterCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty, CMenu* pMenu = nullptr);
	virtual ~CFilterCell();

	virtual std::wstring GetString() override;
	virtual void SetString(const std::wstring& str)override;
	virtual void SetStringCore(const std::wstring& str)override;
	virtual void PaintContent(d2dw::CDirect2DWrite& direct, d2dw::CRectF rcPaint)override;
	virtual bool IsComparable()const override
	{
		return false;
	}

	virtual void OnPropertyChanged(const wchar_t* name) override;


};