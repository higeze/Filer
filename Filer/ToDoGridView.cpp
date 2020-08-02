#include "ToDoGridView.h"
#include "ResourceIDFactory.h"

void CToDoGridView::Open()
{
	std::wstring path;
	OPENFILENAME ofn = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = m_hWnd;
	ofn.lpstrFilter = L"XML file(*.xml)\0*.xml\0\0";
	ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrTitle = L"Open";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.lpstrDefExt = L"xml";

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
	if (::PathFileExists(path.c_str())){
		m_path.notify_set(path);
		auto& itemsSource = GetItemsSource();
		//itemsSource.notify_clear();//TODOTODO
		while (!itemsSource.empty()) {
			itemsSource.notify_erase(GetItemsSource().cbegin());
		}
		//Deserialize
		try {
			//Serialize
			std::vector<MainTask> tempItemsSource;
			CXMLSerializer<std::vector<MainTask>> serializer;
				serializer.Deserialize(m_path.get().c_str(), L"Task", tempItemsSource);
			for (const auto& item : tempItemsSource) {
				itemsSource.notify_push_back(std::make_tuple(item));
			}
			for (auto& colPtr : m_allCols) {
				colPtr->SetIsFitMeasureValid(false);
				colPtr->SetIsMeasureValid(false);
			}
			UpdateAll();
		}
		catch (/*_com_error &e*/...) {
		}
	}
}


void CToDoGridView::Save()
{
	std::wstring path;
	if (m_path.get().empty()) {
		OPENFILENAME ofn = { 0 };
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = m_hWnd;
		ofn.lpstrFilter = L"XML file(*.xml)\0*.xml\0\0";
		ofn.lpstrFile = ::GetBuffer(path, MAX_PATH);
		ofn.nMaxFile = MAX_PATH;
		ofn.lpstrTitle = L"Save as";
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
		ofn.lpstrDefExt = L"xml";

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
	m_path.notify_set(path);
	//Serialize
	try {
		auto itemsSource = GetItemsSource();
		std::vector<MainTask> tempItemsSource;
		for (const auto& item : itemsSource) {
			tempItemsSource.push_back(std::get<MainTask>(item));
		}
		CXMLSerializer<std::vector<MainTask>> serializer;
		serializer.Serialize(m_path.get().c_str(), L"Task", tempItemsSource);
	}
	catch (/*_com_error &e*/...) {
	}
}



void CToDoGridView::Normal_ContextMenu(const ContextMenuEvent& e)
{
	auto spCell = Cell(m_pDirect->Pixels2Dips(e.Point));
	if (spCell) {
		spCell->OnContextMenu(e);
		if (e.Handled) { return; }
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
	mii.dwTypeData = L"Add Row";
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	mii.wID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Remove Row");
	mii.dwTypeData = L"Remove Row";
	menu.InsertMenuItem(menu.GetMenuItemCount(), TRUE, &mii);

	CPoint ptClient(e.Point);
	CPoint ptScreen(e.Point);
	ClientToScreen(ptScreen);
	::SetForegroundWindow(m_hWnd);
	int idCmd = menu.TrackPopupMenu(
		TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
		ptScreen.x,
		ptScreen.y,
		m_hWnd);

	auto& itemsSource = GetItemsSource();
	if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Add Row")) {
		itemsSource.notify_push_back(MainTask());
	} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Remove Row")) {
		auto a = Cell(m_pDirect->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>();
		itemsSource.notify_erase(itemsSource.cbegin() + (Cell(m_pDirect->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>() - m_frozenRowCount));
	}
	e.Handled = TRUE;
}

void CToDoGridView::Normal_KeyDown(const KeyDownEvent& e)
{
	if ((e.Char == 'O') && ::GetAsyncKeyState(VK_CONTROL)) {
		Open();
	} else if ((e.Char == 'S') && ::GetAsyncKeyState(VK_CONTROL)) {
		if (m_path.get().empty()) {
			Save();
		} else {
			Save(m_path);
		}
	} else {
		CGridView::Normal_KeyDown(e);
	}
	InvalidateRect(NULL, FALSE);
}
