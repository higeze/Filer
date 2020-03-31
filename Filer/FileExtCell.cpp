#include "FileExtCell.h"
#include "ShellFile.h"
#include "FileRow.h"
#include "GridView.h"

CFileExtCell::CFileExtCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CParameterCell(pSheet, pRow, pColumn, spProperty){}

std::wstring CFileExtCell::GetString()
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer()->GetExt();
}

void CFileExtCell::SetStringCore(const std::wstring& str)
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	pFileRow->GetFilePointer()->SetExt(str, m_pSheet->GetGridPtr()->m_hWnd);
}
