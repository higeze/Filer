#include "FileLastWriteCell.h"
#include "ShellFile.h"
#include "FileRow.h"

CFileLastWriteCell::CFileLastWriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CTextCell(pSheet, pRow, pColumn, spProperty){}

std::wstring CFileLastWriteCell::GetString()
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer()->GetLastWriteTime();
}
