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
	pFileRow->GetFilePointer()->SetExt(str);

	//CIDLPtr pIdlNew;
	//HRESULT hRes = spFile->GetParentShellFolderPtr()->SetNameOf(
	//	m_pSheet->GetGridPtr()->m_hWnd,
	//	spFile->GetAbsolutePidl().FindLastID(),
	//	(spFile->GetFileNameWithoutExt() + str).c_str(),
	//	SHGDN_NORMAL | SHGDN_INFOLDER | SHGDN_FORPARSING,
	//	&pIdlNew);
	//if (SUCCEEDED(hRes)) {
	//	pFileRow->SetFilePointer(std::make_shared<CShellFile>(spFile->GetParentShellFolderPtr(), spFile->GetAbsolutePidl().GetPreviousIDLPtr() + pIdlNew));
	//}
}
