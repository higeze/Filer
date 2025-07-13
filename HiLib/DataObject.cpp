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

		std::unique_ptr<STGMEDIUM, medium_global_deleter> pMedium = GetGlobalMediumData(formatetc);

		LPIDA pida = (LPIDA)GlobalLock(pMedium->hGlobal);
		CIDL folderIdl(::ILCloneFull((LPCITEMIDLIST)(((LPBYTE)pida) + (pida)->aoffset[0])));
		CComPtr<IShellFolder> pFolder = shell::DesktopBindToShellFolder(folderIdl);

		for (UINT i = 0; i < pida->cidl; i++) {
			CIDL childIdl(::ILCloneFull((LPCITEMIDLIST)(((LPBYTE)pida) + pida->aoffset[1 + i])));
			files.emplace_back(CShellFileFactory::GetInstance()->CreateShellFilePtr(pFolder, folderIdl, std::move(childIdl)));
		}
	}
	return files;
}

std::unique_ptr<STGMEDIUM, CDataObject::medium_deleter> CDataObject::GetMediumData(FORMATETC& format) const
{
	auto pMedium = std::unique_ptr<STGMEDIUM, CDataObject::medium_deleter>(new STGMEDIUM());
	FAILED_THROW(m_pDataObject->GetData(&format, pMedium.get()));
	return pMedium;
}
std::unique_ptr<STGMEDIUM, CDataObject::medium_global_deleter> CDataObject::GetGlobalMediumData(FORMATETC& format) const
{
	auto pMedium = std::unique_ptr<STGMEDIUM, CDataObject::medium_global_deleter>(new STGMEDIUM());
	FAILED_THROW(m_pDataObject->GetData(&format, pMedium.get()));
	FALSE_THROW(::GlobalLock(pMedium->hGlobal));
	return pMedium;
}


