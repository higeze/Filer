#include "FileExtCell.h"
#include "ShellFile.h"
#include "FileRow.h"

CFileExtCell::CFileExtCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty)
	:CTextCell(pSheet, pRow, pColumn, spProperty){}

CFileExtCell::string_type CFileExtCell::GetString()const
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer()->GetExt();
}
