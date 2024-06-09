#pragma once
#include <cereal/cereal.hpp>
#include "MyRect.h"
#include "MyFriendSerializer.h"
#include "Console.h"

extern CConsole g_console;

class CApplicationProperty
{
public:
	//bool m_bOnlyOneInstance;
	//bool m_bCheckUpdateOnCreate;
	bool m_bDebug;

	CApplicationProperty():
		//m_bOnlyOneInstance(false),
		//m_bCheckUpdateOnCreate(true),
		m_bDebug(false){}
	virtual ~CApplicationProperty(){}

	template <class Archive>
	void save(Archive& archive) const
	{
		archive(cereal::make_nvp("Debug", m_bDebug));
	}
	template <class Archive>
	void load(Archive& archive)
	{
		archive(cereal::make_nvp("Debug", m_bDebug));
	}
	friend void to_json(json& j, const CApplicationProperty& o)
	{
		j = json{
			{"Debug", o.m_bDebug}
		};
	}
	friend void from_json(const json& j, CApplicationProperty& o)
	{
		j.at("Debug").get_to(o.m_bDebug);
	}
};
