#include "TextTabData.h"
#include "TextFileDialog.h"
#include "TextEnDecoder.h"
#include "D2DWWindow.h"
#include "MyFile.h"


/***************/
/* TextTabData */
/***************/
void TextTabData::Open(HWND hWnd)
{
	CTextFileOpenDialog dlg;

	dlg.SetEncodingTypes(
		{
			encoding_type::UNKNOWN,
			encoding_type::UTF16BE,
			encoding_type::UTF16LE,
			encoding_type::UTF16LEN,
			encoding_type::UTF8,
			encoding_type::UTF8N,
			encoding_type::SJIS,
			encoding_type::ASCII,
			encoding_type::JIS,
			encoding_type::EUC
		}
	);

	dlg.SetFileTypes({ {L"Text (*.txt)", L"*.txt"}, {L"All (*.*)", L"*.*"} });
	
	dlg.Show(hWnd);

	if (!dlg.GetPath().empty()) {
		Doc.get_unconst()->Open(dlg.GetPath(), dlg.GetSelectedEncodingType());
	} else {
		return;
	}
}

void TextTabData::OpenAs(HWND hWnd)
{
	if (::PathFileExists(Doc->Path->c_str())) {
		CTextFileOpenDialog dlg;

		dlg.SetEncodingTypes(
			{
				encoding_type::UNKNOWN,
				encoding_type::UTF16BE,
				encoding_type::UTF16LE,
				encoding_type::UTF16LEN,
				encoding_type::UTF8,
				encoding_type::UTF8N,
				encoding_type::SJIS,
				encoding_type::ASCII,
				encoding_type::JIS,
				encoding_type::EUC
			}
		);
		dlg.SetFileTypes({ {L"Text (*.txt)", L"*.txt"}, {L"All (*.*)", L"*.*"} });

		dlg.SetFolder(::PathFindDirectory(*Doc->Path));
		dlg.SetFileName(::PathFindFileNameW(Doc->Path->c_str()));
		dlg.SetSelectedEncodingType(*Doc->Encoding);

		dlg.Show(hWnd);

		if (!dlg.GetPath().empty()) {
			Doc.get_unconst()->Open(dlg.GetPath(), dlg.GetSelectedEncodingType());
		} else {
			return;
		}
	} else {
		Open(hWnd);
	}
}

void TextTabData::Save(HWND hWnd)
{
	if (!::PathFileExistsW(Doc->Path->c_str())) {
		CTextFileSaveDialog dlg;

		dlg.SetEncodingTypes(
			{
				encoding_type::UNKNOWN,
				encoding_type::UTF16BE,
				encoding_type::UTF16LE,
				encoding_type::UTF16LEN,
				encoding_type::UTF8,
				encoding_type::UTF8N,
				encoding_type::SJIS,
				encoding_type::ASCII,
				encoding_type::JIS,
				encoding_type::EUC
			}
		);

		dlg.SetFileTypes({ {L"Text (*.txt)", L"*.txt"}, {L"All (*.*)", L"*.*"} });

		dlg.Show(hWnd);

		if (!dlg.GetPath().empty()) {
			Doc.get_unconst()->Save(dlg.GetPath(), dlg.GetSelectedEncodingType());
		} else {
			return;
		}
	} else {
		Doc.get_unconst()->Save(*Doc->Path, *Doc->Encoding);
	}
}

void TextTabData::SaveAs(HWND hWnd)
{
	if (::PathFileExistsW(Doc->Path->c_str())) {
		CTextFileSaveDialog dlg;

		dlg.SetEncodingTypes(
			{
				encoding_type::UNKNOWN,
				encoding_type::UTF16BE,
				encoding_type::UTF16LE,
				encoding_type::UTF16LEN,
				encoding_type::UTF8,
				encoding_type::UTF8N,
				encoding_type::SJIS,
				encoding_type::ASCII,
				encoding_type::JIS,
				encoding_type::EUC
			}
		);
		dlg.SetFileTypes({ {L"Text (*.txt)", L"*.txt"}, {L"All (*.*)", L"*.*"} });

		dlg.SetFolder(::PathFindDirectory(*Doc->Path));
		dlg.SetFileName(::PathFindFileNameW(Doc->Path->c_str()));
		dlg.SetSelectedEncodingType(*Doc->Encoding);

		dlg.Show(hWnd);
		
		if (!dlg.GetPath().empty()) {
			Doc.get_unconst()->Save(dlg.GetPath(), dlg.GetSelectedEncodingType());
		} else {
			return;
		}
	} else {
		Save(hWnd);
	}
}

bool TextTabData::AcceptClosing(CD2DWWindow* pWnd, bool isWndClosing)
{
	if (!TabData::AcceptClosing(pWnd, isWndClosing)) {
		return false;
	} else {
		if (*Doc->Status == FileStatus::Dirty) {
			int ync = pWnd->MessageBox(
				fmt::format(L"\"{}\" is not saved.\r\nDo you like to save?", ::PathFindFileName(Doc->Path->c_str())).c_str(),
				L"Save?",
				MB_YESNOCANCEL);
			switch (ync) {
				case IDYES:
					Save(pWnd->m_hWnd);
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

