#pragma once

class CRow;
class CColumn;

class CRowColumn
{
private:
	static const int invalid_dataindex=-100;
	std::shared_ptr<CRow> m_pRow;
	std::shared_ptr<CColumn> m_pColumn;
public:
	CRowColumn(CRow* pRow=nullptr,CColumn* pColumn=nullptr):m_pRow(pRow),m_pColumn(pColumn){}
	virtual ~CRowColumn(){}
	std::shared_ptr<CRow> GetRowPtr()const{return m_pRow;}
	std::shared_ptr<CColumn> GetColumnPtr()const{return m_pColumn;}
	void SetColumnPtr(const std::shared_ptr<CColumn>& pColumn){m_pColumn=pColumn;}
	void SetRowPtr(const std::shared_ptr<CRow> pRow){m_pRow=pRow;}
	void SetRowColumnPtr(const std::shared_ptr<CRow>& pRow, std::shared_ptr<CColumn>& pColumn){m_pRow=pRow;m_pColumn=pColumn;}

	bool operator==(const CRowColumn& rhs)const
	{
		return (m_pRow==rhs.m_pRow && m_pColumn==rhs.m_pColumn);
	}

	bool operator!=(const CRowColumn& rhs)const
	{
		return (m_pRow!=rhs.m_pRow || m_pColumn!=rhs.m_pColumn);
	}

	CRowColumn& operator=(const CRowColumn& rhs)
	{
		m_pRow=rhs.m_pRow;m_pColumn=rhs.m_pColumn;
		return *this;
	}

	bool operator<(const CRowColumn& rhs)const;

	bool IsInvalid()const{return (m_pRow==nullptr || m_pColumn==nullptr);}

};