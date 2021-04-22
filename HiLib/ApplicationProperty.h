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

	FRIEND_SERIALIZER

	template <class Archive>
	void save(Archive& ar)
	{
		ar("Debug", m_bDebug);
		//const type_info& info = typeid(ar);
		//if (info == typeid(CCellDeserializer&)) {
		//	if (m_bDebug) {
		//		if (!g_console.IsOpen()) {
		//			g_console.Alloc();
		//		}
		//	} else {
		//		if (g_console.IsOpen()) {
		//			g_console.Free();
		//		}
		//	}
		//}
	}

	template <class Archive>
	void load(Archive& ar)
	{
		ar("Debug", m_bDebug);
		//if (m_bDebug) {
		//	if (!g_console.IsOpen()) {
		//		g_console.Alloc();
		//	}
		//}else{
		//	//if (g_console.IsOpen()) {
		//	//	g_console.Free();
		//	//}
		//}
	}

    friend void to_json(json& j, const CApplicationProperty& o);
    friend void from_json(const json& j, CApplicationProperty& o);
};

void to_json(json& j, const CApplicationProperty& o)
{
	j = json{
		{"Debug", o.m_bDebug}
	};
}
void from_json(const json& j, CApplicationProperty& o)
{
	j.at("Debug").get_to(o.m_bDebug);
}
