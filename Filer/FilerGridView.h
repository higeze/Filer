#pragma once
#include "FilerBindGridView.h"
#include "JsonSerializer.h"

class CShellFile;
class CDirectoryWatcher;
struct CreateEvt;
struct DirectoryWatchEvent;

class CFilerGridView:public CFilerBindGridView<std::shared_ptr<CShellFile>>
{
	/*******************/
	/* Static variable */
	/*******************/
private:
	static CLIPFORMAT s_cf_shellidlist;
	static CLIPFORMAT s_cf_filecontents;
	static CLIPFORMAT s_cf_filegroupdescriptor;
	static CLIPFORMAT s_cf_renprivatemessages;

private:	
	//For DirectoryWatch
	std::shared_ptr<CDirectoryWatcher> m_spWatcher;

	std::shared_ptr<CShellFolder> m_spFolder;
	std::shared_ptr<CShellFolder> m_spPreviousFolder;

	//For Drag & Drop 
	CComPtr<IDropTarget> m_pDropTarget;
	CComPtr<IDropSource> m_pDropSource;
	CComPtr<IDragSourceHelper> m_pDragSourceHelper;

	//For Context Menu
	HMENU m_hNewMenu;

	CComPtr<IContextMenu2> m_pcmNew2;
	CComPtr<IContextMenu3> m_pcmNew3;

	CComPtr<IContextMenu2> m_pcm2;
	CComPtr<IContextMenu3> m_pcm3;

	//For New
	bool m_isNewFile = false;

	//Remember past filter
	std::unordered_map<std::wstring, std::unordered_map<std::shared_ptr<CColumn>, std::wstring>> m_filterMap;

public:
	//Constructor
	CFilerGridView(CD2DWControl* pParentControl = nullptr, const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp = nullptr);
	virtual ~CFilerGridView();
	//getter
	std::shared_ptr<CShellFolder>& GetFolder() { return m_spFolder; }
	//signal
	std::function<void(std::shared_ptr<CShellFolder>&)> FolderChanged;
	std::function<void(const std::wstring&)> StatusLog;

	/******************/
	/* Window Message */
	/******************/
	virtual LRESULT OnHandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/**************/
	/* UI Message */
	/**************/
	virtual void OnCreate(const CreateEvt& e)override;
	virtual void OnDirectoryWatch(const DirectoryWatchEvent& e);
	/************************/
	/* StateMachine Message */
	/************************/
	virtual void Normal_KeyDown(const KeyDownEvent& e)override;
	virtual void Normal_ContextMenu(const ContextMenuEvent& e) override;

	virtual void OnBkGndLButtondDblClk(const LButtonDblClkEvent& e) override;
	virtual void OnCellLButtonDblClk(const CellEventArgs& e) override;
	/***********/
	/* Command */
	/***********/
	virtual void OnCommandFind(const CommandEvent& e) override;

	/************/
	/* Function */
	/************/
	virtual void Reload();
	virtual void OpenFolder(const std::shared_ptr<CShellFolder>& spFolder) override;

	std::wstring GetPath()const;
	void SetPath(const std::wstring& path);

	bool NewFolder();

	bool CopySelectedFilesTo(const CIDL& destIDL);
	bool CopyIncrementalSelectedFilesTo(const CIDL& destIDL);
	bool MoveSelectedFilesTo(const CIDL& destIDL);
	virtual bool DeleteSelectedFiles() override;

	std::shared_ptr<CShellFile> GetFocusedFile();
	std::vector<std::shared_ptr<CShellFile>> GetSelectedFiles();

	std::vector<LPITEMIDLIST> GetSelectedLastPIDLVector();
	std::vector<LPITEMIDLIST> GetSelectedAbsolutePIDLVector();
	std::vector<CIDL> GetSelectedChildIDLVector();

	virtual bool CutToClipboard() override;
	virtual bool CopyToClipboard() override;
	virtual bool PasteFromClipboard() override;

	//Drag & Drop
	void Drag();

private:
	//Drag & Drop
	BOOL SetDragImage(CIDL firstIdl, CComPtr<IDragSourceHelper> pDragSourceHelper, IDataObject *pDataObject);
	bool IsDroppable(const std::vector<FORMATETC>& formats);
	void Dropped(IDataObject *pDataObj, DWORD dwEffect);
	void ShowShellContextMenu(HWND hWnd, CPoint ptScreen, const std::shared_ptr<CShellFolder>& folder, const std::vector<std::shared_ptr<CShellFile>>& files, bool hasNew = false);
	bool InvokeNewShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf);
	bool InvokeNormalShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf, std::vector<std::shared_ptr<CShellFile>> vpIdl);


	//DirectoryWatch action
	void Added(const std::wstring& fileName);
	void Modified(const std::wstring& fileName);
	void Removed(const std::wstring& fileName);
	void Renamed(const std::wstring& oldName, const std::wstring& newName);
public:
	FRIEND_SERIALIZER
	template <class Archive>
	void save(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(CColumn, CRowIndexColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileDispNameColumn<std::shared_ptr<CShellFile>>);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileDispExtColumn<std::shared_ptr<CShellFile>>);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileSizeColumn<std::shared_ptr<CShellFile>>);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileLastWriteColumn<std::shared_ptr<CShellFile>>);
	
		ar("Columns", static_cast<std::vector<std::shared_ptr<CColumn>>&>(m_allCols));
		ar("RowFrozenCount", m_frozenRowCount);
		ar("ColFrozenCount", m_frozenColumnCount);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(CColumn, CRowIndexColumn, this);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileDispNameColumn<std::shared_ptr<CShellFile>>, this, L"Name");
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileDispExtColumn<std::shared_ptr<CShellFile>>, this, L"Exe");
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileSizeColumn<std::shared_ptr<CShellFile>>, this, GetFilerGridViewPropPtr()->FileSizeArgsPtr);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileLastWriteColumn<std::shared_ptr<CShellFile>>, this, GetFilerGridViewPropPtr()->FileTimeArgsPtr);

		ar("Columns", static_cast<std::vector<std::shared_ptr<CColumn>>&>(m_allCols));
		for (auto& colPtr : m_allCols) {
			if (auto p = std::dynamic_pointer_cast<CFileDispNameColumn<std::shared_ptr<CShellFile>>>(colPtr)) {
				m_pNameColumn = p;
			} else if (auto p = std::dynamic_pointer_cast<CRowIndexColumn>(colPtr)) {
				m_pHeaderColumn = p;
			}
		}
		ar("RowFrozenCount", m_frozenRowCount);
		ar("ColFrozenCount", m_frozenColumnCount);
	}
	friend void to_json(json& j, const CFilerGridView& o)
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CRowIndexColumn);
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CFileThumbnailColumn<std::shared_ptr<CShellFile>>);
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CFileDispNameColumn<std::shared_ptr<CShellFile>>);
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CFileDispExtColumn<std::shared_ptr<CShellFile>>);
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CFileSizeColumn<std::shared_ptr<CShellFile>>);
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CFileLastWriteColumn<std::shared_ptr<CShellFile>>);
	
		j = json{
			{"Columns", o.m_allCols},
			{"RowFrozenCount", o.m_frozenRowCount},
			{"ColFrozenCount", o.m_frozenColumnCount}
		};
	}
	friend void from_json(const json& j, CFilerGridView& o)
	{
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CRowIndexColumn);// , this);
		json_make_shared_map.insert_or_assign(typeid(CRowIndexColumn).name(), [&]() { return std::make_shared<CRowIndexColumn>(&o); });
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CFileThumbnailColumn<std::shared_ptr<CShellFile>>);// , this, L"Name");
		json_make_shared_map.insert_or_assign(typeid(CFileThumbnailColumn<std::shared_ptr<CShellFile>>).name(), [&]() { return std::make_shared<CFileThumbnailColumn<std::shared_ptr<CShellFile>>>(&o); });
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CFileDispNameColumn<std::shared_ptr<CShellFile>>);// , this, L"Name");
		json_make_shared_map.insert_or_assign(typeid(CFileDispNameColumn<std::shared_ptr<CShellFile>>).name(), [&]() { return std::make_shared<CFileDispNameColumn<std::shared_ptr<CShellFile>>>(&o, L"Name"); });
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CFileDispExtColumn<std::shared_ptr<CShellFile>>);// , this, L"Exe");
		json_make_shared_map.insert_or_assign(typeid(CFileDispExtColumn<std::shared_ptr<CShellFile>>).name(), [&]() { return std::make_shared<CFileDispExtColumn<std::shared_ptr<CShellFile>>>(&o, L"Exe"); });
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CFileSizeColumn<std::shared_ptr<CShellFile>>);// , this, GetFilerGridViewPropPtr()->FileSizeArgsPtr);
		json_make_shared_map.insert_or_assign(typeid(CFileSizeColumn<std::shared_ptr<CShellFile>>).name(), [&]() { return std::make_shared<CFileSizeColumn<std::shared_ptr<CShellFile>>>(&o, o.GetFilerGridViewPropPtr()->FileSizeArgsPtr); });
		JSON_REGISTER_POLYMORPHIC_RELATION(CColumn, CFileLastWriteColumn<std::shared_ptr<CShellFile>>);// , this, GetFilerGridViewPropPtr()->FileTimeArgsPtr);
		json_make_shared_map.insert_or_assign(typeid(CFileLastWriteColumn<std::shared_ptr<CShellFile>>).name(), [&]() { return std::make_shared<CFileLastWriteColumn<std::shared_ptr<CShellFile>>>(&o, o.GetFilerGridViewPropPtr()->FileTimeArgsPtr); });

		j.at("Columns").get_to(static_cast<std::vector<std::shared_ptr<CColumn>>&>(o.m_allCols));



		for (auto& colPtr : o.m_allCols) {
			if (auto p = std::dynamic_pointer_cast<CFileDispNameColumn<std::shared_ptr<CShellFile>>>(colPtr)) {
				o.m_pNameColumn = p;
			} else if (auto p = std::dynamic_pointer_cast<CRowIndexColumn>(colPtr)) {
				o.m_pHeaderColumn = p;
			}
		}



		j.at("RowFrozenCount").get_to(o.m_frozenRowCount);
		j.at("ColFrozenCount").get_to(o.m_frozenColumnCount);
	}
};

