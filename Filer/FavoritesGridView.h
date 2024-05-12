#pragma once
#include "BindGridview.h"
#include "Favorite.h"

struct GridViewProperty;
class CFavoritesProperty;
class CShellFile;
class CFilerWnd;

class CFavoritesGridView : public CBindGridView<std::shared_ptr<CFavorite>>
{
private:
	std::shared_ptr<CFavoritesProperty> m_spFavoritesProp;
public:
	std::function<void(const std::shared_ptr<CShellFile>&)> FileChosen;

public:
	CFavoritesGridView( CD2DWControl* pParentControl = nullptr);
	virtual ~CFavoritesGridView(void) = default;

	virtual void OnCreate(const CreateEvt& e) override;
	virtual void OnKeyDown(const KeyDownEvent& e) override;
	
	void OnCellLButtonDblClk(const CellEventArgs& e);
	void OpenFavorites();
	void UpdateFilter() override {/*Do nothing*/}
	void Reload();

	virtual void MoveRow(int indexTo, typename RowTag::SharedPtr spFrom) override;

public:
	template <class Archive>
	void serialize(Archive& ar)
	{
		//ar("FavoritesProperty",m_spFavoritesProp);
	}

	friend void to_json(json& j, const CFavoritesGridView& o)
	{
	}
	friend void from_json(const json& j, CFavoritesGridView& o)
	{
	}
};

