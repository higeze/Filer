#pragma once
#include "MyFriendSerializer.h"
#include "Favorite.h"

class CFavoritesProperty
{
private:
	std::vector<CFavorite> m_favorites;

public:
	CFavoritesProperty():m_favorites()
	{
		m_favorites.push_back(CFavorite(L"",L"Desktop"));
	};
	~CFavoritesProperty(){};

	std::vector<CFavorite> GetFavorites()const{return m_favorites;}

	FRIEND_SERIALIZER
    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("Favorites", m_favorites);
    }
};