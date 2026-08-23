#include "DataObject.h"
#include "ShellFile.h"
#include "ShellFileFactory.h"

CLIPFORMAT CDataObject::s_cf_shellidlist = ::RegisterClipboardFormat(CFSTR_SHELLIDLIST);
CLIPFORMAT CDataObject::s_cf_filecontents = ::RegisterClipboardFormat(CFSTR_FILECONTENTS);
CLIPFORMAT CDataObject::s_cf_filegroupdescriptor = ::RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
CLIPFORMAT CDataObject::s_cf_renprivatemessages = ::RegisterClipboardFormat(L"RenPrivateMessages");


CDataObject::CDataObject(IDataObject* p)
	:m_pDataObject(p) {
}

std::vector<FORMATETC> CDataObject::EnumFormats() const
{
	std::vector<FORMATETC> formats;
	CComPtr<IEnumFORMATETC> pEnumFormatEtc;
	if (SUCCEEDED(m_pDataObject->EnumFormatEtc(DATADIR::DATADIR_GET, &pEnumFormatEtc))) {
		FORMATETC rgelt[100];
		ULONG celtFetched = 0UL;
		if (SUCCEEDED(pEnumFormatEtc->Next(100, rgelt, &celtFetched))) {
			for (size_t i = 0; i < celtFetched; ++i) {
				formats.push_back(rgelt[i]);
			}
		}
	}
	return formats;
}

std::vector<std::shared_ptr<CShellFile>> CDataObject::EnumShellFiles() const
{
	std::vector<std::shared_ptr<CShellFile>> files;

	auto formats = EnumFormats();
	bool isShellIdList = false;
	for (const auto& format : formats) {
		isShellIdList |= format.cfFormat == s_cf_shellidlist;
	}

	if (isShellIdList) {
		FORMATETC formatetc = { 0 };
		formatetc.cfFormat = (CLIPFORMAT)RegisterClipboardFormat(CFSTR_SHELLIDLIST);
		formatetc.ptd = NULL;
		formatetc.dwAspect = DVASPECT_CONTENT;
		formatetc.lindex = -1;
		formatetc.tymed = TYMED_HGLOBAL;

		UNQ_STDMEDIUM pMedium = GetMediumData(formatetc);

		LPIDA pida = (LPIDA)GlobalLock(pMedium->hGlobal);
		CIDL folderIdl(::ILCloneFull((LPCITEMIDLIST)(((LPBYTE)pida) + (pida)->aoffset[0])));
		CThreadSafeComPtr<IShellFolder> pFolder = shell::DesktopBindToShellFolder(folderIdl);

		for (UINT i = 0; i < pida->cidl; i++) {
			CIDL childIdl(::ILCloneFull((LPCITEMIDLIST)(((LPBYTE)pida) + pida->aoffset[1 + i])));
			files.emplace_back(CShellFileFactory::GetInstance()->CreateShellFilePtr(pFolder, folderIdl, std::move(childIdl)));
		}
	}
	return files;
}

UNQ_STDMEDIUM CDataObject::GetMediumData(FORMATETC& format) const
{
	UNQ_STDMEDIUM pMedium(new STGMEDIUM());
	if (SUCCEEDED(m_pDataObject->GetData(&format, pMedium.get()))) {
		return pMedium;
	} else {
		return nullptr;
	}
}
