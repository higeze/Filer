#pragma once
#include "MyWnd.h"
#include "UniqueIDFactory.h"
#include "MyFriendSerializer.h"
#include "MyRect.h"
#include "FilerTabGridView.h"
#include "FavoritesProperty.h"
#include "ShellFolder.h"
#include "ApplicationProperty.h"

class CFilerGridView;
class CGridViewProperty;
//class CFavoritesProperty;
class CFavoritesGridView;
class CShellFolder;
//class CFilerTabGridView;

class CFilerWnd:public CWnd
{
//public:
//	static CUniqueIDFactory ControlIDFactory;
private:
	CRect m_rcWnd;
	std::shared_ptr<CFilerTabGridView> m_spLeftView;
	std::shared_ptr<CFilerTabGridView> m_spRightView;
	std::shared_ptr<CFilerTabGridView> m_spCurView;

	std::shared_ptr<CFavoritesGridView> m_spFavoritesView;

	//std::vector<std::wstring> m_vwPath;
	//std::map<unsigned int, std::shared_ptr<CShellFolder>> m_viewMap;
	//std::shared_ptr<CTabCtrl> m_spTab1;
	//std::shared_ptr<CTabCtrl> m_spTab2;
	//std::shared_ptr<CFilerGridView> m_spFilerView1;
	//std::shared_ptr<CFilerGridView> m_spFilerView2;

	//unsigned int m_prevID;
	//int m_contextMenuTabIndex;

	std::shared_ptr<CGridViewProperty> m_spGridViewProp;
	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;
	std::shared_ptr<CApplicationProperty> m_spApplicationProp;


public:
	CFilerWnd();
	//CFilerWnd(const CFilerWnd&) = delete;
	//CFilerWnd(CFilerWnd&&) = default;
	//CFilerWnd& operator=(const CFilerWnd&) = delete;
	//CFilerWnd& operator=(CFilerWnd&&) = default;
	virtual ~CFilerWnd();
	virtual HWND Create(HWND hWndParent);
	std::shared_ptr<CApplicationProperty> GetApplicationProperty() { return m_spApplicationProp; }
private:
	std::shared_ptr<CShellFolder> GetShellFolderFromPath(const std::wstring& path);
	//LRESULT OnTabContextMenu(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	//void SetContextMenuTabIndex(int index){m_contextMenuTabIndex = index;}

	LRESULT OnCreate(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnClose(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnDestroy(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	//LRESULT OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);

	//LRESULT OnCommandNewTab(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	//LRESULT OnCommandCloneTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//LRESULT OnCommandCloseTab(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	//LRESULT OnCommandCloseAllButThisTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//LRESULT OnCommandAddToFavorite(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	//LRESULT OnNotifyTabSelChanging(int id, LPNMHDR, BOOL& bHandled);
	//LRESULT OnNotifyTabSelChange(int id, LPNMHDR, BOOL& bHandled);
	//LRESULT OnNotifyTabRClick(int id, LPNMHDR, BOOL& bHandled);
	LRESULT OnCommandApplicationOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandGridViewOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	LRESULT OnCommandFavoritesOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	//void AddNewView(std::wstring path);
public:
	FRIEND_SERIALIZER
    template <class Archive>
    void save(Archive& ar)
    {
        const type_info& info = typeid(ar);
		if(info == typeid(CSerializer&) || info == typeid(CDeserializer&)){ 
			ar("WindowRectangle", m_rcWnd);
		}
		ar("GridViewProperty",m_spGridViewProp);
		ar("LeftView", m_spLeftView);
		ar("RightView", m_spRightView);
		//m_vwPath.clear();
		//for(auto pair : m_viewMap){
		//	m_vwPath.push_back(pair.second->GetPath());
		//}
		//ar("ViewPaths",m_vwPath);
		ar("FavoritesProperty",m_spFavoritesProp);
		ar("ApplicationProperty", m_spApplicationProp);

    }

    template <class Archive>
    void load(Archive& ar)
    {
        const type_info& info = typeid(ar);
		if(info == typeid(CSerializer&) || info == typeid(CDeserializer&)){ 
			ar("WindowRectangle", m_rcWnd);
		}
		ar("GridViewProperty",m_spGridViewProp);

		ar("LeftView", m_spLeftView);
		if (!m_spLeftView) { m_spLeftView = std::make_shared<CFilerTabGridView>();}
		m_spLeftView->SetParentWnd(this);
		m_spLeftView->SetGridViewProp(m_spGridViewProp);
		m_spLeftView->CreateWindowExArgument().hMenu((HMENU)9996);
		
		ar("RightView", m_spRightView);
		if (!m_spRightView) {m_spRightView = std::make_shared<CFilerTabGridView>();}
		m_spRightView->SetParentWnd(this);
		m_spRightView->SetGridViewProp(m_spGridViewProp);
		m_spLeftView->CreateWindowExArgument().hMenu((HMENU)9997);

		//ar("ViewPaths",m_vwPath);
		ar("FavoritesProperty",m_spFavoritesProp);
		ar("ApplicationProperty", m_spApplicationProp);
    }
};