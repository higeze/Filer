#pragma once
#include "TextCell.h"
#include "BindRow.h"
#include "FileLastWriteColumn.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "CellProperty.h"
#include <sigslot/signal.hpp>

template<typename T>
class CFileLastWriteCell:public CTextCell
{
private:
	mutable sigslot::connection m_conDelayUpdateAction;
	mutable sigslot::connection m_conLastWriteChanged;

public:
	CFileLastWriteCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty)
	{
	}

	virtual ~CFileLastWriteCell()
	{
		m_conDelayUpdateAction.disconnect();
		m_conLastWriteChanged.disconnect();
	}

	virtual std::wstring GetString() override
	{
		try {
			auto spFile = GetShellFile();
			std::weak_ptr<CFileLastWriteCell<T>> wp(std::dynamic_pointer_cast<CFileLastWriteCell<T>>(shared_from_this()));
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

			auto times = spFile->GetFileTimes((static_cast<const CFileLastWriteColumn<T>*>(this->m_pColumn))->GetTimeArgsPtr(), changed);
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


private:
	virtual std::shared_ptr<CShellFile> GetShellFile()
	{
		if (auto p = dynamic_cast<CBindRow<T>*>(m_pRow)) {
			return p->GetItem<std::shared_ptr<CShellFile>>();
		} else {
			return nullptr;
		}
	}
};

