#pragma once
#include <vector>
#include <string>
#include <atlbase.h>
#include "encoding_type.h"
#include "ResourceIDFactory.h"
#include "MyString.h"
#include "Debug.h"
#include <ShObjIdl.h>
#include <format>
#include <nameof/nameof.hpp>
#include "strconv.h"



template<class T>
class CTextFileDialog
{
private:

	std::tuple<DWORD, std::wstring> create_id_name(const auto& enc)
	{
		std::wstring enc_name = sjis_to_wide(std::string(nameof::nameof_enum(enc)));
		DWORD id = CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, std::format(L"CTextFileDialogCombobox_{}", enc_name));
		return { id, enc_name };
	};


protected:
	CComPtr<T> m_pDlg;
	CComPtr<IFileDialogCustomize> m_pDlgCust;

	std::vector<encoding_type> m_encoding_types;
	encoding_type m_selected_encoding_type;
	std::wstring m_path;

public:

	CTextFileDialog()
	{
		//Create COM
		if constexpr (std::is_same<T, IFileOpenDialog>::value) {
			FAILED_THROW(m_pDlg.CoCreateInstance(CLSID_FileOpenDialog));
		} else if constexpr (std::is_same<T, IFileSaveDialog>::value) {
			FAILED_THROW(m_pDlg.CoCreateInstance(CLSID_FileSaveDialog));
		} else {
			THROW_FILE_LINE_FUNC;
		}
		FAILED_THROW(m_pDlg->QueryInterface(IID_PPV_ARGS(&m_pDlgCust)));
	}

	void SetFolder(const std::wstring& folder)
	{
		CComPtr<IShellItem> pItem;
		::SHCreateItemFromParsingName(folder.c_str(), nullptr, IID_PPV_ARGS(&pItem));
		FAILED_THROW(m_pDlg->SetDefaultFolder(pItem));
	}
	
	void SetFileName(const std::wstring& file)
	{
		FAILED_THROW(m_pDlg->SetFileName(file.c_str()));
	}

	void SetFileTypes(const std::vector<COMDLG_FILTERSPEC>& types)
	{
		FAILED_THROW(m_pDlg->SetFileTypes(types.size(), types.data()));
	}

	virtual void SetEncodingTypes(const std::vector<encoding_type>& encoding_types)
	{
		if (m_encoding_types != encoding_types) {
			m_encoding_types = encoding_types;
			//Combobox
			DWORD dwComboBoxID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"CTextFileDialogCombobox");
			//Clear
			m_pDlgCust->RemoveAllControlItems(dwComboBoxID);
			//Add
			m_pDlgCust->AddComboBox(dwComboBoxID);
			for (const auto& enc : m_encoding_types) {
				auto [id, name] = create_id_name(enc);
				m_pDlgCust->AddControlItem(dwComboBoxID, id, name.c_str());
			}

			//Select
			m_pDlgCust->SetSelectedControlItem(dwComboBoxID, std::get<0>(create_id_name(m_encoding_types[0])));
		}
	}

	virtual encoding_type GetSelectedEncodingType()const { return m_selected_encoding_type; }
	virtual void SetSelectedEncodingType(const encoding_type& enc)
	{
		if (m_selected_encoding_type != enc) {
			m_selected_encoding_type = enc;

			DWORD dwComboBoxID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"CTextFileDialogCombobox");
			m_pDlgCust->SetSelectedControlItem(dwComboBoxID, std::get<0>(create_id_name(enc)));
		}
	}
	virtual const std::wstring GetPath()const { return m_path; }
	virtual void Show(HWND hWnd)
	{
		if (SUCCEEDED(m_pDlg->Show(hWnd))) {

			DWORD dwComboBoxID = CResourceIDFactory::GetInstance()->GetID(ResourceType::Control, L"CTextFileDialogCombobox");
			DWORD dwSelItem;
			FAILED_THROW(m_pDlgCust->GetSelectedControlItem(dwComboBoxID, &dwSelItem));
			auto iter = std::find_if(m_encoding_types.cbegin(), m_encoding_types.cend(), [dwSelItem, this](const auto& enc)
				{
						return std::get<0>(create_id_name(enc)) == dwSelItem;
				});

			if (iter == m_encoding_types.cend()) {
				THROW_FILE_LINE_FUNC;
			} else {
				m_selected_encoding_type = *iter;
			}

			//Path
			CComPtr<IShellItem> pItem;
			LPWSTR pPath;
			FAILED_THROW(m_pDlg->GetResult(&pItem));
			FAILED_THROW(pItem->GetDisplayName(SIGDN_FILESYSPATH, &pPath));
			m_path = pPath;
			::CoTaskMemFree(pPath);
		}
	}
};

class CTextFileOpenDialog :public CTextFileDialog<IFileOpenDialog>{};

class CTextFileSaveDialog :public CTextFileDialog<IFileSaveDialog>{};
	