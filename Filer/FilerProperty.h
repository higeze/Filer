#pragma once
#include "MyFriendSerializer.h"
#include "ApplicationProperty.h"
#include "FilerWnd.h"


class CFilerProperty
{
private:
	std::shared_ptr<CFilerWnd> m_spFilerWnd;

public:
	CFilerProperty()
		:m_spFilerWnd(std::make_shared<CFilerWnd>()){}
	~CFilerProperty(){}

	std::shared_ptr<CFilerWnd> GetFilerWndPtr(){return m_spFilerWnd;}

public:
	FRIEND_SERIALIZER
    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("FilerWnd", m_spFilerWnd);
    }
};

