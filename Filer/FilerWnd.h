#pragma once
#include "MyWnd.h"
#include "UniqueIDFactory.h"
#include "MyFriendSerializer.h"
#include "MyRect.h"
#include "FilerGridView.h"
#include "FavoritesProperty.h"

class CFilerGridView;
class CGridViewProperty;
//class CFavoritesProperty;
class CFavoritesGridView;

class CFilerWnd:public CWnd
{
private:
	CRect m_rcWnd;
	CUniqueIDFactory m_uniqueIDFactory;
	std::shared_ptr<CTabCtrl> m_spTab;
	std::map<unsigned int, std::wstring> m_viewMap;
	std::shared_ptr<CFilerGridView> m_spFilerView;
	std::shared_ptr<CFavoritesGridView> m_spFavoritesView;

	unsigned int m_prevID;

	std::shared_ptr<CGridViewProperty> m_spGridViewProp;
	std::vector<std::wstring> m_vwPath;

	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;

	int m_contextMenuTabIndex;

public:
	CFilerWnd();
	//CFilerWnd(const CFilerWnd&) = delete;
	//CFilerWnd(CFilerWnd&&) = default;
	//CFilerWnd& operator=(const CFilerWnd&) = delete;
	//CFilerWnd& operator=(CFilerWnd&&) = default;
	virtual ~CFilerWnd();
	virtual HWND Create(HWND hWndParent);
private:
	std::shared_ptr<CShellFolder> GetShellFolderFromPath(const std::wstring& path);
	//LRESULT OnTabContextMenu(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	void SetContextMenuTabIndex(int index){m_contextMenuTabIndex = index;}

	LRESULT OnCreate(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnClose(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnDestroy(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnCommandNewTab(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	LRESULT OnCommandCloseTab(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	LRESULT OnNotifyTabSelChanging(int id, LPNMHDR, BOOL& bHandled);
	LRESULT OnNotifyTabSelChange(int id, LPNMHDR, BOOL& bHandled);
	LRESULT OnNotifyTabRClick(int id, LPNMHDR, BOOL& bHandled);
	LRESULT OnCommandOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	LRESULT OnCommandFavoritesOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	void AddNewView(std::wstring path);
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
		m_vwPath.clear();
		for(auto pair : m_viewMap){
			m_vwPath.push_back(pair.second);
		}
		ar("ViewPaths",m_vwPath);
		ar("FavoritesProperty",m_spFavoritesProp);

    }

    template <class Archive>
    void load(Archive& ar)
    {
        const type_info& info = typeid(ar);
		if(info == typeid(CSerializer&) || info == typeid(CDeserializer&)){ 
			ar("WindowRectangle", m_rcWnd);
		}
		ar("GridViewProperty",m_spGridViewProp);
		ar("ViewPaths",m_vwPath);
		ar("FavoritesProperty",m_spFavoritesProp);
    }
};