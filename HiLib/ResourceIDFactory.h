#pragma once

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/sequenced_index.hpp>
#include <boost/multi_index/random_access_index.hpp>
#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/mem_fun.hpp>
#include <unordered_map>

struct IDTag {};
struct NameTag {};
struct NameID
{
	std::wstring Name;
	unsigned short ID;
};

enum class ResourceType
{
	Multiple = 0x1000,
	Dialog = 0x2000,
	Cursor = 0x3000,
	Icon = 0x4000,
	Bitmap = 0x5000,
	String = 0x6000,
	Control = 0x7000,
	Command = 0x8000,
};

struct Resource
{
	ResourceType Type;
	unsigned short CurID;
};

typedef boost::multi_index::multi_index_container<
	NameID,
	boost::multi_index::indexed_by<
	boost::multi_index::hashed_unique<boost::multi_index::tag<IDTag>, boost::multi_index::member<NameID, unsigned short, &NameID::ID>>,
	boost::multi_index::hashed_unique<boost::multi_index::tag<NameTag>, boost::multi_index::member<NameID, std::wstring, &NameID::Name>>
	>
> NameIDMap;

class CResourceIDFactory
{
private:
	NameIDMap m_nameIdMap;
	std::unordered_map<ResourceType, Resource> m_resourceMap;

	unsigned short GetNewID(const ResourceType& type);
public:
	CResourceIDFactory();

	unsigned short GetID(const ResourceType& type, const std::wstring& name);

	static CResourceIDFactory* GetInstance()
	{
		static CResourceIDFactory factory;
		return &factory;
	}
};