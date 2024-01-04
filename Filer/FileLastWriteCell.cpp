#include "FileLastWriteCell.h"
#include "BindRow.h"
#include "FileLastWriteColumn.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "GridView.h"


std::wstring CFileLastWriteCell::GetString()
{
	try {
		auto spFile = GetShellFile();
		std::weak_ptr<CFileLastWriteCell> wp(std::dynamic_pointer_cast<CFileLastWriteCell>(shared_from_this()));
		auto changed = [wp]()->void {
			if (auto sp = wp.lock()) {
				sp->m_conDelayUpdateAction = sp->GetSheetPtr()->GetGridPtr()->SignalPreDelayUpdate.connect(
					[wp]()->void {
						if (auto sp = wp.lock()) {
							sp->OnPropertyChanged(L"value");
						}
					});
				sp->GetSheetPtr()->GetGridPtr()->DelayUpdate();
			}
		};

		auto times = spFile->GetFileTimes((static_cast<const CFileLastWriteColumn*>(this->m_pColumn))->GetTimeArgsPtr(), changed);
		switch (times.second) {
			case FileTimeStatus::None:
				return L"-";
			case FileTimeStatus::Available:
			case FileTimeStatus::AvailableLoading:
				if (!times.first.LastWriteTime.dwLowDateTime && !times.first.LastWriteTime.dwHighDateTime) {
					return L"-";
				} else {
					return shell::FileTime2String(&(times.first.LastWriteTime));
				}
			case FileTimeStatus::Loading:
				return L"...";
			case FileTimeStatus::Unavailable:
				return L"-";
			default:
				return L"";
		}
	}
	catch (...) {
		return L"err";
	}
}
std::shared_ptr<CShellFile> CFileLastWriteCell::GetShellFile()
{
	if (auto pBindRow = dynamic_cast<CBindRow*>(m_pRow)) {
		return pBindRow->GetItem<std::shared_ptr<CShellFile>>();
	} else {
		return nullptr;
	}
}
