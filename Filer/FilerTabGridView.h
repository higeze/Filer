#pragma once
#include "MyWnd.h"
#include "MyXmlSerializer.h"
#include "UniqueIDFactory.h"
#include "MyFriendSerializer.h"
#include "MyRect.h"
#include "MyFont.h"
#include "ShellFolder.h"
#include "ShellFileFactory.h"
#include "FilerGridView.h"
#include "FilerGridViewProperty.h"
#include "observable.h"
#include "TabControl.h"

class CFilerGridView;
struct FilerGridViewProperty;
struct GridViewProperty;
class CToDoGridView;
class CD2DWWindow;
class CShellFolder;

/***************/
/* FilerTabData */
/***************/
struct FilerTabData:public TabData
{
	std::wstring Path;
	std::shared_ptr<CShellFolder> FolderPtr;

	FilerTabData()
		:TabData(){ }

	FilerTabData(const std::wstring& path)
		:TabData(), Path(path)
	{
		if (!Path.empty()) {
			auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(path);
			FolderPtr = std::dynamic_pointer_cast<CShellFolder>(spFile);
		}
	}
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
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(Path);
		FolderPtr = std::dynamic_pointer_cast<CShellFolder>(spFile);
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
};

/***************/
/* TextTabData */
/***************/
struct TextTabData :public TabData
{
	std::wstring Path;
	bool IsSaved;

	TextTabData(const std::wstring& path = std::wstring())
		:TabData(), Path(path), IsSaved(true)
	{
	}

	virtual ~TextTabData() = default;

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
};

/*************/
/* CFilerTab */
/*************/
class CFilerTabGridView :public CTabControl
{
private:
	CUniqueIDFactory m_uniqueIDFactory;

	std::shared_ptr<FilerGridViewProperty> m_spFilerGridViewProp;
	std::shared_ptr<TextEditorProperty> m_spTextEditorProp;
public:
	CFilerTabGridView(CD2DWControl* pParentControl, std::shared_ptr<TabControlProperty> spTabProp, std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProrperty, std::shared_ptr<TextEditorProperty>& spTextboxProp);
	virtual ~CFilerTabGridView();

	/***********/
	/* Closure */
	/***********/
	std::function<std::shared_ptr<CFilerGridView>()> GetFilerGridViewPtr;
	std::function<std::shared_ptr<CToDoGridView>()> GetToDoGridViewPtr;
	std::function<std::shared_ptr<CTextEditor>()> GetTextViewPtr;

	/**************/
	/* UI Message */
	/**************/
	void OnCreate(const CreateEvent& e) override;
	void OnKeyDown(const KeyDownEvent& e) override;
	void OnContextMenu(const ContextMenuEvent& e) override;

	/***********/
	/* Command */
	/***********/
	void OnCommandNewFilerTab(const CommandEvent& e);
	void OnCommandNewToDoTab(const CommandEvent& e);
	void OnCommandNewTextTab(const CommandEvent& e);
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

		CTabControl::save(ar);
		ar("FilerView", GetFilerGridViewPtr());
	}

	template <class Archive>
	void load(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);

		CTabControl::load(ar);
		ar("FilerView", GetFilerGridViewPtr(), this, m_spFilerGridViewProp);
	}
};
