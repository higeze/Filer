#pragma once
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

	friend void to_json(json& j, const CApplicationProperty& o)
	{
		json_safe_to(j, "Debug", o.m_bDebug);
	}
	friend void from_json(const json& j, CApplicationProperty& o)
	{
		json_safe_from(j, "Debug", o.m_bDebug);
	}
};
