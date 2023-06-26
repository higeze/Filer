#include "ToDoGridView.h"
#include "ResourceIDFactory.h"
#include "D2DWWindow.h"

void CToDoGridView::Initialize()
{
	//Columns
	if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CRowIndexColumn); })) 
	{ 
		m_pHeaderColumn = std::make_shared<CRowIndexColumn>(this);
		m_allCols.idx_push_back(m_pHeaderColumn);
	}
	if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CTaskCheckBoxColumn); })) 
	{ 
		m_allCols.idx_push_back(std::make_shared<CTaskCheckBoxColumn>(this));
	}
	if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CTaskNameColumn); })) 
	{ 
		m_pNameColumn = std::make_shared<CTaskNameColumn>(this);
		m_allCols.idx_push_back(m_pNameColumn);
	}
	if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CTaskMemoColumn); })) 
	{ 
		m_allCols.idx_push_back(std::make_shared<CTaskMemoColumn>(this));
	}
	if (!std::any_of(m_allCols.cbegin(), m_allCols.cend(), [](const std::shared_ptr<CColumn>& pCol) { return typeid(*pCol) == typeid(CTaskDueDateColumn); })) 
	{ 
		m_allCols.idx_push_back(std::make_shared<CTaskDueDateColumn>(this));
	}

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
	if (Path.get().empty()) {
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
			Path.set(path);
		}
	}

	if(!Path.get().empty()){
		Save(Path.get());
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
