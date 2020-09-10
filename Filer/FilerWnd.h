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
#include "StatusBar.h"
#include "FilerGridViewProperty.h"
#include "TextboxWnd.h"
//#include "KonamiCommander.h"

class CFilerGridView;
struct FilerGridViewProperty;
class CShellFolder;


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
	observable_vector<std::tuple<ExeExtension>> ExeExtensions;

	FRIEND_SERIALIZER
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("ExeExtensions", ExeExtensions);
	}
};


class CFilerWnd:public d2dw::CWindow
{
private:
	CRect m_rcWnd;
	CRect m_rcPropWnd;
	const int kSplitterWidth = 5;
	LONG m_splitterLeft;
	bool m_isSizing = false;
	CPoint m_ptBeginClient;

	//Common properties
	std::shared_ptr<CApplicationProperty> m_spApplicationProp;
	std::shared_ptr<FilerGridViewProperty> m_spFilerGridViewProp;
	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;
	std::shared_ptr<ExeExtensionProperty> m_spExeExProp;
	std::shared_ptr<TextEditorProperty> m_spTextEditorProp;
	std::shared_ptr<TabControlProperty> m_spTabControlProp;


	//Controls
	std::shared_ptr<CFilerTabGridView> m_spLeftView;
	std::shared_ptr<CFilerTabGridView> m_spRightView;
	std::shared_ptr<CFilerTabGridView> m_spCurView;
	std::shared_ptr<CFavoritesGridView> m_spLeftFavoritesView;
	std::shared_ptr<CFavoritesGridView> m_spRightFavoritesView;
	std::shared_ptr<d2dw::CStatusBar> m_spStatusBar;

	//Property
	d2dw::SolidFill BackgroundFill = d2dw::SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);
	
	//CKonamiCommander m_konamiCommander;

public:
	//Constructor
	CFilerWnd();
	virtual ~CFilerWnd();

	//Getter
	std::shared_ptr<CApplicationProperty>& GetApplicationProperty() { return m_spApplicationProp; }
	std::shared_ptr<FilerGridViewProperty>& GetFilerGridViewPropPtr() { return m_spFilerGridViewProp; }
	std::shared_ptr<CFavoritesProperty>& GetFavoritesPropPtr() { return m_spFavoritesProp; }

	std::shared_ptr<CFavoritesGridView>& GetLeftFavoritesView() { return m_spLeftFavoritesView; }
	std::shared_ptr<CFavoritesGridView>& GetRightFavoritesView() { return m_spRightFavoritesView; }
	std::shared_ptr<CFilerTabGridView>& GetLeftWnd() { return m_spLeftView; }
	std::shared_ptr<CFilerTabGridView>& GetRightWnd() { return m_spRightView; }

private:
	virtual void OnClose(const CloseEvent& e)override;
	virtual LRESULT OnDestroy(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnCommandSave(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandExit(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnCommandApplicationOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandFilerGridViewOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	LRESULT OnCommandTextOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandFavoritesOption(WORD wNotifyCode,WORD wID,HWND hWndCtl,BOOL& bHandled);
	LRESULT OnCommandExeExtensionOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandLeftViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandRightViewOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	virtual void OnCreate(const CreateEvent& e) override;
	virtual void OnRect(const RectEvent& e) override;
	//virtual void OnSetFocus(const SetFocusEvent& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override;

	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e) override;

public:
	//TODODO
	//template<class T, class... Args>
	//LRESULT OnCommandOption(const std::wstring& name, std::shared_ptr<T>& spProp, std::function<void(const std::wstring&)> changed, Args&... args)
	//{
	//	auto pPropWnd = new CPropertyWnd<T, Args...>(
	//		m_spFilerGridViewProp,
	//		name,
	//		spProp,
	//		args...);

	//	pPropWnd->SignalPropertyChanged.connect(changed);
	//	pPropWnd->SignalClose.connect([this](CPropertyWnd<T, Args...>* pWnd)->void{
	//		WINDOWPLACEMENT wp = { 0 };
	//		wp.length = sizeof(WINDOWPLACEMENT);
	//		pWnd->GetWindowPlacement(&wp);
	//		m_rcPropWnd = CRect(wp.rcNormalPosition);
	//	});

	//	HWND hWnd = NULL;
	//	if ((GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW) {
	//		hWnd = m_hWnd;
	//	} else {
	//		hWnd = GetAncestorByStyle(WS_OVERLAPPEDWINDOW);
	//	}

	//	pPropWnd->CreateOnCenterOfParent(hWnd, m_rcPropWnd.Size());
	//	pPropWnd->ShowWindow(SW_SHOW);
	//	pPropWnd->UpdateWindow();

	//	return 0;
	//}

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
		ar("TextEditorProperty", m_spTextEditorProp);
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
		ar("TextEditorProperty", m_spTextEditorProp);
		ar("FavoritesProperty", m_spFavoritesProp);
		ar("ExeExtensionProperty", m_spExeExProp);
		ar("LeftView", m_spLeftView, this, m_spTabControlProp, m_spFilerGridViewProp, m_spTextEditorProp);
		ar("RightView", m_spRightView, this, m_spTabControlProp, m_spFilerGridViewProp, m_spTextEditorProp);

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