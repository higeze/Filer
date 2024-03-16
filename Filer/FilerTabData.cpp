#include "FilerTabData.h"

/****************/
/* FilerTabData */
/****************/

FilerTabData::FilerTabData(const std::wstring& path)
	:TabData()
{
	if (!path.empty()) {
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(path);
		if (auto sp = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
			Folder.set(sp);
		} else {
			Folder.set(CKnownFolderManager::GetInstance()->GetDesktopFolder());
		}
	}
}