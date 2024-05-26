#pragma once
#include "BindGridview.h"
#include "Launcher.h"

struct GridViewProperty;
class CLauncherProperty;

class CLauncherGridView : public CBindGridView<CLauncher>
{
public:
	CLauncherGridView(CD2DWControl* pParentControl = nullptr);
	
	virtual ~CLauncherGridView(void) = default;
	
	virtual void OnCreate(const CreateEvt& e) override;
	
	//void OnCellLButtonDblClk(const CellEventArgs& e);
	void OpenFavorites();
	void UpdateFilter() override {/*Do nothing*/}
	void Reload();

	virtual void MoveColumn(int indexTo, typename ColTag::SharedPtr spFrom) override;

public:
	template <class Archive>
	void serialize(Archive& ar)
	{
		//ar("FavoritesProperty",m_spFavoritesProp);
	}
	friend void to_json(json& j, const CLauncherGridView& o)
	{
	}
	friend void from_json(const json& j, CLauncherGridView& o)
	{
	}

};

