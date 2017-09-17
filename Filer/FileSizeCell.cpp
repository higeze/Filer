#include "FileSizeCell.h"
#include "ShellFile.h"
#include "FileRow.h"

CFileSizeCell::CFileSizeCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty)
	:CTextCell(pSheet, pRow, pColumn, spProperty){}

bool CFileSizeCell::operator<(const CCell& rhs)const
{
	auto pLhsRow = static_cast<CFileRow*>(m_pRow);
	auto pRhsRow = static_cast<CFileRow*>(rhs.GetRowPtr());
	return pLhsRow->GetFilePointer()->GetSize().QuadPart < pRhsRow->GetFilePointer()->GetSize().QuadPart;
}

bool CFileSizeCell::operator>(const CCell& rhs)const
{
	auto pLhsRow = static_cast<CFileRow*>(m_pRow);
	auto pRhsRow = static_cast<CFileRow*>(rhs.GetRowPtr());
	return pLhsRow->GetFilePointer()->GetSize().QuadPart > pRhsRow->GetFilePointer()->GetSize().QuadPart;
}

CCell::string_type CFileSizeCell::GetString()const
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer()->GetSizeString();
}

CCell::string_type CFileSizeCell::GetSortString()const
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return (boost::wformat(L"%12d") % pFileRow->GetFilePointer()->GetSize().QuadPart).str();
}