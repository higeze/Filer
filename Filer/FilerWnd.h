#pragma once
#include "MyWnd.h"
#include "MyFriendSerializer.h"
#include "MyRect.h"
#include "FilerTabGridView.h"
#include "FavoritesGridView.h"
#include "FavoritesProperty.h"
#include "LauncherGridView.h"
#include "LauncherProperty.h"
#include "ShellFolder.h"
#include "ApplicationProperty.h"
#include "ViewProperty.h"
#include "CellProperty.h"
#include "PropertyWnd.h"
#include "FilerWndStatusBar.h"
#include "Splitter.h"
#include "FilerGridViewProperty.h"
#include "D2DWWindow.h"
#include "ReactiveProperty.h"
#include "PdfEditorProperty.h"
#include "ExeExtensionProperty.h"
#include "EditorProperty.h"
#include "DialogProperty.h"
//#include "KonamiCommander.h"
#include "JsonSerializer.h"

class CFilerGridView;
class CLauncherGridView;
struct FilerGridViewProperty;
class CShellFolder;
//struct PdfViewProperty;


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


class CFilerWnd:public CD2DWWindow
{
private:
	CRect m_rcWnd;
	CRect m_rcPropWnd;
	ReactiveProperty<FLOAT> m_splitterLeft;

	bool m_isSizing = false;
	CPoint m_ptBeginClient;

	//Common properties
	std::shared_ptr<CApplicationProperty> m_spApplicationProp;
	std::shared_ptr<FilerGridViewProperty> m_spFilerGridViewProp;
	std::shared_ptr<StatusBarProperty> m_spStatusBarProp;
	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;
	std::shared_ptr<CLauncherProperty> m_spLauncherProp;

	std::shared_ptr<ExeExtensionProperty> m_spExeExProp;
	std::shared_ptr<EditorProperty> m_spEditorProp;
	std::shared_ptr<PDFEditorProperty> m_spPdfEditorProp;
	std::shared_ptr<TabControlProperty> m_spTabControlProp;
	std::shared_ptr<SplitterProperty> m_spSplitterProp;
	std::shared_ptr<DialogProperty> m_spDialogProp;

	//Controls
	std::shared_ptr<CFilerTabGridView> m_spLeftView;
	std::shared_ptr<CFilerTabGridView> m_spRightView;
	std::shared_ptr<CFilerTabGridView> m_spCurView;
	std::shared_ptr<CFavoritesGridView> m_spLeftFavoritesView;
	std::shared_ptr<CFavoritesGridView> m_spRightFavoritesView;
	std::shared_ptr<CLauncherGridView> m_spLauncher;
	std::shared_ptr<CFilerWndStatusBar> m_spStatusBar;
	std::shared_ptr<CHorizontalSplitter> m_spSplitter;

	//Property
	SolidFill BackgroundFill = SolidFill(200.f / 255.f, 200.f / 255.f, 200.f / 255.f, 1.0f);
	
	//CKonamiCommander m_konamiCommander;
	CBinding m_splitterBinding;

public:
	//Constructor
	CFilerWnd();
	virtual ~CFilerWnd();

	HWND Create(HWND hWndParent) override;


	//Getter/Setter
	std::shared_ptr<CApplicationProperty>& GetApplicationProperty() { return m_spApplicationProp; }
	std::shared_ptr<FilerGridViewProperty>& GetFilerGridViewPropPtr() { return m_spFilerGridViewProp; }
	std::shared_ptr<CFavoritesProperty>& GetFavoritesPropPtr() { return m_spFavoritesProp; }
	void SetFavoritesPropPtr(const std::shared_ptr<CFavoritesProperty>& value) { m_spFavoritesProp = value; }

	std::shared_ptr<CLauncherProperty>& GetLauncherPropPtr() { return m_spLauncherProp; }
	void SetLauncherPropPtr(const std::shared_ptr<CLauncherProperty>& value) { m_spLauncherProp = value; }

	std::shared_ptr<ExeExtensionProperty>& GetExeExtensionPropPtr() { return m_spExeExProp; }
	void SetExeExtensionPropPtr(const std::shared_ptr<ExeExtensionProperty>& value) { m_spExeExProp = value; }

	std::shared_ptr<DialogProperty>& GetDialogPropPtr(){ return m_spDialogProp; }


	std::shared_ptr<CFavoritesGridView>& GetLeftFavoritesView() { return m_spLeftFavoritesView; }
	std::shared_ptr<CFavoritesGridView>& GetRightFavoritesView() { return m_spRightFavoritesView; }
	std::shared_ptr<CFilerTabGridView>& GetLeftWnd() { return m_spLeftView; }
	std::shared_ptr<CFilerTabGridView>& GetRightWnd() { return m_spRightView; }

	// Launcher, LeftFavorite, LeftTab, Splitter, RightFavorite, RightTab, StatusBar
	std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF> GetRects();

	/******************/
	/* Window Message */
	/******************/
	virtual LRESULT OnDestroy(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	/**************/
	/* UI Message */
	/**************/
	virtual void OnPaint(const PaintEvent& e)override;
	virtual void OnClose(const CloseEvent& e)override;
	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnRect(const RectEvent& e) override;
	//virtual void OnSetFocus(const SetFocusEvent& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override;
	virtual void OnLButtonDown(const LButtonDownEvent& e) override;
	virtual void OnLButtonUp(const LButtonUpEvent& e) override;
	virtual void OnMouseMove(const MouseMoveEvent& e) override;
	
	/***********/
	/* Command */
	/***********/
	void OnCommandSave(const CommandEvent& e);
	void OnCommandExit(const CommandEvent& e);

	void OnCommandApplicationOption(const CommandEvent& e);
	void OnCommandFilerGridViewOption(const CommandEvent& e);
	void OnCommandTextOption(const CommandEvent& e);
	void OnCommandLauncherOption(const CommandEvent& e);
	void OnCommandFavoritesOption(const CommandEvent& e);
	void OnCommandExeExtensionOption(const CommandEvent& e);

public:
	//TODOLOW
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
		ar("EditorProperty", m_spEditorProp);
		ar("PdfEditorProperty", m_spPdfEditorProp);
		ar("LauncherProperty", m_spLauncherProp);
		ar("FavoritesProperty", m_spFavoritesProp);
		ar("ExeExtensionProperty", m_spExeExProp);
		ar("LeftView", m_spLeftView);
		ar("RightView", m_spRightView);
		ar("HorizontalSplitter", m_spSplitter);
		//ar("LeftFavoritesView", m_spLeftFavoritesView);
		//ar("RightFavoritesView", m_spRightFavoritesView);
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
		ar("EditorProperty", m_spEditorProp);
		ar("PdfEditorProperty", m_spPdfEditorProp);
		ar("LauncherProperty", m_spLauncherProp);
		ar("FavoritesProperty", m_spFavoritesProp);
		ar("ExeExtensionProperty", m_spExeExProp);
		ar("SplitterProperty", m_spSplitterProp);
		ar("LeftView", m_spLeftView, this, m_spTabControlProp, m_spFilerGridViewProp, m_spEditorProp, m_spPdfEditorProp);
		ar("RightView", m_spRightView, this, m_spTabControlProp, m_spFilerGridViewProp, m_spEditorProp, m_spPdfEditorProp);
		ar("HorizontalSplitter", m_spSplitter, this, m_spLeftView.get(), m_spRightView.get(), m_spSplitterProp);
		//ar("LeftFavoritesView", m_spLeftFavoritesView, this, m_spFilerGridViewProp, m_spFavoritesProp);
		//ar("RightFavoritesView", m_spRightFavoritesView, this, m_spFilerGridViewProp, m_spFavoritesProp);
#ifdef USE_PYTHON_EXTENSION
		ar("PythonExtensionProperty", m_spPyExProp);
#endif
	}

#ifdef USE_PYTHON_EXTENSION
private:
	std::shared_ptr<CPythonExtensionProperty> m_spPyExProp;
	LRESULT OnCommandPythonExtensionOption(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
#endif

	friend void to_json(json& j, const CFilerWnd& o) 
	{
		j = json{
			{"WindowRectangle", o.m_rcWnd},
			{"PropertyWindowRectangle", o.m_rcPropWnd},
			{"LeftSplit", o.m_splitterLeft},
			{"ApplicationProperty", o.m_spApplicationProp },
			{"FilerGridViewProperty", o.m_spFilerGridViewProp },
			{"EditorProperty", o.m_spEditorProp },
			{"PdfEditorProperty", o.m_spPdfEditorProp },
			{"StatusBarProperty", o.m_spStatusBarProp},
			{"LauncherProperty", o.m_spLauncherProp },
			{"FavoritesProperty", o.m_spFavoritesProp },
			{"ExeExtensionProperty", o.m_spExeExProp },
			{"SplitterProperty", o.m_spSplitterProp},
			{"LeftFavorites", o.m_spLeftFavoritesView},
			{"RightFavorites", o.m_spRightFavoritesView},
			{"Launcher", o.m_spLauncher},
			{"LeftView", o.m_spLeftView },
			{"RightView", o.m_spRightView },
			{"HorizontalSplitter", o.m_spSplitter },
			{"DialogProperty", o.m_spDialogProp}
	#ifdef USE_PYTHON_EXTENSION
			{ "PythonExtensionProperty", m_spPyExProp }
	#endif
		};
	}

	friend void from_json(const json& j, CFilerWnd& o)
	{
			get_to(j, "WindowRectangle", o.m_rcWnd);
			get_to(j, "PropertyWindowRectangle", o.m_rcPropWnd);
			get_to(j, "LeftSplit", o.m_splitterLeft);
			get_to(j, "ApplicationProperty", o.m_spApplicationProp);
			get_to(j, "FilerGridViewProperty", o.m_spFilerGridViewProp);
			get_to(j, "EditorProperty", o.m_spEditorProp);
			get_to(j, "PdfEditorProperty", o.m_spPdfEditorProp);
			get_to(j, "StatusBarProperty", o.m_spStatusBarProp);
			get_to(j, "LauncherProperty", o.m_spLauncherProp);
			get_to(j, "FavoritesProperty", o.m_spFavoritesProp);
			get_to(j, "ExeExtensionProperty", o.m_spExeExProp);
			get_to(j, "SplitterProperty", o.m_spSplitterProp);
			get_to(j, "LeftFavorites", o.m_spLeftFavoritesView);
			get_to(j, "RightFavorites", o.m_spRightFavoritesView);
			get_to(j, "Launcher", o.m_spLauncher);
			get_to(j, "LeftView", o.m_spLeftView);
			get_to(j, "RightView", o.m_spRightView);
			get_to(j, "HorizontalSplitter", o.m_spSplitter);
			get_to(j, "DialogProp", o.m_spDialogProp);
	#ifdef USE_PYTHON_EXTENSION
			get_to(j, "PythonExtensionProperty", m_spPyExProp);
	#endif
	}
};

