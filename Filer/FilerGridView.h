#pragma once
#include "FilerBindGridView.h"
#include "FileColumn.h"
#include "reactive_property.h"
#include "reactive_string.h"
#include "JsonSerializer.h"
#include "ShellContextMenu.h"

class CShellFile;
class CDirectoryWatcher;
struct CreateEvt;
struct DirectoryWatchEvent;
struct ExeExtension;

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

	std::shared_ptr<CShellFolder> m_spPreviousFolder;

	//For Drag & Drop 
	CComPtr<IDropTarget> m_pDropTarget;
	CComPtr<IDropSource> m_pDropSource;
	CComPtr<IDragSourceHelper> m_pDragSourceHelper;

	//For New
	bool m_isNewFile = false;

	//Remember past filter
	std::unordered_map<std::wstring, std::unordered_map<std::shared_ptr<CColumn>, std::wstring>> m_filterMap;
public:
	std::shared_ptr<int> Dummy;
	reactive_property_ptr<std::shared_ptr<CShellFolder>> Folder;
	reactive_wstring_ptr StatusLog;
	reactive_vector_ptr<ExeExtension> ExeExtensions;

	DECLARE_LAZY_GETTER_UNCONST(CShellContextMenu, FileContextMenu)
	DECLARE_LAZY_GETTER_UNCONST(CShellContextMenu, FolderContextMenu)

public:
	//Constructor
	CFilerGridView(CD2DWControl* pParentControl = nullptr);
	virtual ~CFilerGridView();

	virtual void Measure(const CSizeF& availableSize) override { CFilerBindGridView::Measure(availableSize); }


	/******************/
	/* Window Message */
	/******************/
	//virtual LRESULT OnHandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
	/**************/
	/* UI Message */
	/**************/
	virtual void OnCreate(const CreateEvt& e)override;
	virtual void OnDirectoryWatch(const DirectoryWatchEvent& e);
	virtual void OnMouseWheel(const MouseWheelEvent& e)override;
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
	virtual void Open(const std::shared_ptr<CShellFile>& spFile) override;
	virtual void OpenFolder(const std::shared_ptr<CShellFolder>& spFolder, bool isReload) override;

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
	//bool InvokeNewShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, const std::shared_ptr<CShellFolder>& folder);
	//bool InvokeNormalShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, const std::shared_ptr<CShellFolder>& folder, const std::vector<std::shared_ptr<CShellFile>>& files);


	//DirectoryWatch action
	void Added(const std::wstring& fileName);
	void Modified(const std::wstring& fileName);
	void Removed(const std::wstring& fileName);
	void Renamed(const std::wstring& oldName, const std::wstring& newName);

public:
	friend void to_json(json& j, const CFilerGridView& o)
	{
		json_safe_to(j, "Columns", o.m_allCols);
		json_safe_to(j, "RowFrozenCount", o.m_frozenRowCount);
		json_safe_to(j, "ColFrozenCount", o.m_frozenColumnCount);

	}
	friend void from_json(const json& j, CFilerGridView& o)
	{
		json_safe_from(j, "Columns", static_cast<std::vector<std::shared_ptr<CColumn>>&>(o.m_allCols));
		json_safe_from(j, "RowFrozenCount", o.m_frozenRowCount);
		json_safe_from(j, "ColFrozenCount", o.m_frozenColumnCount);

		for (auto& col : o.m_allCols) {
			col->SetSheetPtr(&o);
		}
	}
};

JSON_ENTRY_TYPE(CColumn, CFileNameColumn<std::shared_ptr<CShellFile>>)
JSON_ENTRY_TYPE(CColumn, CFileDispExtColumn<std::shared_ptr<CShellFile>>)
JSON_ENTRY_TYPE(CColumn, CFileSizeColumn<std::shared_ptr<CShellFile>>)
JSON_ENTRY_TYPE(CColumn, CFileLastWriteColumn<std::shared_ptr<CShellFile>>)

