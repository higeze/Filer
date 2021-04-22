#pragma once
#include "MyWnd.h"
#include "MyXmlSerializer.h"
#include "MyFriendSerializer.h"
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

class CFilerGridView;
struct FilerGridViewProperty;
struct GridViewProperty;
class CToDoGridView;
class CD2DWWindow;
class CShellFolder;
class CPdfView;
struct PdfViewProperty;

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

	template<class Archive>
	void save(Archive& ar)
	{
		Path = FolderPtr->GetPath();
		ar("Path", Path);
	}

	template<class Archive>
	void load(Archive& ar)
	{
		ar("Path", Path);
		if (!Path.empty()) {
			auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(Path);
			if (auto sp = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
				FolderPtr = sp;
			} else {
				FolderPtr = CKnownFolderManager::GetInstance()->GetDesktopFolder();
				Path = FolderPtr->GetPath();
			}
		}
	}
    friend void to_json(json& j, const FilerTabData& o);
    friend void from_json(const json& j, FilerTabData& o);
};
void to_json(json& j, const FilerTabData& o)
{
	to_json(j, static_cast<const TabData&>(o));
	j["Path"] = o.FolderPtr->GetPath();
}
void from_json(const json& j, FilerTabData& o)
{
	from_json(j, static_cast<FilerTabData&>(o));
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

/***************/
/* ToDoTabData */
/***************/
struct ToDoTabData:public TabData
{
	std::wstring Path;

	ToDoTabData(const std::wstring& path = std::wstring())
		:TabData(), Path(path){}

	virtual ~ToDoTabData() = default;

	template<class Archive>
	void save(Archive& ar)
	{
		ar("Path", Path);
	}
	template<class Archive>
	void load(Archive& ar)
	{
		ar("Path", Path);
	}

	friend void to_json(json& j, const ToDoTabData& o);
    friend void from_json(const json& j, ToDoTabData& o);
};
void to_json(json& j, const ToDoTabData& o)
{
	to_json(j, static_cast<const TabData&>(o));
	j["Path"] = o.Path;
}
void from_json(const json& j, ToDoTabData& o)
{
	from_json(j, static_cast<TabData&>(o));
	j.at("Path").get_to(o.Path);
}

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
	ReactiveProperty<CPointF> CaretPos;
	ReactiveTupleProperty<int, int, int, int, int> Carets;
	ReactiveProperty<TextStatus> Status;

	ReactiveCommand<void> OpenCommand;
	ReactiveCommand<void> SaveCommand;

	TextTabData(const std::wstring& path = std::wstring())
		:TabData(), Path(path), Text(), Status(TextStatus::None), Carets(0,0,0,0,0)
	{
		OpenCommand.Subscribe([this]() { Open(); });
		SaveCommand.Subscribe([this]() { Save(); });
		//CloseCommand.Subscribe([this]() { Close(); });
		Text.Subscribe([this](const auto&)
		{
			Status.set(TextStatus::Dirty);
		});
	}

	virtual ~TextTabData() = default;

	void Open();
	void Open(const std::wstring& path);

	void Save();
	void Save(const std::wstring& path);

	template<class Archive>
	void save(Archive & ar)
	{
		ar("Path", Path);
	}
	template<class Archive>
	void load(Archive & ar)
	{
		ar("Path", Path);
	}
	friend void to_json(json& j, const TextTabData& o);
    friend void from_json(const json& j, TextTabData& o);
};
void to_json(json& j, const TextTabData& o)
{
	to_json(j, static_cast<const TabData&>(o));
	j["Path"] = o.Path;
}
void from_json(const json& j, TextTabData& o)
{
	from_json(j, static_cast<TabData&>(o));
	j.at("Path").get_to(o.Path);
}

/**************/
/* PdfTabData */
/**************/

struct PdfTabData :public TabData
{
	ReactiveWStringProperty Path;
	ReactiveProperty<FLOAT> Scale;
	
	ReactiveCommand<void> OpenCommand;

	PdfTabData(const std::wstring& path = std::wstring())
		:TabData(), Path(path)
	{
		OpenCommand.Subscribe([this]() { Open(); });
	}

	virtual ~PdfTabData() = default;

	void Open() {}
	void Open(const std::wstring& path) {}

	template<class Archive>
	void save(Archive & ar)
	{
		ar("Path", Path);
	}
	template<class Archive>
	void load(Archive & ar)
	{
		ar("Path", Path);
	}
	friend void to_json(json& j, const PdfTabData& o);
    friend void from_json(const json& j, PdfTabData& o);
};
void to_json(json& j, const PdfTabData& o)
{
	to_json(j, static_cast<const TabData&>(o));
	j["Path"] = o.Path;
}
void from_json(const json& j, PdfTabData& o)
{
	from_json(j, static_cast<TabData&>(o));
	j.at("Path").get_to(o.Path);
}


/*************/
/* CFilerTab */
/*************/
class CFilerTabGridView :public CTabControl
{
private:
	std::shared_ptr<FilerGridViewProperty> m_spFilerGridViewProp;
	std::shared_ptr<TextEditorProperty> m_spTextEditorProp;
	std::shared_ptr<PdfViewProperty> m_spPdfViewProp;

	std::unique_ptr<CBinding<std::wstring>> m_pTextBinding;
	std::unique_ptr<CBinding<std::wstring>> m_pTextPathBinding;
	std::unique_ptr<CBinding<std::wstring>> m_pPdfPathBinding;
	std::unique_ptr<CBinding<FLOAT>> m_pPdfScaleBinding;
	std::unique_ptr<CBinding<bool>> m_pStatusBinding;
	std::unique_ptr<CBinding<CPointF>> m_pCaretPosBinding;
	std::unique_ptr<CBinding<std::tuple<int, int, int, int, int>>> m_pCaretsBinding;
	std::unique_ptr<CBinding<void>> m_pSaveBinding;
	std::unique_ptr<CBinding<void>> m_pOpenBinding;


	std::unique_ptr<sigslot::scoped_connection> m_pTextPathConnection;
	std::unique_ptr<sigslot::scoped_connection> m_pPdfPathConnection;
	std::unique_ptr<sigslot::scoped_connection> m_pStatusConnection;
	std::unique_ptr<sigslot::scoped_connection> m_pCloseConnection;




public:
	CFilerTabGridView(CD2DWControl* pParentControl = nullptr,
		const std::shared_ptr<TabControlProperty>& spTabProp = nullptr, 
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProrperty = nullptr,
		const std::shared_ptr<TextEditorProperty>& spTextboxProp = nullptr,
		const std::shared_ptr<PdfViewProperty>& spPdfViewProp = nullptr);
	virtual ~CFilerTabGridView();

	/***********/
	/* Closure */
	/***********/
	std::function<std::shared_ptr<CFilerGridView>()> GetFilerGridViewPtr;
	std::function<std::shared_ptr<CToDoGridView>()> GetToDoGridViewPtr;
	std::function<std::shared_ptr<CTextEditor>()> GetTextViewPtr;
	std::function<std::shared_ptr<CPdfView>()> GetPdfViewPtr;

	/**************/
	/* UI Message */
	/**************/
	void OnCreate(const CreateEvt& e) override;
	void OnKeyDown(const KeyDownEvent& e) override;
	void OnContextMenu(const ContextMenuEvent& e) override;

	/***********/
	/* Command */
	/***********/
	void OnCommandNewFilerTab(const CommandEvent& e);
	void OnCommandNewToDoTab(const CommandEvent& e);
	void OnCommandNewTextTab(const CommandEvent& e);
	void OnCommandNewPdfTab(const CommandEvent& e);
	void OnCommandAddToFavorite(const CommandEvent& e);
	void OnCommandOpenSameAsOther(const CommandEvent& e);

public:
	FRIEND_SERIALIZER
	template <class Archive>
	void save(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData );
		REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, PdfTabData);

		CTabControl::save(ar);
		auto spGrid = GetFilerGridViewPtr();
		ar("FilerView", spGrid);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, PdfTabData);

		CTabControl::load(ar);
		auto spGrid = GetFilerGridViewPtr();
		ar("FilerView", spGrid, this, m_spFilerGridViewProp);
	}

    friend void to_json(json& j, const CFilerTabGridView& o);
    friend void from_json(const json& j, CFilerTabGridView& o);
};

void to_json(json& j, const CFilerTabGridView& o)
{
	JSON_REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
	JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData );
	JSON_REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);
	JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PdfTabData);

	to_json(j, static_cast<const CTabControl&>(o));
	j["FilerView"] = o.GetFilerGridViewPtr();
}

void from_json(const json& j, CFilerTabGridView& o)
{
	JSON_REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
	JSON_REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData);
	JSON_REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);
	JSON_REGISTER_POLYMORPHIC_RELATION(TabData, PdfTabData);

	from_json(j, static_cast<CTabControl&>(o));
	auto spGrid = o.GetFilerGridViewPtr();
	j.at("FilerView").get_to(spGrid);
}
