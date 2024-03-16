#pragma once

#include "BindGridView.h"
#include "IDL.h"
#include "Resource.h"

#include "FilerGridViewProperty.h"

#include "shobjidl.h"
#include "MyFriendSerializer.h"
#include "RowIndexColumn.h"
#include "FileColumn.h"
#include "FileSizeColumn.h"
#include "FileLastWriteColumn.h"
#include "MyXMLSerializer.h"
#include "ShellFile.h"
#include "ShellFolder.h"
#include "FileDragger.h"
#include "SheetEventArgs.h"
#include "ShowHideMenuItem.h"
#include "D2DWWindow.h"

#define WM_UAHMEASUREMENUITEM 0x0094

template<typename T>
class CFilerBindGridView :public CBindGridView<T>
{
	using base = CBindGridView<T>;
protected:
	//HeaderMenuItems
	std::vector<std::shared_ptr<CShowHideColumnMenuItem>> m_headerMenuItems;
public:
	//Constructor
	template<typename... TArgs>
	CFilerBindGridView(
		CD2DWControl* pParentControl = nullptr,
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProp = nullptr,
		TArgs... args)
		:base(pParentControl, std::static_pointer_cast<GridViewProperty>(spFilerGridViewProp), args...)
	{
		this->m_spItemDragger = std::make_shared<CFileDragger>();

		this->CellLButtonDblClk.connect(std::bind(&CFilerBindGridView::OnCellLButtonDblClk, this, std::placeholders::_1));
	}
	//Destructor
	virtual ~CFilerBindGridView() = default;

	//Getter
	std::shared_ptr<FilerGridViewProperty> GetFilerGridViewPropPtr() { return std::static_pointer_cast<FilerGridViewProperty>(this->m_spGridViewProp); }
	virtual bool HasSheetCell()override { return false; }
	virtual bool IsVirtualPage()override { return true; }

	/**********/
	/* Signal */
	/**********/
	std::function<void(CMenu&)> AddCustomContextMenu;
	std::function<bool(int, const std::shared_ptr<CShellFolder>&, const std::vector<std::shared_ptr<CShellFile>>&)> ExecCustomContextMenu;

	virtual void OnCellLButtonDblClk(const CellEventArgs& e)
	{
		auto pCell = e.CellPtr;
		if (auto spRow = dynamic_cast<CBindRow<T>*>(e.CellPtr->GetRowPtr())) {
			auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
			OpenFile(spFile);
		}
	}

	virtual void Open(const std::shared_ptr<CShellFile>& spFile)
	{
		if (auto spFolder = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
			OpenFolder(spFolder, false);
		} else {
			spFile->AddToRecentDocs();
			OpenFile(spFile);
		}
	}

	virtual void OpenFile(const std::shared_ptr<CShellFile>& spFile)
	{
		spFile->Open();
	}	
	
	virtual void OpenFolder(const std::shared_ptr<CShellFolder>& spFolder, bool isReload)
	{
		//auto pWnd = new CFilerGridView(std::static_pointer_cast<FilerGridViewProperty>(m_spGridViewProp));

		//pWnd->RegisterClassExArgument()
		//	.lpszClassName(L"CFilerGridViewWnd")
		//	.style(CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS)
		//	.hCursor(::LoadCursor(NULL, IDC_ARROW))
		//	.hbrBackground((HBRUSH)GetStockObject(GRAY_BRUSH));

		//pWnd->CreateWindowExArgument()
		//	.lpszClassName(_T("CFilerGridViewWnd"))
		//	.lpszWindowName(spFolder->GetDispName().c_str())
		//	.dwStyle(WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN)
		//	.dwExStyle(WS_EX_ACCEPTFILES)
		//	.hMenu(NULL);

		//pWnd->FolderChanged = [pWnd](std::shared_ptr<CShellFolder>& pFolder) {
		//	pWnd->SetWindowTextW(pFolder->GetDispName().c_str());};

		//pWnd->SetIsDeleteOnFinalMessage(true);

		//HWND hWnd = NULL;
		//if ((GetWindowLongPtr(GWL_STYLE) & WS_OVERLAPPEDWINDOW) == WS_OVERLAPPEDWINDOW) {
		//	hWnd = m_hWnd;
		//} else {
		//	hWnd = GetAncestorByStyle(WS_OVERLAPPEDWINDOW);
		//}

		//pWnd->CreateOnCenterOfParent(hWnd, CSize(300, 500));
		//pWnd->SetIsDeleteOnFinalMessage(true);
		//pWnd->OpenFolder(spFolder);
		//pWnd->ShowWindow(SW_SHOW);
		//pWnd->UpdateWindow();
	}

	index_vector<std::shared_ptr<CRow>>::const_iterator FindIfRowIterByFileNameExt(const std::wstring& fileNameExt)
	{
		return std::find_if(this->m_allRows.begin(), this->m_allRows.end(),
							[&](const std::shared_ptr<CRow>& rowPtr)->bool {
								if (auto p = std::dynamic_pointer_cast<CBindRow<T>>(rowPtr)) {
									return p->GetItem<std::shared_ptr<CShellFile>>()->GetDispName() == fileNameExt;
								} else {
									return false;
								}
							});
	}

	std::vector<std::shared_ptr<CShellFile>> GetAllFiles()
	{
		std::vector<std::shared_ptr<CShellFile>> files;
		for (const auto& rowPtr : this->m_visRows) {
			if (auto spRow = std::dynamic_pointer_cast<CBindRow<std::shared_ptr<CShellFile>>>(rowPtr)) {
				files.push_back(spRow->GetItem<std::shared_ptr<CShellFile>>());
			}
		}
		return files;
	}

	std::vector<std::shared_ptr<CShellFile>> GetSelectedFiles()
	{
		std::vector<std::shared_ptr<CShellFile>> files;
		for (const auto& rowPtr : this->m_visRows) {
			if (auto spRow = std::dynamic_pointer_cast<CBindRow<std::shared_ptr<CShellFile>>>(rowPtr); spRow && spRow->GetIsSelected()) {
				files.push_back(spRow->GetItem<std::shared_ptr<CShellFile>>());
			}
		}
		return files;
	}

	virtual bool CutToClipboard() { return false; }
	virtual bool CopyToClipboard() { return false; }
	virtual bool PasteFromClipboard() { return false; }
	virtual bool DeleteSelectedFiles() { return false; }

	/***********/
	/* Command */
	/***********/
	virtual void OnCommandCut(const CommandEvent& e) override
	{
		CutToClipboard();
	}

	virtual void OnCommandCopy(const CommandEvent& e) override
	{
		CopyToClipboard();
	}
	
	virtual void OnCommandPaste(const CommandEvent& e) override
	{
		PasteFromClipboard();
	}
	
	virtual void OnCommandDelete(const CommandEvent& e) override
	{
		DeleteSelectedFiles();
	}

	/*****************/
	/* State Message */
	/*****************/
	void Normal_KeyDown(const KeyDownEvent& e) override
	{
		this->m_keepEnsureVisibleFocusedCell = false;
		switch (e.Char) {
			case 'X':
				if (::GetAsyncKeyState(VK_CONTROL)) {
					CutToClipboard();
					(*e.HandledPtr) = true;
				}
				break;
			case 'C':
				if (::GetAsyncKeyState(VK_CONTROL)) {
					CopyToClipboard();
					(*e.HandledPtr) = true;
				}
				break;
			case 'V':
				if (::GetAsyncKeyState(VK_CONTROL)) {
					PasteFromClipboard();
					(*e.HandledPtr) = true;
				}
				break;
			case VK_DELETE:
				DeleteSelectedFiles();
					(*e.HandledPtr) = true;
				break;
			case VK_RETURN:
				{
					if (this->m_spCursorer->GetFocusedCell()) {
						if (auto spRow = dynamic_cast<CBindRow<T>*>(this->m_spCursorer->GetFocusedCell()->GetRowPtr())) {
							auto spFile = spRow->GetItem<std::shared_ptr<CShellFile>>();
							Open(spFile);
							(*e.HandledPtr) = true;
						}
					}
				}
				break;
			default:
				break;
		}
		if (!(*e.HandledPtr)) {
			CGridView::Normal_KeyDown(e);
		}

	};

	//bool Edit_Guard_KeyDown_WithNormal(const KeyDownEvent& e) override
	//{
	//	return false;
	//}

	//bool Edit_Guard_KeyDown_WithoutNormal(const KeyDownEvent& e) override
	//{
	//	return this->Edit_Guard_KeyDown(e);
	//}
};