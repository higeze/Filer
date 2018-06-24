#pragma once

#include "TextCell.h"
class CShellFile;

class CFileSizeCell:public CTextCell
{
private:
	std::function<void()> m_delayUpdateAction;
	//std::wstring m_size;
	//std::wstring m_sortSize;
public:
	CFileSizeCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty);
	virtual ~CFileSizeCell(void);

	//bool operator<(const CFileSizeCell& rhs)const;
	//bool operator>(const CFileSizeCell& rhs)const;

	virtual bool IsComparable()const override{return false;}
	virtual string_type GetString()const override;
	virtual string_type GetSortString()const override;
	virtual UINT GetFormat()const override{ return DT_RIGHT|DT_TOP|DT_NOPREFIX|DT_WORDBREAK|DT_EDITCONTROL; }
private:
	virtual std::shared_ptr<CShellFile> GetShellFile()const;
};

