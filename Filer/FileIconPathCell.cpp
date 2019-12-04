#include "FileIconPathCell.h"
#include "ShellFile.h"
#include "FileRow.h"
#include "GridView.h"
#include "CellProperty.h"
#include "FileIconCache.h"

CFileIconPathCell::CFileIconPathCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CFileIconNameCell(pSheet, pRow, pColumn, spProperty)
{
}

std::wstring CFileIconPathCell::GetString()
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer()->GetPath();
}
