#pragma once
#include "MyWnd.h"
#include "UniqueIDFactory.h"
#include "MyFriendSerializer.h"
#include "MyRect.h"
#include "FilerGridView.h"
#include "ShellFolder.h"

class CFilerGridView;
class CGridViewProperty;
class CShellFolder;

class CFilerTabGridView :public CTabCtrl
{
private:
	CUniqueIDFactory m_uniqueIDFactory;
	std::vector<std::wstring> m_vwPath;
	std::map<unsigned int, std::shared_ptr<CShellFolder>> m_viewMap;
	std::shared_ptr<CFilerGridView> m_spFilerView;

	//std::shared_ptr<CGridViewProperty> m_spGridViewProp;
	CWnd* m_pParentWnd;

	CFont m_font;
	unsigned int m_prevID;
	int m_contextMenuTabIndex;

public:
	CFilerTabGridView(std::shared_ptr<CGridViewProperty> spGridViewProp, std::shared_ptr<FilerGridViewProperty> spFilerGridViewProrperty);
	virtual ~CFilerTabGridView() {}

	//Getter Setter
	void SetContextMenuTabIndex(int index) { m_contextMenuTabIndex = index; }
	std::shared_ptr<CFilerGridView> GetGridView() { return m_spFilerView; }
	//void SetGridViewProp(std::shared_ptr<CGridViewProperty> prop) { m_spGridViewProp = prop;}
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
	void AddNewView(std::wstring path);
public:
	FRIEND_SERIALIZER
	template <class Archive>
	void save(Archive& ar)
	{
		m_vwPath.clear();
		for (auto pair : m_viewMap) {
			m_vwPath.push_back(pair.second->GetPath());
		}
		ar("ViewPaths", m_vwPath);
		ar("FilerView", m_spFilerView);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		ar("ViewPaths", m_vwPath);
		ar("FilerView", m_spFilerView, m_spFilerView->GetGridViewPropPtr(), m_spFilerView->GetFilerGridViewPropPtr());
	}


};
