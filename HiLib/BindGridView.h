#pragma once
#include "GridView.h"
#include "BindRow.h"
#include "observable.h"
#include "MyString.h"
#include "MyXmlSerializer.h"

template<typename T>
class CBindGridView :public CGridView
{
private:
	observable_vector<T> m_itemsSource;
public:
	CBindGridView(std::shared_ptr<GridViewProperty>& spGridViewProp)
		:CGridView(spGridViewProp)
	{
		m_itemsSource.VectorChanged.connect(
			[this](const NotifyVectorChangedEventArgs<T>& e)->void {
				switch (e.Action) {
				case NotifyVectorChangedAction::Add:
					PushRow(std::make_shared<CBindRow<T>>(this));
					break;
				case NotifyVectorChangedAction::Remove:
					EraseRow(m_allRows.back());
					break;
				case NotifyVectorChangedAction::Reset:
					//TODOTODO
				default:
					break;
				}

			}
		);
	}

	

	virtual bool HasSheetCell()override { return true; }
	virtual bool IsVirtualPage()override { return true; }

	observable_vector<T>& GetItemsSource() { return m_itemsSource; }

	/******************/
	/* Window Message */
	/******************/

	/****************/
	/* StateMachine */
	/****************/
	void Normal_ContextMenu(const ContextMenuEvent& e) override
	{
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

		if(idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Add Row")){
			m_itemsSource.notify_push_back(T());
		} else if (idCmd == CResourceIDFactory::GetInstance()->GetID(ResourceType::Command, L"Remove Row")) {
			auto a = Cell(m_pDirect->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>();
			m_itemsSource.notify_erase(m_itemsSource.cbegin() + Cell(m_pDirect->Pixels2Dips(ptClient))->GetRowPtr()->GetIndex<AllTag>() - m_frozenRowCount);
		}

	}

	void Normal_KeyDown(const KeyDownEvent& e) override
	{
		switch (e.Char) {
		case 'O':
			if (::GetAsyncKeyState(VK_CONTROL)) {

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
					//Deserialize
					std::vector<T> itemsSource;
					try {
						//Serialize
						CXMLSerializer<std::vector<T>> serializer;
						serializer.Deserialize(path.c_str(), L"Task", itemsSource);
						//m_itemsSource.notify_clear();//TODOTODO
						while (!m_itemsSource.empty()) {
							m_itemsSource.notify_erase(m_itemsSource.cbegin());
						}
						for (const auto& item : itemsSource) {
							m_itemsSource.notify_push_back(item);
						}
					}
					catch (/*_com_error &e*/...) {
					}
				}
			}
			break;
		case 'S':
			if (::GetAsyncKeyState(VK_CONTROL)) {

				std::wstring path;
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
					//Serialize
					try {
						std::vector<T> itemsSource;
						std::copy(m_itemsSource.cbegin(), m_itemsSource.cend(), std::back_inserter(itemsSource));
						CXMLSerializer<std::vector<T>> serializer;
						serializer.Serialize(path.c_str(), L"Task", itemsSource);
					}
					catch (/*_com_error &e*/...) {
					}
				}
			}
			break;
		default:
			break;
		}
	}

};