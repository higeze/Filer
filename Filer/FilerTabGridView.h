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


class CFilerGridView;
struct FilerGridViewProperty;
class CToDoGridView;
struct GridViewProperty;
class CTextboxWnd;
class CShellFolder;
class CFont;

struct TabData
{
	TabData(){}
	virtual ~TabData() = default;

	virtual std::wstring GetItemText() = 0;

	//In case of REGISTER_POLYMORPHIC_RELATION, Base and Derived class have to be same function structure
	template<class Archive>
	void save(Archive& ar){}
	template<class Archive>
	void load(Archive& ar){}
};

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

	virtual std::wstring GetItemText() override
	{
		if (FolderPtr) {
			return FolderPtr->GetFileNameWithoutExt().c_str();
		} else {
			return L"nullptr";
		}
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

struct ToDoTabData:public TabData
{
	std::wstring Path;

	ToDoTabData(const std::wstring& path = std::wstring())
		:TabData(), Path(path){}

	virtual ~ToDoTabData() = default;

	virtual std::wstring GetItemText() override
	{
		return ::PathFindFileName(Path.c_str());
	}

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

struct TextTabData :public TabData
{
	std::wstring Path;
	bool IsSaved;

	TextTabData(const std::wstring& path = std::wstring())
		:TabData(), Path(path), IsSaved(true)
	{
	}

	virtual ~TextTabData() = default;

	virtual std::wstring GetItemText() override
	{
		return std::wstring(IsSaved?L"":L"*") + ::PathFindFileName(Path.c_str());
	}

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

class CFilerTabGridView :public CTabCtrl
{
private:
	CWnd* m_pParentWnd;
	CUniqueIDFactory m_uniqueIDFactory;

	observable_vector<std::shared_ptr<TabData>> m_itemsSource;
	std::unordered_map<std::string, std::function<std::shared_ptr<CWnd>(const std::shared_ptr<TabData>&)>> m_itemsTemplate;
	observable<int> m_selectedIndex = -1;
	int m_contextMenuTabIndex;

	std::shared_ptr<CWnd> m_curView;
	std::shared_ptr<FilerGridViewProperty> m_spFilerGridViewProp;

	CFont m_font;
public:
	CFilerTabGridView(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProrperty);
	virtual ~CFilerTabGridView();

	//Getter Setter
	std::function<std::shared_ptr<CFilerGridView>()> GetFilerGridViewPtr;
	std::function<std::shared_ptr<CToDoGridView>()> GetToDoGridViewPtr;
	std::function<std::shared_ptr<CTextboxWnd>()> GetTextViewPtr;

	observable_vector<std::shared_ptr<TabData>>& GetItemsSource(){ return m_itemsSource; }
	int GetSelectedIndex()const{ return m_selectedIndex.get(); }

	void SetContextMenuTabIndex(int index) { m_contextMenuTabIndex = index; }
	std::shared_ptr<CWnd> GetCurView() { return m_curView; }
	void SetParentWnd(CWnd* pParentWnd) { m_pParentWnd = pParentWnd; }

	//Message
	LRESULT OnCreate(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnClose(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	//LRESULT OnDestroy(UINT uiMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	LRESULT OnKeyDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

	LRESULT OnCommandNewFilerTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandNewToDoTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandNewTextTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandCloneTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandCloseTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandCloseAllButThisTab(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandAddToFavorite(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);
	LRESULT OnCommandOpenSameAsOther(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled);

	LRESULT OnNotifyTabSelChanging(int id, LPNMHDR, BOOL& bHandled);
	LRESULT OnNotifyTabSelChange(int id, LPNMHDR, BOOL& bHandled);
	LRESULT OnNotifyTabLClick(int id, LPNMHDR, BOOL& bHandled);
	LRESULT OnNotifyTabRClick(int id, LPNMHDR, BOOL& bHandled);

private:

	CRect GetTabRect();
	//void AddNewTab(const std::wstring& path);
	//void AddNewTab(const std::shared_ptr<CShellFile>& spFile);
public:
	FRIEND_SERIALIZER
	template <class Archive>
	void save(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData );
		REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);

		ar("ItemsSource", static_cast<std::vector<std::shared_ptr<TabData>>&>(m_itemsSource));
		ar("SelectedIndex", m_selectedIndex);
		ar("FilerView", GetFilerGridViewPtr());
	}

	template <class Archive>
	void load(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(TabData, FilerTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, ToDoTabData);
		REGISTER_POLYMORPHIC_RELATION(TabData, TextTabData);

		ar("ItemsSource", static_cast<std::vector<std::shared_ptr<TabData>>&>(m_itemsSource));
		ar("SelectedIndex", m_selectedIndex);
		ar("FilerView", GetFilerGridViewPtr(), m_spFilerGridViewProp);
	}
};
