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
	std::function<void(std::shared_ptr<CShellFile>&)> FileChosen;

public:
	CFavoritesGridView(
		CWnd* pWnd, 
		std::shared_ptr<GridViewProperty> spGridViewProp,
		std::shared_ptr<CFavoritesProperty> spFavoritesProp);
	virtual ~CFavoritesGridView(void) = default;

	std::shared_ptr<CFavoritesProperty>& GetFavoritesProp() { return m_spFavoritesProp; }

	virtual void OnCreate(const CreateEvent& e) override;
	
	void OnCellLButtonDblClk(CellEventArgs& e);
	void OpenFavorites();
	void FilterAll() override {/*Do nothing*/}
	void Reload();

	virtual void MoveRow(int indexTo, typename RowTag::SharedPtr spFrom) override;

public:
	template <class Archive>
	void serialize(Archive& ar)
	{
		ar("FavoritesProperty",m_spFavoritesProp);
	}
};

