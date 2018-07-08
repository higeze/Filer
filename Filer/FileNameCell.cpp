#include "FileNameCell.h"
#include "ShellFile.h"
#include "FileRow.h"
#include "GridView.h"

CFileNameCell::CFileNameCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CCellProperty> spProperty)
	:CParameterCell(pSheet, pRow, pColumn, spProperty){}

std::basic_string<TCHAR> CFileNameCell::GetString()
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	return pFileRow->GetFilePointer()->GetName();
}

void CFileNameCell::SetStringCore(const std::basic_string<TCHAR>& str)
{
	auto pFileRow = static_cast<CFileRow*>(m_pRow);
	std::shared_ptr<CShellFile> spFile = pFileRow->GetFilePointer();

	CIDLPtr pIdlNew;
	HRESULT hRes = spFile->GetParentShellFolderPtr()->SetNameOf(
		m_pSheet->GetGridPtr()->m_hWnd,
		spFile->GetAbsolutePidl().FindLastID(),
		str.c_str(),
		SHGDN_NORMAL | SHGDN_INFOLDER,
		&pIdlNew );
	if(SUCCEEDED(hRes)){
		pFileRow->SetFilePointer(std::make_shared<CShellFile>(spFile->GetParentShellFolderPtr(), spFile->GetAbsolutePidl().GetPreviousIDLPtr() + pIdlNew));
	}
}