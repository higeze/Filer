#include "FileLastWriteCell.h"
#include "ShellFile.h"
#include "FileRow.h"

CFileLastWriteCell::CFileLastWriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty)
	:CTextCell(pSheet, pRow, pColumn, spProperty){}

CFileLastWriteCell::string_type CFileLastWriteCell::GetString()const
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer()->GetLastWriteTime();
}
