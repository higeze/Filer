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
#include "PropertyWnd.h"
//#include "KonamiCommander.h"

class CFilerGridView;
struct FilerGridViewProperty;
//class CFavoritesProperty;
//class CFavoritesGridView;
class CShellFolder;
//class CFilerTabGridView;


#ifdef USE_PYTHON_EXTENSION
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
#endif

struct ExeExtension
{
	std::wstring Name;
	std::wstring Path;
	std::wstring Parameter;

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("Name", Name);
		ar("Path", Path);
		ar("Parameter", Parameter);
	}

};

struct ExeExtensionProperty
{
public:
	std::vector<ExeExtension> ExeExtensions;

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("ExeExtensions", ExeExtensions);
	}
};


class CFilerWnd:public CWnd
{
private:
	CRect m_rcWnd;
	CRect m_rcPropWnd;
	const int kSplitterWidth = 5;
	LONG m_splitterLeft;
	bool m_isSizing = false;
	CPoint m_ptStart;

	//Common properties
	std::shared_ptr<CApplicationProperty> m_spApplicationProp;
	std::shared_ptr<FilerGridViewProperty> m_spFilerGridViewProp;
	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;
	std::shared_ptr<ExeExtensionProperty> m_spExeExProp;

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
	LRESULT OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnCommandSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnCommandApplicationOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandFilerGridViewOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	LRESULT OnCommandFavoritesOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	LRESULT OnCommandExeExtensionOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandLeftViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandRightViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

public:
	template<class T, class... Args>
	LRESULT OnCommandOption(const std::wstring& name, std::shared_ptr<T>& spProp, std::function<void(const std::wstring&)> changed, Args&... args)
	{
		auto pPropWnd = new CPropertyWnd<T, Args...>(
			m_spFilerGridViewProp,
			name,
			spProp,
			args...);

		pPropWnd->SignalPropertyChanged.connect(changed);
		pPropWnd->SignalClose.connect([this](CPropertyWnd<T, Args...>* pWnd)->void{
			WINDOWPLACEMENT wp = { 0 };
			wp.length = sizeof(WINDOWPLACEMENT);
			pWnd->GetWindowPlacement(&wp);
			m_rcPropWnd = CRect(wp.rcNormalPosition);
		});

		HWND hWnd = NULL;
		if ((GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW) {
			hWnd = m_hWnd;
		} else {
			hWnd = GetAncestorByStyle(WS_OVERLAPPEDWINDOW);
		}

		pPropWnd->CreateOnCenterOfParent(hWnd, m_rcPropWnd.Size());
		pPropWnd->ShowWindow(SW_SHOW);
		pPropWnd->UpdateWindow();

		return 0;
	}

	FRIEND_SERIALIZER
    template <class Archive>
    void save(Archive& ar)
    {
        const type_info& info = typeid(ar);
		if(info == typeid(CSerializer&) || info == typeid(CDeserializer&)){ 
			ar("WindowRectangle", m_rcWnd);
			ar("PropertyWindowRectangle", m_rcPropWnd);
		}
		ar("LeftSplit", m_splitterLeft);

		ar("ApplicationProperty", m_spApplicationProp);
		ar("FilerGridViewProperty",m_spFilerGridViewProp);
		ar("FavoritesProperty", m_spFavoritesProp);
		ar("ExeExtensionProperty", m_spExeExProp);
		ar("LeftView", m_spLeftView);
		ar("RightView", m_spRightView);

		ar("LeftFavoritesView", m_spLeftFavoritesView);
		ar("RightFavoritesView", m_spRightFavoritesView);
#ifdef USE_PYTHON_EXTENSION
		ar("PythonExtensionProperty", m_spPyExProp);
#endif
	}

    template <class Archive>
    void load(Archive& ar)
    {
        const type_info& info = typeid(ar);
		if(info == typeid(CSerializer&) || info == typeid(CDeserializer&)){ 
			ar("WindowRectangle", m_rcWnd);
			ar("PropertyWindowRectangle", m_rcPropWnd);
		}
		ar("LeftSplit", m_splitterLeft);

		ar("ApplicationProperty", m_spApplicationProp);
		ar("FilerGridViewProperty",m_spFilerGridViewProp);
		ar("FavoritesProperty", m_spFavoritesProp);
		ar("ExeExtensionProperty", m_spExeExProp);
		ar("LeftView", m_spLeftView, m_spFilerGridViewProp);
		ar("RightView", m_spRightView, m_spFilerGridViewProp);

		ar("LeftFavoritesView", m_spLeftFavoritesView, this, m_spFilerGridViewProp, m_spFavoritesProp);
		ar("RightFavoritesView", m_spRightFavoritesView, this, m_spFilerGridViewProp, m_spFavoritesProp);
#ifdef USE_PYTHON_EXTENSION
		ar("PythonExtensionProperty", m_spPyExProp);
#endif
	}

#ifdef USE_PYTHON_EXTENSION
private:
	std::shared_ptr<CPythonExtensionProperty> m_spPyExProp;
	LRESULT OnCommandPythonExtensionOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
#endif

};