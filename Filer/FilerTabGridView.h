#pragma once
#include "MyWnd.h"
#include "MyRect.h"
#include "MyFont.h"
#include "ShellFileFactory.h"
#include "FilerView.h"
#include "FilerGridViewProperty.h"
#include "ToDoGridView.h"
#include "TabControl.h"
#include "KnownFolder.h"
#include <functional>
#include "JsonSerializer.h"
#include "encoding_type.h"
#include "FileStatus.h"
#include "ToDoDoc.h"
#include "PDFDoc.h"

#include "reactive_property.h"
#include "reactive_command.h"
#include "scoped_connections.h"

#include "FilerTabData.h"
#include "TextTabData.h"
#include "PDFTabData.h"
#include "ImageTabData.h"
#include "PreviewTabData.h"
#include "ToDoTabData.h"

class CFilerView;
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
	virtual void Measure(const CSizeF& availableSize) override;


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

	/**********/
	/* Setter */
	/**********/
	void SetOther(const std::shared_ptr<CFilerTabGridView>& spOther) { m_wpOther = spOther; }

	/**************/
	/* UI Message */
	/**************/
	//void OnCreate(const CreateEvt& e) override;
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

	void OnCommandAddToFavorite();
	void OnCommandOpenSameAsOther();

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

		j["FilerView"] = o.m_spFilerView;
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

		get_to(j, "FilerView", o.m_spFilerView);
		get_to(j, "ToDoView", o.m_spToDoGridView);
	}
};

