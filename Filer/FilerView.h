#pragma once
#include "D2DWControl.h"
#include "FilerGridView.h"
#include "JsonSerializer.h"
#include "getter_macro.h"

class CTextBox;
class CButton;
class CFavoritesGridView;
class CFilerGridView;
class CShellFile;

class CRecentFolderGridView :public CFilerBindGridView<std::shared_ptr<CShellFile>>
{
public:
	CRecentFolderGridView(CD2DWControl* pParentControl = nullptr);
	virtual void OnPaint(const PaintEvent& e) override;
};

class CFilerView:public CD2DWControl
{
public:
	SHAREDPTR_GETTER(CTextBox, TextBox)
	SHAREDPTR_GETTER(CButton, RecentButton)
	SHAREDPTR_GETTER(CFavoritesGridView, FavoriteGrid)
	SHAREDPTR_GETTER(CFilerGridView, FileGrid)
public:
	//Default
	CFilerView(CD2DWControl* pParentControl = nullptr);
	virtual ~CFilerView();
	//Event
	virtual void OnCreate(const CreateEvt& e) override;
	//virtual void OnPaint(const PaintEvent& e) override;
	virtual void OnRect(const RectEvent& e) override;

	virtual void Measure(const CSizeF& availableSize) override;
	//virtual CSizeF DesiredSize() const { return m_size; }
	//virtual CSizeF RenderSize() const { return CSizeF(m_size.width - Margin->Width(), m_size.height - Margin->Height()); }
	virtual void Arrange(const CRectF& rc) override;

public:
    template<class Archive>
    void save(Archive & archive) const
    {
		archive(
			cereal::base_class<CD2DWControl>(this),
			cereal::make_nvp("FileGrid", m_spFileGrid));
    }
    template<class Archive>
    void load(Archive & archive)
    {
		archive(
			cereal::base_class<CD2DWControl>(this),
			cereal::make_nvp("FileGrid", m_spFileGrid));
    }
	//friend void to_json(json& j, const CFilerView& o)
	//{
	//	j["GridView"] = o.m_spFileGrid;
	//}

	//friend void from_json(const json& j, CFilerView& o)
	//{
	//	get_to(j, "GridView", o.m_spFileGrid);
	//}
};

CEREAL_REGISTER_TYPE(CFilerView);