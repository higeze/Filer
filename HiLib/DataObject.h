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


class CDataObject
{
public:
	static CLIPFORMAT s_cf_shellidlist;
	static CLIPFORMAT s_cf_filecontents;
	static CLIPFORMAT s_cf_filegroupdescriptor;
	static CLIPFORMAT s_cf_renprivatemessages;

	struct medium_global_deleter
	{
		void operator()(LPSTGMEDIUM p)
		{
			if (p) {
				::GlobalUnlock(p->hGlobal);
				::ReleaseStgMedium(p);
				delete p;
			}
		}
	};

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

	std::unique_ptr<STGMEDIUM, medium_deleter> GetMediumData(FORMATETC& format) const;
	std::unique_ptr<STGMEDIUM, medium_global_deleter> GetGlobalMediumData(FORMATETC& format) const;


};