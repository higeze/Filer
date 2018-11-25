#pragma once
#include "MyWnd.h"
#include "UniqueIDFactory.h"
#include "MyFriendSerializer.h"
#include "MyRect.h"
#include "FilerTabGridView.h"
#include "FavoritesGridView.h"
#include "FavoritesProperty.h"
#include "ShellFolder.h"
#include "ApplicationProperty.h"
//#include "KonamiCommander.h"

class CFilerGridView;
class CGridViewProperty;
//class CFavoritesProperty;
//class CFavoritesGridView;
class CShellFolder;
//class CFilerTabGridView;

struct PythonExtension
{
	UINT ID;
	std::wstring Name;
	std::wstring ScriptPath;
	std::string FunctionName;

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("ID", ID);
		ar("Name", Name);
		ar("ScriptPath", ScriptPath);
		ar("FunctionName", FunctionName);
	}
};

class CFilerWnd:public CWnd
{
//public:
//	static CUniqueIDFactory ControlIDFactory;
private:
	const int kSplitterWidth = 5;

	CRect m_rcWnd;
	std::shared_ptr<CApplicationProperty> m_spApplicationProp;
	std::shared_ptr<CGridViewProperty> m_spGridViewProp;
	std::shared_ptr<CFavoritesGridView> m_spFavoritesView;
	std::shared_ptr<CFilerTabGridView> m_spLeftView;
	std::shared_ptr<CFilerTabGridView> m_spRightView;
	std::shared_ptr<CFilerTabGridView> m_spCurView;

	std::vector<PythonExtension> m_pyExtensions;
	//CKonamiCommander m_konamiCommander;

public:
	CFilerWnd();
	//CFilerWnd(const CFilerWnd&) = delete;
	//CFilerWnd(CFilerWnd&&) = default;
	//CFilerWnd& operator=(const CFilerWnd&) = delete;
	//CFilerWnd& operator=(CFilerWnd&&) = default;
	virtual ~CFilerWnd();
	virtual HWND Create(HWND hWndParent);
	std::shared_ptr<CApplicationProperty> GetApplicationProperty() { return m_spApplicationProp; }
	std::shared_ptr<CFavoritesGridView>& GetFavoritesView() { return m_spFavoritesView; }
	std::shared_ptr<CFilerTabGridView>& GetLeftView() { return m_spLeftView; }
	std::shared_ptr<CFilerTabGridView>& GetRightView() { return m_spRightView; }

private:
	std::shared_ptr<CShellFolder> GetShellFolderFromPath(const std::wstring& path);
	//LRESULT OnTabContextMenu(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	//void SetContextMenuTabIndex(int index){m_contextMenuTabIndex = index;}

	LRESULT OnCreate(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnClose(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnDestroy(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);

	bool m_isSizing = false;
	CPoint m_ptStart;
	CRect m_rcLeft;
	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	//LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//LRESULT OnPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

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

	LRESULT OnCommandLeftViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandRightViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandViewOption(std::shared_ptr<CFilerTabGridView>& view);

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

		ar("LeftViewRect", m_rcLeft);

		ar("GridViewProperty",m_spGridViewProp);

		ar("FavoritesView", m_spFavoritesView);

		ar("LeftView", m_spLeftView);
		ar("RightView", m_spRightView);

		ar("ApplicationProperty", m_spApplicationProp);

		ar("PythonExtensions", m_pyExtensions);

    }

    template <class Archive>
    void load(Archive& ar)
    {
        const type_info& info = typeid(ar);
		if(info == typeid(CSerializer&) || info == typeid(CDeserializer&)){ 
			ar("WindowRectangle", m_rcWnd);
		}

		ar("LeftViewRect", m_rcLeft);

		ar("GridViewProperty",m_spGridViewProp);

		ar("FavoritesView", m_spFavoritesView, m_spGridViewProp);

		ar("LeftView", m_spLeftView, m_spGridViewProp);
		m_spLeftView->SetParentWnd(this);
		m_spLeftView->CreateWindowExArgument().hMenu((HMENU)9996);
		
		ar("RightView", m_spRightView, m_spGridViewProp);
		m_spRightView->SetParentWnd(this);
		m_spLeftView->CreateWindowExArgument().hMenu((HMENU)9997);

		ar("ApplicationProperty", m_spApplicationProp);

		ar("PythonExtensions", m_pyExtensions);

		//{
		//	PythonExtension pyex;
		//	pyex.ID = IDM_ADDTOFAVORITEINGRID + 1000;
		//	pyex.Name = L"Pdf Split";
		//	pyex.ScriptPath = L"C:\\Users\\kuuna\\AppData\\Local\\Programs\\Python\\Python37-32\\pdf.py";
		//	pyex.FunctionName = "pdf_split";
		//	m_pyExtensions.push_back(pyex);
		//}
		//{
		//	PythonExtension pyex;
		//	pyex.ID = IDM_ADDTOFAVORITEINGRID + 1001;
		//	pyex.Name = L"Pdf Merge";
		//	pyex.ScriptPath = L"C:\\Users\\kuuna\\AppData\\Local\\Programs\\Python\\Python37-32\\pdf.py";
		//	pyex.FunctionName = "pdf_merge";
		//	m_pyExtensions.push_back(pyex);
		//}

	}
};