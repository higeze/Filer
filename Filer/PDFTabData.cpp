#include "PDFTabData.h"
#include "D2DWWindow.h"


/**************/
/* PdfTabData */
/**************/
bool PdfTabData::AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing)
{
	if (!TabData::AcceptClosing(pWnd, isWndClosing)) {
		return false;
	} else {
		if (*Doc->IsDirty) {
			int ync = pWnd->MessageBox(
				fmt::format(L"\"{}\" is not saved.\r\nDo you like to save?", ::PathFindFileName(Doc->Path->c_str())).c_str(),
				L"Save?",
				MB_YESNOCANCEL);
			switch (ync) {
				case IDYES:
					Doc.get_unconst()->Save();
					return true;
				case IDNO:
					return true;
				case IDCANCEL:
					return false;
				default:
					return true;
			}
		} else {
			return true;
		}
	}
}
