#include "FileLastWriteCell.h"
#include "FileLastWriteColumn.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "FileRow.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "GridView.h"

CFileLastWriteCell::CFileLastWriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
	:CTextCell(pSheet, pRow, pColumn, spProperty){}

CFileLastWriteCell::~CFileLastWriteCell()
{
	m_conDelayUpdateAction.disconnect();
	m_conLastWriteChanged.disconnect();
}

std::shared_ptr<CShellFile> CFileLastWriteCell::GetShellFile()
{
	if (auto pFileRow = dynamic_cast<CFileRow*>(m_pRow)) {
		if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(pFileRow->GetFilePointer())) {
			if (!m_conLastWriteChanged.connected()) {
				std::weak_ptr<CFileLastWriteCell> wp(shared_from_this());
				m_conLastWriteChanged = spFolder->SignalFileSizeChanged.connect(
					[wp](CShellFile* pFile)->void {
					if (auto sp = wp.lock()) {
						auto con = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
							[wp]()->void {
							if (auto sp = wp.lock()) {
								sp->OnPropertyChanged(L"value");
							}
						});
						sp->m_conDelayUpdateAction = con;
						sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
					}
				});
			}
			return spFolder;
		} else {
			return pFileRow->GetFilePointer();
		}
	} else {
		return nullptr;
	}
}

std::wstring CFileLastWriteCell::GetString()
{
	try {
		auto spFile = GetShellFile();
		auto time = spFile->GetLastWriteTime(static_cast<CFileLastWriteColumn*>(m_pColumn)->GetTimeArgsPtr());
		switch (time.second) {
		case FileTimeStatus::None:
			return L"-";
		case FileTimeStatus::Available:
		case FileTimeStatus::AvailableLoading:
			return FileTime2String(&(time.first));
		case FileTimeStatus::Loading:
			return L"...";
		case FileTimeStatus::Unavailable:
			return L"-";
		default:
			return L"";
		}
	} catch (...) {
		return L"err";
	}
}
