#pragma once
#include "MyWnd.h"
#include "MyRect.h"
#include "MyFont.h"
#include "ShellFolder.h"
#include "ShellFileFactory.h"
#include "FilerGridView.h"
#include "FilerGridViewProperty.h"
#include "TabControl.h"
#include "ReactiveProperty.h"
#include "KnownFolder.h"
#include <functional>
#include "JsonSerializer.h"
#include "ReactiveProperty.h"
#include "encoding_type.h"

class CFilerGridView;
struct FilerGridViewProperty;
struct GridViewProperty;
class CToDoGridView;
class CD2DWWindow;
class CShellFolder;
class CPDFEditor;
struct PDFEditorProperty;
struct EditorProperty;

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
	std::wstring Path;

	ToDoTabData(const std::wstring& path = std::wstring())
		:TabData(), Path(path){}

	virtual ~ToDoTabData() = default;

	friend void to_json(json& j, const ToDoTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Path"] = o.Path;
	}
	friend void from_json(const json& j, ToDoTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		j.at("Path").get_to(o.Path);
	}
};

/***************/
/* TextTabData */
/***************/

enum class TextStatus
{
	None,
	Dirty,
	Saved
};


struct TextTabData :public TabData
{
	ReactiveWStringProperty Path;
	
	ReactiveWStringProperty Text;
	ReactiveProperty<encoding_type> Encoding;
	ReactiveProperty<CPointF> CaretPos;
	ReactiveTupleProperty<int, int, int, int, int> Carets;
	ReactiveProperty<TextStatus> Status;

	ReactiveCommand<HWND> OpenCommand;
	ReactiveCommand<HWND> SaveCommand;
	ReactiveCommand<HWND> OpenAsCommand;
	ReactiveCommand<HWND> SaveAsCommand;


	TextTabData(const std::wstring& path = std::wstring())
		:TabData(), Path(path), Text(), Status(TextStatus::None), Carets(0,0,0,0,0)
	{
		OpenCommand.Subscribe([this](HWND hWnd) { Open(hWnd); });
		SaveCommand.Subscribe([this](HWND hWnd) { Save(hWnd); });
		OpenAsCommand.Subscribe([this](HWND hWnd) { OpenAs(hWnd); });
		SaveAsCommand.Subscribe([this](HWND hWnd) { SaveAs(hWnd); });

		//CloseCommand.Subscribe([this]() { Close(); });
		Text.Subscribe([this](const auto&)
		{
			Status.set(TextStatus::Dirty);
		});
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

/**************/
/* PdfTabData */
/**************/

struct PdfTabData :public TabData
{
	ReactiveWStringProperty Path;
	ReactiveProperty<FLOAT> VScroll = 0.0f;
	ReactiveProperty<FLOAT> HScroll = 0.0f;
	ReactiveProperty<FLOAT> Scale = 1.0f;
	
	ReactiveCommand<void> OpenCommand;

	PdfTabData(const std::wstring& path = std::wstring())
		:TabData(), Path(path)
	{
		OpenCommand.Subscribe([this]() { Open(); });
	}

	virtual ~PdfTabData() = default;

	void Open() {}
	void Open(const std::wstring& path) {}

	friend void to_json(json& j, const PdfTabData& o)
	{
		to_json(j, static_cast<const TabData&>(o));
		j["Path"] = o.Path;
	}
	friend void from_json(const json& j, PdfTabData& o)
	{
		from_json(j, static_cast<TabData&>(o));
		j.at("Path").get_to(o.Path);
	}
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

	CBinding m_textBinding;
	CBinding m_textEncodingBinding;
	CBinding m_textPathBinding;
	CBinding m_pdfPathBinding;
	CBinding m_pdfScaleBinding;
	CBinding m_pdfVScrollBinding;
	CBinding m_pdfHScrollBinding;
	CBinding m_statusBinding;
	CBinding m_caretPosBinding;
	CBinding m_caretsBinding;
	CBinding m_openBinding;
	CBinding m_openAsBinding;
	CBinding m_saveBinding;
	CBinding m_saveAsBinding;


	std::unique_ptr<sigslot::scoped_connection> m_pTextPathConnection;
	std::unique_ptr<sigslot::scoped_connection> m_pPdfPathConnection;
	std::unique_ptr<sigslot::scoped_connection> m_pStatusConnection;
	std::unique_ptr<sigslot::scoped_connection> m_pCloseConnection;




public:
	CFilerTabGridView(CD2DWControl* pParentControl = nullptr,
		const std::shared_ptr<TabControlProperty>& spTabProp = nullptr, 
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProrperty = nullptr,
		const std::shared_ptr<EditorProperty>& spTextboxProp = nullptr,
		const std::shared_ptr<PDFEditorProperty>& spPdfViewProp = nullptr);
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
	SHAREDPTR_GETTER(CToDoGridView, ToDoGridView)

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
	void OnCommandAddToFavorite(const CommandEvent& e);
	void OnCommandOpenSameAsOther(const CommandEvent& e);

public:
	friend void to_json(json& j, const CFilerTabGridView& o)
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData );
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PdfTabData);

		to_json(j, static_cast<const CTabControl&>(o));
		j["FilerView"] = o.m_spFilerGridView;
	}

	friend void from_json(const json& j, CFilerTabGridView& o)
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);
		JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PdfTabData);

		from_json(j, static_cast<CTabControl&>(o));
		get_to(j, "FilerView", o.m_spFilerGridView);
	}
};

