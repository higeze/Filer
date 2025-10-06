#pragma once
#include <vector>
#include <atlcomcli.h>

class CShellFile;

#define INITGUID
#include <objbase.h>

#define INITGUID
#include <objbase.h>

#define USES_IID_IMessage
#include <initguid.h> //this is needed,
#include <mapiguid.h> //then this
#include <mapix.h>
#include <mapitags.h>
#include <mapidefs.h>
#include <mapiutil.h>
#include <imessage.h>
#include <fcntl.h>

#define SETFormatEtc(fe, cf, asp, td, med, li)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=asp;\
    (fe).ptd=td;\
    (fe).tymed=med;\
    (fe).lindex=li;\
    }

#define SETDefFormatEtc(fe, cf, med)   \
    {\
    (fe).cfFormat=cf;\
    (fe).dwAspect=DVASPECT_CONTENT;\
    (fe).ptd=NULL;\
    (fe).tymed=med;\
    (fe).lindex=-1;\
    }

// {00020D0B-0000-0000-C000-000000000046}
DEFINE_GUID(CLSID_MailMessage,
	0x00020D0B,
	0x0000, 0x0000, 0xC0, 0x00, 0x0, 0x00, 0x0, 0x00, 0x00, 0x46);

struct medium_deleter
{
	void operator()(LPSTGMEDIUM p)
	{
		if (p) {
			::ReleaseStgMedium(p);
			delete p;
		}
	}
};

using UNQ_STDMEDIUM = std::unique_ptr<STGMEDIUM, medium_deleter>;

template<class T>
struct void_global_deleter
{
	HGLOBAL m_hGlobal;
	void_global_deleter(HGLOBAL hGlobal) :m_hGlobal(hGlobal) {}
	void operator()(T* p)
	{
		::GlobalUnlock(m_hGlobal);
	}
};

template<class T>
using UNQ_GLOBAL = std::unique_ptr<T, void_global_deleter<T>>;

class CDataObject
{
public:
	static CLIPFORMAT s_cf_shellidlist;
	static CLIPFORMAT s_cf_filecontents;
	static CLIPFORMAT s_cf_filegroupdescriptor;
	static CLIPFORMAT s_cf_renprivatemessages;

	//struct message_deleter
	//{
	//	void operator()(LPMESSAGE p)
	//	{
	//		if (p) {
	//			p->Release();
	//		}
	//	}
	//};


private:
    CComPtr<IDataObject> m_pDataObject;
public:
	CDataObject(IDataObject* p);

	CComPtr<IDataObject> ptr() const { return m_pDataObject; }

	std::vector<FORMATETC> EnumFormats() const;

	std::vector<std::shared_ptr<CShellFile>> EnumShellFiles() const;

	UNQ_STDMEDIUM GetMediumData(FORMATETC& format) const;
};

template<class T>
UNQ_GLOBAL<T> GetGlobalData(HGLOBAL hGlobal)
{
	return UNQ_GLOBAL<T>(
			reinterpret_cast<T*>(::GlobalLock(hGlobal)),
			void_global_deleter<T>(hGlobal));
}


