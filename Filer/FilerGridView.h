#pragma once
#include "FilerGridViewBase.h"

class CDirectoryWatcher;

class CFilerGridView:public CFilerGridViewBase
{
private:	
	//For DirectoryWatch
	std::shared_ptr<CDirectoryWatcher> m_spWatcher;
	std::wstring m_oldName;

	std::shared_ptr<CShellFolder> m_spFolder;
	std::shared_ptr<CShellFolder> m_spPreviousFolder;

	std::wstring m_initPath;

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
	bool m_bNewFile = false;

	//Remember past filter
	std::unordered_map<std::wstring, std::unordered_map<std::shared_ptr<CColumn>, std::wstring>> m_filterMap;

public:
	//Constructor
	CFilerGridView(std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp);
	virtual ~CFilerGridView(){}
	//getter
	std::shared_ptr<CShellFolder>& GetFolder() { return m_spFolder; }
	//signal
	boost::signals2::signal<void(std::shared_ptr<CShellFolder>&)> FolderChanged;

	virtual LRESULT OnCreate(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnDirectoryWatch(UINT uMsg,WPARAM wParam,LPARAM lParam,BOOL& bHandled);
	virtual LRESULT OnHandleMenuMsg(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);


	LRESULT OnCommandFind(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled) override;



	virtual void OnKeyDown(const KeyDownEvent& e)override;
	virtual void OnContextMenu(const ContextMenuEvent& e) override;
	virtual void OnBkGndLButtondDblClk(const LButtonDblClkEvent& e) override;

	virtual void OnCellLButtonDblClk(CellEventArgs& e);
	virtual void Reload();
	virtual void OpenFolder(std::shared_ptr<CShellFolder>& spFolder) override;

	std::wstring GetPath()const;
	void SetPath(const std::wstring& path);

	bool NewFolder();

	bool CopySelectedFilesTo(const CIDL& destIDL);
	bool CopyIncrementalSelectedFilesTo(const CIDL& destIDL);
	bool MoveSelectedFilesTo(const CIDL& destIDL);

	std::vector<LPITEMIDLIST> GetSelectedLastPIDLVector();
	std::vector<LPITEMIDLIST> GetSelectedAbsolutePIDLVector();

	virtual bool CutToClipboard() override;
	virtual bool CopyToClipboard() override;
	virtual bool PasteFromClipboard() override;
	virtual bool Delete() override;

	//Drag & Drop
	void Drag();

private:
	//Drag & Drop
	BOOL SetDragImage(CIDL firstIdl, CComPtr<IDragSourceHelper> pDragSourceHelper, IDataObject *pDataObject);
	void Dropped(IDataObject *pDataObj, DWORD dwEffect);
	void ShowShellContextMenu(HWND hWnd, CPoint ptScreen, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl, bool hasNew = false);
	bool InvokeNewShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf);
	bool InvokeNormalShellContextmenuCommand(HWND hWnd, LPCSTR lpVerb, CComPtr<IShellFolder> psf, std::vector<PITEMID_CHILD> vpIdl);


	//DirectoryWatch action
	void Added(const std::wstring& fileName);
	void Modified(const std::wstring& fileName);
	void Removed(const std::wstring& fileName);
	void Renamed(const std::wstring& oldName, const std::wstring& newName);
protected:
	void InsertDefaultRowColumn() override; 
public:
	FRIEND_SERIALIZER
	template <class Archive>
	void save(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(CColumn, CParentRowHeaderColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileNameColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileExtColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileSizeColumn);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileLastWriteColumn);

		std::vector<ColumnData> columns;
		for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
			columns.push_back(*iter);
		}
		ar("Columns", columns);
	}

	template <class Archive>
	void load(Archive& ar)
	{
		REGISTER_POLYMORPHIC_RELATION(CColumn, CParentRowHeaderColumn, this);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileNameColumn, this);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileExtColumn, this);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileSizeColumn, this, GetFilerGridViewPropPtr()->FileSizeArgsPtr);
		REGISTER_POLYMORPHIC_RELATION(CColumn, CFileLastWriteColumn, this, GetFilerGridViewPropPtr()->FileTimeArgsPtr);

		std::vector<ColumnData> columns;
		for (auto iter = m_columnAllDictionary.begin(); iter != m_columnAllDictionary.end(); ++iter) {
			columns.push_back(*iter);
		}
		ar("Columns", columns);

		if (m_columnAllDictionary.empty()) {

			for (auto& col : columns) {
				if (auto p = std::dynamic_pointer_cast<CFileNameColumn>(col.DataPtr)) {
					m_pNameColumn = p;
				} else if (auto p = std::dynamic_pointer_cast<CParentRowHeaderColumn>(col.DataPtr)) {
					m_pHeaderColumn = p;
				}
				InsertColumnNotify(col.Index, col.DataPtr, false);
			}
		}
	}



};