#pragma once
#include "Favorite.h"
#include "KnownFolder.h"
#include "ReactiveProperty.h"

class CFavoritesProperty
{
private:
	std::shared_ptr<ReactiveVectorProperty<std::tuple<std::shared_ptr<CFavorite>>>> m_spFavorites;

public:
	CFavoritesProperty():m_spFavorites(std::make_shared<ReactiveVectorProperty<std::tuple<std::shared_ptr<CFavorite>>>>())
	{
		m_spFavorites->push_back(std::make_tuple(std::make_shared<CFavorite>(CKnownFolderManager::GetInstance()->GetDesktopFolder()->GetPath(),L"DT")));
	};
	~CFavoritesProperty(){};

	std::shared_ptr<ReactiveVectorProperty<std::tuple<std::shared_ptr<CFavorite>>>>& GetFavoritesPtr() { return m_spFavorites; }
	ReactiveVectorProperty<std::tuple<std::shared_ptr<CFavorite>>>& GetFavorites(){return *m_spFavorites;}

    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("Favorites", m_spFavorites);
    }

	friend void to_json(json& j, const CFavoritesProperty& o);
    friend void from_json(const json& j, CFavoritesProperty& o);
};

void to_json(json& j, const CFavoritesProperty& o)
{
	j = json{
		{"Favorites", o.m_spFavorites}
	};
}
void from_json(const json& j, CFavoritesProperty& o)
{
	j.at("Favorites").get_to(o.m_spFavorites);
}
