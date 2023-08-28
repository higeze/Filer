#pragma once
#include "MyWnd.h"
#include "MyRect.h"
#include "MyFont.h"
#include "ShellFileFactory.h"
#include "FilerGridView.h"
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

#include "FilerTabData.h"
#include "TextTabData.h"
#include "PDFTabData.h"
#include "ImageTabData.h"
#include "PreviewTabData.h"
#include "ToDoTabData.h"

class CFilerGridView;
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
	std::shared_ptr<FilerGridViewProperty> m_spFilerGridViewProp;
	std::shared_ptr<EditorProperty> m_spEditorProp;
	std::shared_ptr<PDFEditorProperty> m_spPdfEditorProp;
	std::shared_ptr<ImageEditorProperty> m_spImageEditorProp;
	std::shared_ptr<PreviewControlProperty> m_spPreviewControlProp;

public:
	CFilerTabGridView(CD2DWControl* pParentControl = nullptr,
		const std::shared_ptr<TabControlProperty>& spTabProp = nullptr, 
		const std::shared_ptr<FilerGridViewProperty>& spFilerGridViewProrperty = nullptr,
		const std::shared_ptr<EditorProperty>& spTextboxProp = nullptr,
		const std::shared_ptr<PDFEditorProperty>& spPdfViewProp = nullptr,
		const std::shared_ptr<ImageEditorProperty>& spImageEditorProp = nullptr,
		const std::shared_ptr<PreviewControlProperty>& spPreviewControlProp = nullptr);
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
	SHAREDPTR_GETTER(CImageEditor, ImageView)
	SHAREDPTR_GETTER(CToDoGridView, ToDoGridView)
	SHAREDPTR_GETTER(CPreviewControl, PreviewControl)

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
	void OnCommandNewImageTab(const CommandEvent& e);
	void OnCommandNewPreviewTab(const CommandEvent& e);

	void OnCommandAddToFavorite(const CommandEvent& e);
	void OnCommandOpenSameAsOther(const CommandEvent& e);

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

		j["FilerView"] = o.m_spFilerGridView;
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

		get_to(j, "FilerView", o.m_spFilerGridView);
		get_to(j, "ToDoView", o.m_spToDoGridView);
	}
};

