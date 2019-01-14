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
#include "ViewProperty.h"
#include "CellProperty.h"
//#include "KonamiCommander.h"

class CFilerGridView;
class CGridViewProperty;
//class CFavoritesProperty;
//class CFavoritesGridView;
class CShellFolder;
//class CFilerTabGridView;

struct PythonExtension
{
	//UINT ID;
	std::wstring Name;
	std::wstring ScriptPath;
	std::string FunctionName;

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		//ar("ID", ID);
		ar("Name", Name);
		ar("ScriptPath", ScriptPath);
		ar("FunctionName", FunctionName);
	}
};

struct CPythonExtensionProperty
{
public:
	std::wstring PythonHome;
	std::vector<PythonExtension> PythonExtensions;

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("PythonHome", PythonHome);
		ar("PythonExtensions", PythonExtensions);
	}
};

class CFilerWnd:public CWnd
{
//public:
//	static CUniqueIDFactory ControlIDFactory;
private:
	CRect m_rcWnd;

	const int kSplitterWidth = 5;
	LONG m_splitterLeft;
	bool m_isSizing = false;
	CPoint m_ptStart;

	//Common properties
	std::shared_ptr<CApplicationProperty> m_spApplicationProp;
	std::shared_ptr<CGridViewProperty> m_spGridViewProp;
	std::shared_ptr<FilerGridViewProperty> m_spFilerGridViewProp;
	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;
	std::shared_ptr<CPythonExtensionProperty> m_spPyExProp;
	//Windows
	std::shared_ptr<CFavoritesGridView> m_spLeftFavoritesView;
	std::shared_ptr<CFavoritesGridView> m_spRightFavoritesView;

	std::shared_ptr<CFilerTabGridView> m_spLeftView;
	std::shared_ptr<CFilerTabGridView> m_spRightView;
	
	std::shared_ptr<CFilerTabGridView> m_spCurView;
	
	//CKonamiCommander m_konamiCommander;

public:
	//Constructor
	CFilerWnd();
	virtual ~CFilerWnd();

	virtual HWND Create(HWND hWndParent);
	//Getter
	std::shared_ptr<CApplicationProperty>& GetApplicationProperty() { return m_spApplicationProp; }
	std::shared_ptr<FilerGridViewProperty>& GetFilerGridViewPropPtr() { return m_spFilerGridViewProp; }
	std::shared_ptr<CGridViewProperty>& GetGridViewPropPtr() { return m_spGridViewProp; }
	std::shared_ptr<CFavoritesProperty>& GetFavoritesPropPtr() { return m_spFavoritesProp; }

	std::shared_ptr<CFavoritesGridView>& GetLeftFavoritesView() { return m_spLeftFavoritesView; }
	std::shared_ptr<CFavoritesGridView>& GetRightFavoritesView() { return m_spRightFavoritesView; }
	std::shared_ptr<CFilerTabGridView>& GetLeftView() { return m_spLeftView; }
	std::shared_ptr<CFilerTabGridView>& GetRightView() { return m_spRightView; }

private:
	LRESULT OnCreate(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnClose(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnDestroy(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnSize(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	LRESULT OnSetFocus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);


	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	template<class T, class... Args>
	LRESULT OnCommandOption(const std::wstring& name, std::shared_ptr<T>& spProp, std::function<void(const std::wstring&)> changed, Args&... args)
	{
		auto pPropWnd = new CPropertyWnd<T, Args...>(
			m_spGridViewProp,
			name,
			spProp,
			args...);

		pPropWnd->PropertyChanged.connect(changed);

		CRect rc(0, 0, 0, 0);
		pPropWnd->Create(m_hWnd, rc);
		rc = CRect(pPropWnd->GetGridView()->GetDirect()->Dips2Pixels(pPropWnd->GetGridView()->MeasureSize()));
		AdjustWindowRectEx(&rc, WS_OVERLAPPEDWINDOW, TRUE, 0);
		pPropWnd->MoveWindow(0, 0, rc.Width() + ::GetSystemMetrics(SM_CXVSCROLL), min(500, rc.Height() + ::GetSystemMetrics(SM_CYVSCROLL) + 10), FALSE);
		pPropWnd->CenterWindow();
		pPropWnd->ShowWindow(SW_SHOW);
		pPropWnd->UpdateWindow();
		
		return 0;
	}

	LRESULT OnCommandApplicationOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandFilerGridViewOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	LRESULT OnCommandGridViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandFavoritesOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	LRESULT OnCommandPythonExtensionOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnCommandLeftViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandRightViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:
	FRIEND_SERIALIZER
    template <class Archive>
    void save(Archive& ar)
    {
        const type_info& info = typeid(ar);
		if(info == typeid(CSerializer&) || info == typeid(CDeserializer&)){ 
			ar("WindowRectangle", m_rcWnd);
		}
		ar("LeftSplit", m_splitterLeft);

		ar("ApplicationProperty", m_spApplicationProp);
		ar("GridViewProperty", m_spGridViewProp);
		ar("FilerGridViewProperty",m_spFilerGridViewProp);
		ar("FavoritesProperty", m_spFavoritesProp);
		ar("PythonExtensionProperty", m_spPyExProp);

		ar("LeftView", m_spLeftView);
		ar("RightView", m_spRightView);

		ar("LeftFavoritesView", m_spLeftFavoritesView);
		ar("RightFavoritesView", m_spRightFavoritesView);
    }

    template <class Archive>
    void load(Archive& ar)
    {
        const type_info& info = typeid(ar);
		if(info == typeid(CSerializer&) || info == typeid(CDeserializer&)){ 
			ar("WindowRectangle", m_rcWnd);
		}
		ar("LeftSplit", m_splitterLeft);

		ar("ApplicationProperty", m_spApplicationProp);
		ar("GridViewProperty", m_spGridViewProp);
		ar("FilerGridViewProperty",m_spFilerGridViewProp);
		ar("FavoritesProperty", m_spFavoritesProp);
		ar("PythonExtensionProperty", m_spPyExProp);

		ar("LeftView", m_spLeftView, m_spGridViewProp,  m_spFilerGridViewProp);		
		ar("RightView", m_spRightView, m_spGridViewProp, m_spFilerGridViewProp);

		ar("LeftFavoritesView", m_spLeftFavoritesView, m_spGridViewProp, m_spFavoritesProp);
		ar("RightFavoritesView", m_spRightFavoritesView, m_spGridViewProp, m_spFavoritesProp);
	}
};