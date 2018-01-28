#pragma once
#include "MyFriendSerializer.h"
#include "Favorite.h"

class CFavoritesProperty
{
private:
	std::shared_ptr<std::vector<CFavorite>> m_favorites;

public:
	CFavoritesProperty():m_favorites(std::make_shared<std::vector<CFavorite>>())
	{
		m_favorites->push_back(CFavorite(L"",L"DT"));
	};
	~CFavoritesProperty(){};

	std::shared_ptr<std::vector<CFavorite>> GetFavorites()const{return m_favorites;}
	void SetFavorites(std::shared_ptr<std::vector<CFavorite>>& favorites) { m_favorites = favorites; }

	FRIEND_SERIALIZER
    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("Favorites", m_favorites);
    }
};