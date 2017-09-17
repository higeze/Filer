#pragma once
#include "MyFriendSerializer.h"

class CFavorite
{
private:
	std::wstring m_path;
	std::wstring m_shortName;
public:
	CFavorite(void);
	CFavorite(std::wstring path, std::wstring shortName);
	virtual ~CFavorite(void){}

	std::wstring GetPath()const{return m_path;}
	std::wstring GetShortName()const{return m_shortName;}

	FRIEND_SERIALIZER
    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("Path", m_path);
		ar("ShortName",m_shortName);
    }
};

