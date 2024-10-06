#pragma once
#include "MyWnd.h"
#include "MyRect.h"
#include "MyFont.h"
#include "ShellFileFactory.h"
#include "FilerView.h"
#include "ToDoGridView.h"
#include "TabControl.h"
#include "KnownFolder.h"
#include <functional>
#include "JsonSerializer.h"
#include "encoding_type.h"
#include "FileStatus.h"

#include "reactive_property.h"
#include "reactive_command.h"
#include "scoped_connections.h"

class CD2DWWindow;
class CShellFolder;
class CPDFEditor;
class CPreviewControl;
class CImageEditor;
class CThreadMonitorView;


/*************/
/* CFilerTab */
/*************/
class CFilerTabGridView :public CTabControl
{
private:
	static std::vector<std::wstring> imageExts;
	static std::vector<std::wstring> previewExts;
	bool m_isPreview = false;
	std::weak_ptr<CFilerTabGridView> m_wpOther;
public:
	CFilerTabGridView(CD2DWControl* pParentControl = nullptr);
	virtual ~CFilerTabGridView();

	//MeasureArrange
	virtual CSizeF MeasureOverride(const CSizeF& availableSize) override;


	scoped_connections m_filerConnections;
	scoped_connections m_textConnections;
	scoped_connections m_pdfConnections;
	scoped_connections m_imageConnections;
	scoped_connections m_todoConnections;
	scoped_connections m_prevConnections;
	/****************/
	/* Pure Virtual */
	/****************/

	/**********/
	/* Getter */
	/**********/

	SHAREDPTR_GETTER(CFilerView, FilerView)
	SHAREDPTR_GETTER(CEditor, TextView)
	SHAREDPTR_GETTER(CPDFEditor, PdfView)
	SHAREDPTR_GETTER(CImageEditor, ImageView)
	SHAREDPTR_GETTER(CToDoGridView, ToDoGridView)
	SHAREDPTR_GETTER(CPreviewControl, PreviewControl)
	SHAREDPTR_GETTER(CThreadMonitorView, ThreadMonitorView)


	/**********/
	/* Setter */
	/**********/
	void SetOther(const std::shared_ptr<CFilerTabGridView>& spOther) { m_wpOther = spOther; }

	/**************/
	/* UI Message */
	/**************/
	void OnCreate(const CreateEvt& e) override;
	void OnKeyDown(const KeyDownEvent& e) override;
	void OnContextMenu(const ContextMenuEvent& e) override;

	/***********/
	/* Command */
	/***********/
	void OnCommandNewTab() override { return OnCommandNewFilerTab(); }
	void OnCommandNewFilerTab();
	void OnCommandNewToDoTab();
	void OnCommandNewTextTab();
	void OnCommandNewPdfTab();
	void OnCommandNewImageTab();
	void OnCommandNewPreviewTab();
	void OnCommandNewThreadMonitorTab();

	void OnCommandAddToFavorite();
	void OnCommandOpenSameAsOther();

public:

	friend void to_json(json& j, const CFilerTabGridView& o)
	{
		to_json(j, static_cast<const CTabControl&>(o));

		json_safe_to(j, "FilerView", o.m_spFilerView);
		json_safe_to(j, "ToDoView", o.m_spToDoGridView);
	}

	friend void from_json(const json& j, CFilerTabGridView& o)
	{
		from_json(j, static_cast<CTabControl&>(o));

		json_safe_from(j, "FilerView", o.m_spFilerView);
		json_safe_from(j, "ToDoView", o.m_spToDoGridView);
	}
};

JSON_ENTRY_TYPE(CD2DWControl, CFilerTabGridView)




