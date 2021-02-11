#pragma once
#include "BindGridview.h"
#include "Favorite.h"

struct GridViewProperty;
class CLauncherProperty;

class CLauncherGridView : public CBindGridView<std::shared_ptr<CFavorite>>
{
private:
	std::shared_ptr<CLauncherProperty> m_spLauncherProp;

public:
	CLauncherGridView(
		CD2DWControl* pParentControl, 
		std::shared_ptr<GridViewProperty> spGridViewProp,
		std::shared_ptr<CLauncherProperty> spLauncherProp);
	
	virtual ~CLauncherGridView(void) = default;

	std::shared_ptr<CLauncherProperty>& GetLauncherProp() { return m_spLauncherProp; }
	
	bool GetIsFocusable()const { return false; }

	virtual void OnCreate(const CreateEvt& e) override;
	
	void OnCellLButtonDblClk(const CellEventArgs& e);
	void OpenFavorites();
	void FilterAll() override {/*Do nothing*/}
	void Reload();

	virtual void MoveColumn(int indexTo, typename ColTag::SharedPtr spFrom) override;

public:
	template <class Archive>
	void serialize(Archive& ar)
	{
		//ar("FavoritesProperty",m_spFavoritesProp);
	}
};

