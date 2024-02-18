#pragma once
#include "getter_macro.h"
#include "MyString.h"
#include "MyIcon.h"
#include "IDL.h"
#include "ShellFunction.h"
#include <future>
#include <chrono>
#include <optional>
#include <atlcom.h>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/seq/fold_left.hpp>
#include <boost/preprocessor/variadic/to_seq.hpp>
#include <boost/preprocessor/seq/for_each.hpp>




#define OPT_RESET(r, data, elem) BOOST_PP_CAT(BOOST_PP_CAT(m_opt, elem), .reset());
#define DECLARE_RESET_OPTS(...) void ResetOpts(){BOOST_PP_SEQ_FOR_EACH(OPT_RESET, _, BOOST_PP_VARIADIC_TO_SEQ(__VA_ARGS__))}


class CShellFolder;
struct FileSizeArgs;
struct FileTimeArgs;

bool GetDirSize(std::wstring path, ULARGE_INTEGER& size, std::function<void()> cancel);

struct findclose
{
	void operator()(HANDLE handle)const
	{
		if (!::FindClose(handle)) {
			//FILE_LINE_FUNC_TRACE;
		}
	}

};

enum class FileSizeStatus
{
	None,
	Available,
	Calculating,
	Unavailable,
};

enum class FileIconStatus
{
	None,
	Available,
	Loading,
};

enum class FileTimeStatus
{
	None,
	Available,
	AvailableLoading,
	Loading,
	Unavailable,
};

class CShellFile: public std::enable_shared_from_this<CShellFile>
{
protected:
	CComPtr<IShellFolder> m_pParentShellFolder;
	CIDL m_absoluteIdl;
	CIDL m_parentIdl;
	CIDL m_childIdl;

	std::pair<FileTimes, FileTimeStatus> m_fileTimes = std::make_pair(FileTimes(), FileTimeStatus::None);

	std::pair<ULARGE_INTEGER, FileSizeStatus> m_size = std::make_pair(ULARGE_INTEGER(), FileSizeStatus::None);
	//std::pair<std::shared_ptr<CIcon>, FileIconStatus> m_icon = std::make_pair(std::shared_ptr<CIcon>(nullptr), FileIconStatus::None);

public:
	//Constructor
	CShellFile() {}
	CShellFile(const std::wstring& path);
	CShellFile(CComPtr<IShellFolder> pParentShellFolder, CIDL parentIdl, CIDL childIdl);
	//CShellFile(const std::wstring& path);

	//Destructor
	virtual ~CShellFile();
	
	//Getter 
	CComPtr<IShellFolder>& GetParentShellFolderPtr(){return m_pParentShellFolder;}
	const CIDL& GetAbsoluteIdl() const { return m_absoluteIdl; }
	const CIDL& GetChildIdl() const { return m_childIdl; }

	//Lazy Evaluation Getter
	DECLARE_LAZY_GETTER(std::wstring, Path);
	DECLARE_LAZY_GETTER(std::wstring, PathName);
	DECLARE_LAZY_GETTER(std::wstring, PathNameWithoutExt);
	DECLARE_LAZY_GETTER(std::wstring, PathExt);
	DECLARE_LAZY_GETTER(std::wstring, PathWithoutExt);
	DECLARE_LAZY_GETTER(std::wstring, DispName);
	DECLARE_LAZY_GETTER(std::wstring, DispExt);
	DECLARE_LAZY_GETTER(std::wstring, DispNameWithoutExt);
	
	DECLARE_LAZY_GETTER(std::wstring, TypeName);

	DECLARE_LAZY_GETTER(DWORD, Attributes);
	DECLARE_LAZY_GETTER(SFGAOF, SFGAO);

	DECLARE_LAZY_GETTER(std::wstring, IconKey);

	DECLARE_RESET_OPTS(Path, PathName, PathNameWithoutExt, PathExt, PathWithoutExt, DispName, DispExt, DispNameWithoutExt, TypeName, Attributes, SFGAO, IconKey)
	
	
	//Non-lazy
	CIcon GetIcon() const;
	bool GetIsExist();

	virtual void SetFileNameWithoutExt(const std::wstring& wstrNameWoExt, HWND hWnd = NULL);
	virtual void SetExt(const std::wstring& wstrExt, HWND hWnd = NULL);

	//LastWrite
	std::optional<FileTimes> GetFileTimes();
	virtual std::pair<FileTimes, FileTimeStatus> GetFileTimes(const std::shared_ptr<FileTimeArgs>& spArgs, std::function<void()> changed = nullptr);

	//Size
	bool GetFileSize(ULARGE_INTEGER& size/*, std::shared_future<void> future*/);
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(const std::shared_ptr<FileSizeArgs>& spArgs, std::function<void()> changed = nullptr);
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> ReadSize();

	//Icon
	//virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon(std::function<void(CShellFile*)>& changedAction);

	//bool HasIconInCache();
	bool IsInvalid() const;
	bool IsDirectory() const;
	virtual void Reset();
	virtual void Execute(const wchar_t* lpVerb);
	virtual void Load(const std::wstring& path);
	virtual void Open();
	virtual void RunAs();
	virtual void AddToRecentDocs();

private:
	//virtual void ResetIcon();
	virtual void ResetSize();
	virtual void ResetTime();

	//std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetLockIcon();
	//void SetLockIcon(std::pair<std::shared_ptr<CIcon>, FileIconStatus>& icon);
};


class CShellInvalidFile :public CShellFile
{
public:
	//Constructor
	CShellInvalidFile() {}

	//Destructor
	virtual ~CShellInvalidFile(){}

	virtual const std::wstring& GetPath() const override{ return std::wstring(); }
	virtual const std::wstring& GetDispNameWithoutExt() const override { return std::wstring(); }
	virtual const std::wstring& GetDispName() const override { return std::wstring(); }
	virtual const std::wstring& GetDispExt() const override { return std::wstring(); }
	virtual void Open() override {/* Do Nothing */ }

	//Icon
//	virtual std::pair<std::shared_ptr<CIcon>, FileIconStatus> GetIcon(std::function<void(CShellFile*)>& changedAction)override;
};

