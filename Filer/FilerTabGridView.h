#pragma once
#include "MyWnd.h"
#include "UniqueIDFactory.h"
#include "MyFriendSerializer.h"
#include "MyRect.h"
#include "FilerGridView.h"
#include "ShellFolder.h"

#include "observable.h"


class CFilerGridView;
struct GridViewProperty;
class CShellFolder;

class CFilerTabGridView :public CTabCtrl
{
private:
	CUniqueIDFactory m_uniqueIDFactory;
	observable_vector<std::wstring> m_vwPath;
	//observable_vector< std::tuple<std::shared_ptr<CShellFile>>> m_itemsSource;
	//std::vector< std::tuple<std::shared_ptr<CShellFile>>> m_selectedItems;

	std::shared_ptr<CFilerGridView> m_spFilerView;

	observable_vector<std::shared_ptr<CShellFolder>> m_folders;
	observable<int> m_selectedIndex = -1;


	CWnd* m_pParentWnd;

	CFont m_font;
	//unsigned int m_prevSelectedIndex;
	int m_contextMenuTabIndex;

public:
	CFilerTabGridView(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProrperty);
	virtual ~CFilerTabGridView() {}

	//Getter Setter
	void SetContextMenuTabIndex(int index) { m_contextMenuTabIndex = index; }
	std::shared_ptr<CFilerGridView> GetGridView() { return m_spFilerView; }
	void SetParentWnd(CWnd* pParentWnd) { m_pParentWnd = pParentWnd; }

	//Message
	LRESULT OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//LRESULT OnDestroy(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnCommandNewTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandCloneTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandCloseTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandCloseAllButThisTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandAddToFavorite(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandOpenSameAsOther(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnNotifyTabSelChanging(int id, LPNMHDR, BOOL& bHandled);
	LRESULT OnNotifyTabSelChange(int id, LPNMHDR, BOOL& bHandled);
	LRESULT OnNotifyTabLClick(int id, LPNMHDR, BOOL& bHandled);
	LRESULT OnNotifyTabRClick(int id, LPNMHDR, BOOL& bHandled);

private:
	//void AddNewTab(const std::wstring& path);
	//void AddNewTab(const std::shared_ptr<CShellFile>& spFile);
public:
	FRIEND_SERIALIZER
	template <class Archive>
	void save(Archive& ar)
	{
		m_vwPath.clear();
		std::transform(m_folders.begin(), m_folders.end(), std::back_inserter(m_vwPath),
			[](auto spFolder)->std::wstring {return spFolder->GetPath(); });
		ar("ViewPaths", m_vwPath);
		ar("SelectedIndex", m_selectedIndex);
		ar("FilerView", m_spFilerView);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		ar("ViewPaths", m_vwPath);
		ar("SelectedIndex", m_selectedIndex);
		ar("FilerView", m_spFilerView, m_spFilerView->GetFilerGridViewPropPtr());
	}


};
