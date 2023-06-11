#include "ToDoGridView.h"
#include "ResourceIDFactory.h"
#include "D2DWWindow.h"

#include "RowIndexColumn.h"
#include "BindTextColumn.h"
#include "BindTextCell.h"
#include "BindCheckBoxColumn.h"
#include "TaskDateColumn.h"


void CToDoGridView::Initialize()
{
	//Columns
	SetHeaderColumnPtr(std::make_shared<CRowIndexColumn>(this));
	PushColumns(
		GetHeaderColumnPtr(),
		std::make_shared<CBindCheckBoxColumn<MainTask>>(
			this,
			L"State",
			[](const std::tuple<MainTask>& tk)->CheckBoxState {return std::get<MainTask>(tk).State.get(); },
			[](std::tuple<MainTask>& tk, const CheckBoxState& state)->void {std::get<MainTask>(tk).State = state; }),
		std::make_shared<CBindTextColumn<MainTask>>(
			this,
			L"Name",
			[](const std::tuple<MainTask>& tk)->std::wstring {return std::get<MainTask>(tk).Name.get(); },
			[](std::tuple<MainTask>& tk, const std::wstring& str)->void {std::get<MainTask>(tk).Name = str; }),
		std::make_shared<CBindTextColumn<MainTask>>(
			this,
			L"Memo",
			[](const std::tuple<MainTask>& tk)->std::wstring {return std::get<MainTask>(tk).Memo.get(); },
			[](std::tuple<MainTask>& tk, const std::wstring& str)->void {std::get<MainTask>(tk).Memo = str; }),
		std::make_shared<CDateColumn>(
			this,
			L"Due date")//,
		//std::make_shared<CBindSheetCellColumn< MainTask, SubTask>>(
		//	m_spToDoGridView.get(),
		//	L"Sub Task",
		//	[](std::tuple<MainTask>& tk)->ReactiveVectorProperty<std::tuple<SubTask>>& {return std::get<MainTask>(tk).SubTasks; },
		//	[](CBindItemsSheetCell<MainTask, SubTask>* pCell)->void {
		//		pCell->SetHeaderColumnPtr(std::make_shared<CRowIndexColumn>(pCell));
		//		pCell->PushColumns(
		//			pCell->GetHeaderColumnPtr(),
		//			std::make_shared<CBindCheckBoxColumn<SubTask>>(
		//				pCell,
		//				L"Done",
		//				[](const std::tuple<SubTask>& tk)->CheckBoxState {return std::get<SubTask>(tk).Done ? CheckBoxState::True : CheckBoxState::False; },
		//				[](std::tuple<SubTask>& tk, const CheckBoxState& state)->void {std::get<SubTask>(tk).Done = state == CheckBoxState::True ? true : false; }),
		//			std::make_shared<CBindTextColumn<SubTask>>(
		//				pCell,
		//				L"Name",
		//				[](const std::tuple<SubTask>& tk)->std::wstring {return std::get<SubTask>(tk).Name; },
		//				[](std::tuple<SubTask>& tk, const std::wstring& str)->void {std::get<SubTask>(tk).Name = str; }),
		//			std::make_shared<CBindTextColumn<SubTask>>(
		//				pCell,
		//				L"Memo",
		//				[](const std::tuple<SubTask>& tk)->std::wstring {return std::get<SubTask>(tk).Memo; },
		//				[](std::tuple<SubTask>& tk, const std::wstring& str)->void {std::get<SubTask>(tk).Memo = str; })
		//		);
		//		pCell->SetFrozenCount<ColTag>(1);

		//		pCell->SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(pCell));
		//		pCell->InsertRow(0, pCell->GetNameHeaderRowPtr());
		//		pCell->SetFrozenCount<RowTag>(1);
		//	},
		//	arg<"maxwidth"_s>() = FLT_MAX)
	);
	SetFrozenCount<ColTag>(1);

	//Rows
	SetNameHeaderRowPtr(std::make_shared<CHeaderRow>(this));
	SetFilterRowPtr(std::make_shared<CRow>(this));

	PushRows(
		GetNameHeaderRowPtr(),
		GetFilterRowPtr());

	SetFrozenCount<RowTag>(2);

}

void CToDoGridView::Open()
{
	std::wstring path;
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetWndPtr()->m_hWnd;
	ofn.lpstrFilter = L"JSON file(*.json)\0*.json\0\0";
	ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Open";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = L"json";

	if (!GetOpenFileName(&ofn)) {
		DWORD errCode = CommDlgExtendedError();
		if (errCode) {
			//wsprintf(szErrMsg, L"Error code : %d", errCode);
			//MessageBox(NULL, szErrMsg, L"GetOpenFileName", MB_OK);
		}
	} else {
		::ReleaseBuffer(path);
		Open(path);
	}
}

void CToDoGridView::Open(const std::wstring& path)
{
	OpenCommand.execute(path);
	//if (::PathFileExists(path.c_str())){
	//	m_path.set(path);
	//	auto& itemsSource = GetItemsSource();
	//	//itemsSource.notify_clear();//TODOTODO
	//	while (!itemsSource.empty()) {
	//		itemsSource.erase(GetItemsSource().cbegin());
	//	}
	//	//Deserialize
	//	try {
	//		//Serialize
	//		std::vector<MainTask> tempItemsSource;
	//		std::ifstream i(path);
	//		json j;
	//		i >> j;
	//		j.get_to(tempItemsSource);
	//		//CXMLSerializer<std::vector<MainTask>> serializer;
	//		//	serializer.Deserialize(m_path.get().c_str(), L"Task", tempItemsSource);
	//		for (const auto& item : tempItemsSource) {
	//			itemsSource.push_back(std::make_tuple(item));
	//		}
	//		for (auto& colPtr : m_allCols) {
	//			colPtr->SetIsFitMeasureValid(false);
	//			colPtr->SetIsMeasureValid(false);
	//		}
	//		UpdateAll();
	//	}
	//	catch (/*_com_error &e*/...) {
	//	}
	//}
}


void CToDoGridView::Save()
{
	std::wstring path;
	if (Status == FileStatus::None) {
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = GetWndPtr()->m_hWnd;
		ofn.lpstrFilter = L"JSON file(*.json)\0*.json\0\0";
		ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = L"Save as";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt = L"json";

		if (!GetSaveFileName(&ofn)) {
			DWORD errCode = CommDlgExtendedError();
			if (errCode) {
				//wsprintf(szErrMsg, L"Error code : %d", errCode);
				//MessageBox(NULL, szErrMsg, L"GetOpenFileName", MB_OK);
			}
		} else {
			::ReleaseBuffer(path);
		}
	}
	//Serialize
	try {
		Save(path);
	}
	catch (/*_com_error &e*/...) {
	}

}

void CToDoGridView::Save(const std::wstring& path)
{
	SaveCommand.execute(path);
}



void CToDoGridView::Normal_ContextMenu(const ContextMenuEvent& e)
{
	auto spCell = Cell(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient));
	if (spCell) {
		spCell->OnContextMenu(e);
		if (*e.HandledPtr) { return; }
	}

	//CreateMenu
	CMenu menu(::CreatePopupMenu());
	//Add Row
	MENUITEMINFO mii = { 0 };
	mii.cbSize = sizeof(MENUITEMINFO);
	mii.fMask = MIIM_TYPE | MIIM_ID;
	mii.fType = MFT_STRING;
	mii.fState = MFS_ENABLED;
	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Add Row");
	mii.dwTypeData = const_cast<LPWSTR>(L"Add Row");
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Remove Row");
	mii.dwTypeData = const_cast<LPWSTR>(L"Remove Row");
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	::SetForegroundWindow(GetWndPtr()->m_hWnd);
	int idCmd = menu.TrackPopupMenu(
		TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
		e.PointInScreen.x,
		e.PointInScreen.y,
		GetWndPtr()->m_hWnd);

	if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Add Row")) {
		ItemsSource.push_back(std::make_tuple(MainTask()));
	} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Remove Row")) {
		auto a = Cell(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient))->GetRowPtr()->GetIndex<AllTag>();
		ItemsSource.erase(ItemsSource.cbegin() + (Cell(GetWndPtr()->GetDirectPtr()->Pixels2Dips(e.PointInClient))->GetRowPtr()->GetIndex<AllTag>() - m_frozenRowCount));
	}
	*e.HandledPtr = TRUE;
}

void CToDoGridView::Normal_KeyDown(const KeyDownEvent& e)
{
	if ((e.Char == 'O') && ::GetAsyncKeyState(VK_CONTROL)) {
		Open();
	} else if ((e.Char == 'S') && ::GetAsyncKeyState(VK_CONTROL)) {
		Save();
	} else {
		CGridView::Normal_KeyDown(e);
	}
	//InvalidateRect(NULL, FALSE);
}
