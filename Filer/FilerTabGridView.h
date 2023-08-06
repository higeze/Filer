#pragma once
#include "MyWnd.h"
#include "MyRect.h"
#include "MyFont.h"
#include "ShellFolder.h"
#include "ShellFileFactory.h"
#include "FilerGridView.h"
#include "FilerGridViewProperty.h"
#include "ToDoGridView.h"
#include "TabControl.h"
#include "ReactiveProperty.h"
#include "KnownFolder.h"
#include <functional>
#include "JsonSerializer.h"
#include "ReactiveProperty.h"
#include "encoding_type.h"
#include "FileStatus.h"
#include "ToDoDoc.h"
#include "PDFDoc.h"

#include "reactive_property.h"
#include "reactive_command.h"

class CFilerGridView;
struct FilerGridViewProperty;
struct GridViewProperty;
class CToDoGridView;
class CD2DWWindow;
class CShellFolder;
class CPDFEditor;
struct PDFEditorProperty;
struct EditorProperty;

class CPreviewControl;
struct PreviewControlProperty;

class CImageEditor;
struct ImageEditorProperty;
#include "D2DImage.h"

/***************/
/* FilerTabData */
/***************/
struct FilerTabData:public TabData
{
	std::wstring Path;
	std::shared_ptr<CShellFolder> FolderPtr;

	FilerTabData()
		:TabData(){ }

	FilerTabData(const std::wstring& path);
	FilerTabData(const std::shared_ptr<CShellFolder>& spFolder)
		:TabData(), FolderPtr(spFolder), Path(spFolder->GetPath()){}

	virtual ~FilerTabData() = default;

	FilerTabData(const FilerTabData& data)
	{
		Path = data.Path;
		FolderPtr = data.FolderPtr->Clone();
	}

	friend void to_json(json& j, const FilerTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Path"] = o.FolderPtr->GetPath();
	}
	friend void from_json(const json& j, FilerTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		j.at("Path").get_to(o.Path);
		if (!o.Path.empty()) {
			auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(o.Path);
			if (auto sp = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
				o.FolderPtr = sp;
			} else {
				o.FolderPtr = CKnownFolderManager::GetInstance()->GetDesktopFolder();
				o.Path = o.FolderPtr->GetPath();
			}
		}
	}
};

/***************/
/* ToDoTabData */
/***************/
struct ToDoTabData:public TabData
{
private:
	std::shared_ptr<int> Dummy;
public:
	CToDoDoc Doc;
	reactive_command_ptr<std::wstring> OpenCommand;
	reactive_command_ptr<std::wstring> SaveCommand;

	ToDoTabData(const std::wstring& path = std::wstring())
		:TabData(), Dummy(std::make_shared<int>(0)),OpenCommand(make_reactive_command<std::wstring>()),SaveCommand(make_reactive_command<std::wstring>())
	{
		OpenCommand->subscribe([this](const std::wstring& path) { Doc.Open(path); }, Dummy);
		SaveCommand->subscribe([this](const std::wstring& path) { Doc.Save(path); }, Dummy);
	};

	virtual ~ToDoTabData() = default;

	virtual bool AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing) override;

	friend void to_json(json& j, const ToDoTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Path"] = o.Doc.Path;
	}
	friend void from_json(const json& j, ToDoTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		j.at("Path").get_to(o.Doc.Path);
		o.Doc.Open(o.Doc.Path->get_const());
	}
};

/***************/
/* TextTabData */
/***************/
struct TextTabData :public TabData
{
	reactive_wstring_ptr Path;
	
	reactive_wstring_ptr Text;
	ReactiveProperty<encoding_type> Encoding;
	ReactiveProperty<CPointF> CaretPos;
	ReactiveTupleProperty<int, int, int, int, int> Carets;
	ReactiveProperty<FileStatus> Status;

	ReactiveCommand<HWND> OpenCommand;
	ReactiveCommand<HWND> SaveCommand;
	ReactiveCommand<HWND> OpenAsCommand;
	ReactiveCommand<HWND> SaveAsCommand;


	TextTabData(const std::wstring& path = std::wstring())
		:TabData(), Path(make_reactive_wstring(path)), Text(make_reactive_wstring()), Status(FileStatus::None), Carets(0, 0, 0, 0, 0)
	{
		OpenCommand.Subscribe([this](HWND hWnd) { Open(hWnd); });
		SaveCommand.Subscribe([this](HWND hWnd) { Save(hWnd); });
		OpenAsCommand.Subscribe([this](HWND hWnd) { OpenAs(hWnd); });
		SaveAsCommand.Subscribe([this](HWND hWnd) { SaveAs(hWnd); });

		//CloseCommand.Subscribe([this]() { Close(); });
		Text->subscribe([this](const reactive_wstring::notify_type&)
		{
			Status.set(FileStatus::Dirty);
		}, shared_from_this());
	}

	virtual ~TextTabData() = default;

	void Open(HWND hWnd);
	void OpenAs(HWND hWnd);
	void Open(const std::wstring& path, const encoding_type& enc);

	void Save(HWND hWnd);
	void SaveAs(HWND hWnd);
	void Save(const std::wstring& path, const encoding_type& enc);

	virtual bool AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing) override;

	friend void to_json(json& j, const TextTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Path"] = o.Path;
	}
	friend void from_json(const json& j, TextTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		j.at("Path").get_to(o.Path);
	}
};

/***************/
/* ImageTabData */
/***************/
struct ImageTabData :public TabData
{
	//ReactiveWStringProperty Path;
	ReactiveProperty<CD2DImage> Image;
	reactive_property_ptr<FLOAT> VScroll;
	reactive_property_ptr<FLOAT> HScroll;
	reactive_property_ptr<FLOAT> Scale;

	//ReactiveCommand<HWND> OpenCommand;
	//ReactiveCommand<HWND> SaveCommand;
	//ReactiveCommand<HWND> OpenAsCommand;
	//ReactiveCommand<HWND> SaveAsCommand;


	ImageTabData(CDirect2DWrite* pDirect = nullptr, const std::wstring& path = std::wstring())
		:TabData(),Image(CD2DImage(path)),
		VScroll(make_reactive_property<FLOAT>(0.f)),
		HScroll(make_reactive_property<FLOAT>(0.f)),
		Scale(make_reactive_property<FLOAT>(-1.f))
	{
		
		//OpenCommand.Subscribe([this](HWND hWnd) { Open(hWnd); });
		//SaveCommand.Subscribe([this](HWND hWnd) { Save(hWnd); });
		//OpenAsCommand.Subscribe([this](HWND hWnd) { OpenAs(hWnd); });
		//SaveAsCommand.Subscribe([this](HWND hWnd) { SaveAs(hWnd); });
	}

	virtual ~ImageTabData() = default;

	//void Open(HWND hWnd);
	//void OpenAs(HWND hWnd);
	//void Open(const std::wstring& path, const encoding_type& enc);

	//void Save(HWND hWnd);
	//void SaveAs(HWND hWnd);
	//void Save(const std::wstring& path, const encoding_type& enc);

	virtual bool AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing) override { return true; }

	friend void to_json(json& j, const ImageTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Image"] = o.Image;
	}
	friend void from_json(const json& j, ImageTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		j.at("Image").get_to(o.Image);
	}
};

/**************/
/* PdfTabData */
/**************/
struct PdfTabData :public TabData
{
	reactive_property_ptr<CPDFDoc> Doc;
	reactive_property_ptr<FLOAT> VScroll;
	reactive_property_ptr<FLOAT> HScroll;
	reactive_property_ptr<FLOAT> Scale;
	
	PdfTabData(const std::wstring& path = std::wstring())
		:TabData(),
		Doc(make_reactive_property<CPDFDoc>()),
		VScroll(make_reactive_property<FLOAT>(0.f)),
		HScroll(make_reactive_property<FLOAT>(0.f)),
		Scale(make_reactive_property<FLOAT>(-1.f))
	{
		Doc->get_const().Path->set(path);
	}

	virtual ~PdfTabData() = default;

	virtual bool AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing) override;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(
		PdfTabData,
		Doc)
};

/******************/
/* PreviewTabData */
/******************/

struct PreviewTabData :public TabData
{
	ReactiveWStringProperty Path;

	PreviewTabData(const std::wstring& path = std::wstring())
		:TabData(), Path(path){}

	virtual ~PreviewTabData() = default;

	NLOHMANN_DEFINE_TYPE_INTRUSIVE_NOTHROW(
		PreviewTabData,
		Path)
};

/*************/
/* CFilerTab */
/*************/
class CFilerTabGridView :public CTabControl
{
private:
	std::shared_ptr<FilerGridViewProperty> m_spFilerGridViewProp;
	std::shared_ptr<EditorProperty> m_spEditorProp;
	std::shared_ptr<PDFEditorProperty> m_spPdfEditorProp;
	std::shared_ptr<ImageEditorProperty> m_spImageEditorProp;
	std::shared_ptr<PreviewControlProperty> m_spPreviewControlProp;

	CBinding m_textBinding;
	CBinding m_textEncodingBinding;
	CBinding m_textPathBinding;
	CBinding m_pdfPathBinding;
	CBinding m_pdfScaleBinding;
	CBinding m_pdfVScrollBinding;
	CBinding m_pdfHScrollBinding;
	CBinding m_imageBinding;
	CBinding m_imageScaleBinding;
	CBinding m_imageVScrollBinding;
	CBinding m_imageHScrollBinding;
	CBinding m_statusBinding;
	CBinding m_caretPosBinding;
	CBinding m_caretsBinding;
	CBinding m_openBinding;
	CBinding m_openAsBinding;
	CBinding m_saveBinding;
	CBinding m_saveAsBinding;
	CBinding m_previewPathBinding;

	std::unique_ptr<sigslot::scoped_connection> m_pTextPathConnection;
	std::unique_ptr<sigslot::scoped_connection> m_pPdfPathConnection;
	std::unique_ptr<sigslot::scoped_connection> m_pPreviewPathConnection;
	std::unique_ptr<sigslot::scoped_connection> m_pImageConnection;
	std::unique_ptr<sigslot::scoped_connection> m_pStatusConnection;
	std::unique_ptr<sigslot::scoped_connection> m_pCloseConnection;

	std::pair<sigslot::scoped_connection, sigslot::scoped_connection> m_todoTasksConnections;

public:
	CFilerTabGridView(CD2DWControl* pParentControl = nullptr,
		const std::shared_ptr<TabControlProperty>& spTabProp = nullptr, 
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProrperty = nullptr,
		const std::shared_ptr<EditorProperty>& spTextboxProp = nullptr,
		const std::shared_ptr<PDFEditorProperty>& spPdfViewProp = nullptr,
		const std::shared_ptr<ImageEditorProperty>& spImageEditorProp = nullptr,
		const std::shared_ptr<PreviewControlProperty>& spPreviewControlProp = nullptr);
	virtual ~CFilerTabGridView();

	/****************/
	/* Pure Virtual */
	/****************/

	/**********/
	/* Getter */
	/**********/

	SHAREDPTR_GETTER(CFilerGridView, FilerGridView)
	SHAREDPTR_GETTER(CEditor, TextView)
	SHAREDPTR_GETTER(CPDFEditor, PdfView)
	SHAREDPTR_GETTER(CImageEditor, ImageView)
	SHAREDPTR_GETTER(CToDoGridView, ToDoGridView)
	SHAREDPTR_GETTER(CPreviewControl, PreviewControl)

	/**************/
	/* UI Message */
	/**************/
	void OnCreate(const CreateEvt& e) override;
	void OnContextMenu(const ContextMenuEvent& e) override;

	/***********/
	/* Command */
	/***********/
	void OnCommandNewTab(const CommandEvent& e) override { return OnCommandNewFilerTab(e); }
	void OnCommandNewFilerTab(const CommandEvent& e);
	void OnCommandNewToDoTab(const CommandEvent& e);
	void OnCommandNewTextTab(const CommandEvent& e);
	void OnCommandNewPdfTab(const CommandEvent& e);
	void OnCommandNewImageTab(const CommandEvent& e);
	void OnCommandNewPreviewTab(const CommandEvent& e);

	void OnCommandAddToFavorite(const CommandEvent& e);
	void OnCommandOpenSameAsOther(const CommandEvent& e);

	/***********/
	/* Observe */
	/***********/
	void ObservePath(const reactive_wstring::notify_type&);

public:
	friend void to_json(json& j, const CFilerTabGridView& o)
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData );
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PdfTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ImageTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PreviewTabData);

		to_json(j, static_cast<const CTabControl&>(o));

		j["FilerView"] = o.m_spFilerGridView;
		j["ToDoView"] = o.m_spToDoGridView;
	}

	friend void from_json(const json& j, CFilerTabGridView& o)
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PdfTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ImageTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PreviewTabData);

		from_json(j, static_cast<CTabControl&>(o));

		get_to(j, "FilerView", o.m_spFilerGridView);
		get_to(j, "ToDoView", o.m_spToDoGridView);
	}
};

