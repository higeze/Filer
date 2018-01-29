#pragma once
#include "MyFriendSerializer.h"

class CShellFile;

class CFavorite
{
private:
	std::wstring m_path;
	std::wstring m_shortName;
	std::shared_ptr<CShellFile> m_spFile;
public:
	CFavorite(void);
	CFavorite(std::wstring path, std::wstring shortName);
	virtual ~CFavorite(void){}

	std::wstring GetPath()const{return m_path;}
	std::wstring GetShortName()const{return m_shortName;}
	std::shared_ptr<CShellFile>& GetShellFile() { return m_spFile; }
	void SetShellFile(const std::shared_ptr<CShellFile> spFile) { m_spFile = spFile; }

	FRIEND_SERIALIZER
    template <class Archive>
    void serialize(Archive& ar)
    {
		ar("Path", m_path);
		ar("ShortName",m_shortName);
    }
};

