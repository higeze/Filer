#pragma once

#include "TextCell.h"
class CShellFile;

class CFileSizeCell:public CTextCell, public std::enable_shared_from_this<CFileSizeCell>
{
private:
	mutable boost::signals2::connection m_conDelayUpdateAction;
	mutable boost::signals2::connection m_conFileSizeChanged;

public:
	CFileSizeCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty);
	virtual ~CFileSizeCell(void);

	virtual bool IsComparable()const override{return false;}
	virtual CSize MeasureContentSize(CDC* pDC) override;
	virtual string_type GetString() override;
	virtual string_type GetSortString() override;
	virtual UINT GetFormat()const override{ return DT_RIGHT|DT_TOP|DT_NOPREFIX|DT_WORDBREAK|DT_EDITCONTROL; }

	//virtual void SetDelayUpdateConnection(boost::signals2::connection con)const { m_conDelayUpdateAction = con; }
	
private:
	virtual std::shared_ptr<CShellFile> GetShellFile() ;
};

