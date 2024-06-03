#pragma once
//#include <boost/asio.hpp>
//#include <boost/date_time/posix_time/posix_time.hpp>
//#include <boost/asio/deadline_timer.hpp>
//#include <boost/asio/io_service.hpp>

#include "MyWnd.h"
#include "D2DWTypes.h"
#include "MyFriendSerializer.h"
#include "MyRect.h"
#include "FilerTabGridView.h"
//#include "FavoritesGridView.h"
#include "FavoritesProperty.h"
#include "LauncherGridView.h"
#include "LauncherProperty.h"
#include "ShellFolder.h"
#include "ApplicationProperty.h"
#include "ViewProperty.h"
#include "CellProperty.h"
#include "StatusBar.h"
#include "Splitter.h"
#include "FilerGridViewProperty.h"
#include "D2DWWindow.h"
#include "reactive_property.h"
#include "PdfEditorProperty.h"
#include "ImageEditorProperty.h"
#include "ExeExtensionProperty.h"
#include "EditorProperty.h"
#include "DialogProperty.h"
//#include "KonamiCommander.h"
#include "JsonSerializer.h"
#include "TabControlProperty.h"
#include "PreviewControlProperty.h"

#include "ToolBar.h"
#include "Timer.h"

class CFilerGridView;
#include "LauncherGridView.h"
class CShellFolder;
class CPerformance;

#include "SplitContainer.h"
#include "ColoredTextBox.h"
#include "StatusBar.h"
#include "PreviewButton.h"
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

class CD2DFileIconDrawer;
class CNetworkMessanger;

class CFilerWnd:public CD2DWWindow
{
private:
	static std::vector<std::wstring> imageExts;
	static std::vector<std::wstring> previewExts;

	//boost::asio::io_service m_reloadIosv;
	//boost::asio::io_service::work m_reloadWork;
	//boost::asio::deadline_timer m_reloadTimer;
	CTimer m_timer;
	CDeadlineTimer m_reloadTimer;

	std::unique_ptr<CNetworkMessanger> m_pNetworkMessanger;

	bool m_isSizing = false;
	CPoint m_ptBeginClient;
	//bool m_isPreview = false;

	//Common properties
	std::shared_ptr<CApplicationProperty> m_spApplicationProp;
	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;
	std::shared_ptr<CLauncherProperty> m_spLauncherProp;
	std::shared_ptr<ExeExtensionProperty> m_spExeExProp;

	//Controls
	//std::shared_ptr<CDockPanel> spDock;
	//std::shared_ptr<CLauncherGridView> m_spLauncher;
	//std::shared_ptr<CToolBar> m_spToolBar;
	//std::shared_ptr<CStatusBar> m_spStatusBar;
	//std::shared_ptr<CHorizontalSplitContainer> m_spHorizontalSplit;
	//std::shared_ptr<CVerticalSplitContainer> m_spTopVerticalSplit;
	//std::shared_ptr<CVerticalSplitContainer> m_spBottomVerticalSplit;

	//std::shared_ptr<CFilerTabGridView> m_spLeftView;
	//std::shared_ptr<CFilerTabGridView> m_spRightView;
	//std::shared_ptr<CFilerTabGridView> m_spCurView;
	//std::shared_ptr<CFavoritesGridView> m_spLeftFavoritesView;
	//std::shared_ptr<CFavoritesGridView> m_spRightFavoritesView;
	//std::shared_ptr<CColoredTextBox> m_spLogText;
	//std::shared_ptr<CHorizontalSplitter> m_spHSplitter;
	//std::shared_ptr<CVerticalSplitter> m_spVSplitter;




	//Other
	std::unique_ptr<CPerformance> m_pPerformance;
	
	//CKonamiCommander m_konamiCommander;

public:
	std::shared_ptr<int> Dummy;
	reactive_property_ptr<CRect> Rectangle;
	reactive_wstring_ptr PerformanceLog;
	reactive_wstring_ptr ThreadPoolLog;
	reactive_vector_ptr<CFavorite> Favorites;
	reactive_vector_ptr<CLauncher> Launchers;
	reactive_property_ptr<bool> IsPreview;


	virtual void Measure(const CSizeF& availableSize) override;
	virtual void Arrange(const CRectF& rc) override;
	

public:
	//Constructor
	CFilerWnd();
	virtual ~CFilerWnd();

	HWND Create(HWND hWndParent) override;


	//Getter/Setter
	std::shared_ptr<CApplicationProperty>& GetApplicationProperty() { return m_spApplicationProp; }
	std::shared_ptr<CFavoritesProperty>& GetFavoritesPropPtr() { return m_spFavoritesProp; }
	void SetFavoritesPropPtr(const std::shared_ptr<CFavoritesProperty>& value) { m_spFavoritesProp = value; }

	std::shared_ptr<CLauncherProperty>& GetLauncherPropPtr() { return m_spLauncherProp; }
	void SetLauncherPropPtr(const std::shared_ptr<CLauncherProperty>& value) { m_spLauncherProp = value; }

	std::shared_ptr<ExeExtensionProperty>& GetExeExtensionPropPtr() { return m_spExeExProp; }
	void SetExeExtensionPropPtr(const std::shared_ptr<ExeExtensionProperty>& value) { m_spExeExProp = value; }

	//std::shared_ptr<CFavoritesGridView>& GetLeftFavoritesView() { return m_spLeftFavoritesView; }
	//std::shared_ptr<CFavoritesGridView>& GetRightFavoritesView() { return m_spRightFavoritesView; }
	//std::shared_ptr<CFilerTabGridView>& GetLeftWnd() { return m_spLeftView; }
	//std::shared_ptr<CFilerTabGridView>& GetRightWnd() { return m_spRightView; }

	// Launcher, ToolBar, LeftFavorite, LeftTab, Splitter, RightFavorite, RightTab, Splitter, Log,  StatusBar
	//std::tuple<CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF, CRectF> GetRects();

	/******************/
	/* Window Message */
	/******************/
	virtual LRESULT OnDestroy(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnConnectivityChanged(UINT uiMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDeviceChange(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	virtual void Reload();

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

	void SetUpPreview(const std::shared_ptr<CFilerTabGridView>& subject, const std::shared_ptr<CFilerTabGridView>& observer);
	void SetUpFilerGrid(const std::shared_ptr<CFilerTabGridView>& subject, const std::shared_ptr<CFilerTabGridView>& observer);


	
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

private:
	void SetUpFilerView(const std::shared_ptr<CFilerView>& view, const std::shared_ptr<CStatusBar>& status);

	friend void to_json(json& j, const CFilerWnd& o) 
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CToolBar);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CLauncherGridView);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CStatusBar);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CDockPanel);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CVerticalSplitter);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CHorizontalSplitter);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CFilerTabGridView);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CColoredTextBox);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CPreviewButton);

		j = json{
			{"Rectangle", o.Rectangle},
			{"ApplicationProperty", o.m_spApplicationProp },
			{"LauncherProperty", o.m_spLauncherProp },
			{"FavoritesProperty", o.m_spFavoritesProp },
			{"ExeExtensionProperty", o.m_spExeExProp },
			{"Children", o.m_childControls}
		};
	}

	friend void from_json(const json& j, CFilerWnd& o)
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CToolBar);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CLauncherGridView);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CStatusBar);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CDockPanel);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CVerticalSplitter);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CHorizontalSplitter);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CFilerTabGridView);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CColoredTextBox);
		JSON_REGISTER_POLYMORPHIC_RELATION(CD2DWControl, CPreviewButton);

		get_to(j, "Rectangle", o.Rectangle);
		get_to(j, "ApplicationProperty", o.m_spApplicationProp);
		get_to(j, "LauncherProperty", o.m_spLauncherProp);
		get_to(j, "FavoritesProperty", o.m_spFavoritesProp);
		get_to(j, "ExeExtensionProperty", o.m_spExeExProp);
		get_to(j, "Children", o.m_childControls);
	}
};

