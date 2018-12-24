#include "ResourceIDFactory.h"

CResourceIDFactory::CResourceIDFactory()
	:m_resourceMap{
		{ResourceType::Multiple, {ResourceType::Multiple, (unsigned short)ResourceType::Multiple}},
		{ResourceType::Dialog, {ResourceType::Dialog, (unsigned short)ResourceType::Dialog}},
		{ResourceType::Cursor, {ResourceType::Cursor, (unsigned short)ResourceType::Cursor}},
		{ResourceType::Icon, {ResourceType::Icon, (unsigned short)ResourceType::Icon}},
		{ResourceType::Bitmap, {ResourceType::Bitmap, (unsigned short)ResourceType::Bitmap}},
		{ResourceType::String, {ResourceType::String, (unsigned short)ResourceType::String}},
		{ResourceType::Control, {ResourceType::Control, (unsigned short)ResourceType::Control}},
		{ResourceType::Command, {ResourceType::Command, (unsigned short)ResourceType::Command}},
	}
{}


unsigned short CResourceIDFactory::GetID(const ResourceType& type, const std::wstring& name)
{
	auto& nameDict = m_nameIdMap.get<NameTag>();
	auto iter = nameDict.find(name);
	if (iter != nameDict.end()) {
		return iter->ID;
	} else {
		return nameDict.insert({ name, GetNewID(type) }).first->ID;
	}
}

unsigned short CResourceIDFactory::GetNewID(const ResourceType& type)
{
	return m_resourceMap.find(type)->second.CurID++;
}

