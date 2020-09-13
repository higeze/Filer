#pragma once

#include "TextCell.h"
#include "FileSizeColumn.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "BindRow.h"
#include "CellProperty.h"
#include "Sheet.h"
#include "GridView.h"
#include <fmt/format.h>

template<typename... TItems>
class CFileSizeCell:public CTextCell, public std::enable_shared_from_this<CFileSizeCell<TItems...>>
{
private:
	mutable boost::signals2::connection m_conDelayUpdateAction;

public:


	CFileSizeCell(CSheet* pSheet, CRow* pRow, CColumn* pColumn, std::shared_ptr<CellProperty> spProperty)
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
			std::weak_ptr<CFileSizeCell> wp(shared_from_this());
			auto changed = [wp]()->void {
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
			};
			auto size = spFile->GetSize(static_cast<CFileSizeColumn<TItems...>*>(m_pColumn)->GetSizeArgsPtr(), changed);
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
			std::weak_ptr<CFileSizeCell<TItems...>> wp(shared_from_this());
			auto changed = [wp]()->void {
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
			};
			auto size = spFile->GetSize(static_cast<CFileSizeColumn<TItems...>*>(m_pColumn)->GetSizeArgsPtr(), changed);
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
		if (auto pBindRow = dynamic_cast<CBindRow<TItems...>*>(m_pRow)) {
			return std::get<std::shared_ptr<CShellFile>>(pBindRow->GetTupleItems());
		} else {
			return nullptr;
		}
	}
};

