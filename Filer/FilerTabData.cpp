#include "FilerTabData.h"

/****************/
/* FilerTabData */
/****************/

FilerTabData::FilerTabData(const std::wstring& path)
	:TabData(), Path(path)
{
	if (!Path.empty()) {
		auto spFile = CShellFileFactory::GetInstance()->CreateShellFilePtr(path);
		if (auto sp = std::dynamic_pointer_cast<CShellFolder>(spFile)) {
			FolderPtr = sp;
		} else {
			FolderPtr = CKnownFolderManager::GetInstance()->GetDesktopFolder();
			Path = FolderPtr->GetPath();
		}
	}
}