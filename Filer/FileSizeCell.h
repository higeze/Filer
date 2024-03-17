#pragma once

#include "TextCell.h"
#include "FileSizeColumn.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "BindRow.h"
#include "CellProperty.h"
#include "GridView.h"
#include "GridView.h"
#include <fmt/format.h>
#include <sigslot/signal.hpp>


template<typename T>
class CFileSizeCell:public CTextCell
{
private:
	mutable sigslot::connection m_conDelayUpdateAction;

public:


	CFileSizeCell(CGridView* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
		:CTextCell(pSheet, pRow, pColumn, spProperty)
	{
	}

	virtual ~CFileSizeCell()
	{
		m_conDelayUpdateAction.disconnect();
	}


	virtual CSizeF MeasureContentSize(CDirect2DWrite* pDirect) override
	{
		//Calc Content Rect
	//	CRectF rcContent;
		std::basic_string<TCHAR> str;
		auto spFile = GetShellFile();
		auto size = spFile->ReadSize();
		switch (size.second) {
			case FileSizeStatus::Available:
				str = shell::Size2String(size.first.QuadPart);
				break;
			default:
				str = L"00,000,000";
				break;
		}
		if (str.empty()) { str = _T("a"); }
		return pDirect->CalcTextSize(*(m_spCellProperty->Format), str);
		//return rcContent.Size();
	}


	virtual std::wstring GetString() override
	{
		try {
			auto spFile = GetShellFile();
			std::weak_ptr<CFileSizeCell> wp(std::dynamic_pointer_cast<CFileSizeCell>(shared_from_this()));
			auto changed = [wp]()->void {
				if (auto sp = wp.lock()) {
					auto con = sp->GetGridPtr()->SignalPreDelayUpdate.connect(
						[wp]()->void {
							if (auto sp = wp.lock()) {
								sp->OnPropertyChanged(L"value");
							}
						});
					sp->m_conDelayUpdateAction = con;
					sp->GetGridPtr()->DelayUpdate();
				}
			};
			auto size = spFile->GetSize(static_cast<const CFileSizeColumn<T>*>(m_pColumn)->GetSizeArgsPtr(), changed);
			switch (size.second) {
				case FileSizeStatus::None:
					return L"none";
				case FileSizeStatus::Available:
					return shell::Size2String(size.first.QuadPart);
				case FileSizeStatus::Calculating:
					return L"...";
				case FileSizeStatus::Unavailable:
					return L"-";
				default:
					return L"";
			}
		}
		catch (...) {
			return L"err";
		}
	}

	virtual std::wstring GetSortString() override
	{
		try {
			auto spFile = GetShellFile();
			auto changed = [wp = std::weak_ptr(std::dynamic_pointer_cast<CFileSizeCell<T>>(shared_from_this()))]()->void {
				if (auto sp = wp.lock()) {
					sp->m_conDelayUpdateAction = sp->GetGridPtr()->SignalPreDelayUpdate.connect(
						[wp]()->void {
							if (auto sp = wp.lock()) {
								sp->OnPropertyChanged(L"value");
							}
						});
					sp->GetGridPtr()->DelayUpdate();
				}
			};
			auto size = spFile->GetSize(static_cast<const CFileSizeColumn<T>*>(m_pColumn)->GetSizeArgsPtr(), changed);
			switch (size.second) {
				case FileSizeStatus::None:
					return L"none";
				case FileSizeStatus::Available:
					return fmt::format(L"{:12d}", size.first.QuadPart);
				case FileSizeStatus::Calculating:
					return L"...";
				case FileSizeStatus::Unavailable:
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

