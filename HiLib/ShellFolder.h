#pragma once
#include "ShellFile.h"
#include <chrono>
#include <mutex> // ’Ç‰Á: #include <mutex> ‚ÍŠù‚É‚ ‚é‚ª”O‚Ì‚½‚ßŠm”F

//template<typename T>
//class comptr_deleter
//{
//public:
//	void operator() (T* ptr)
//	{
//		if (ptr) {
//            ptr->Release();
//		}
//	}
//};
//
//#include <atlbase.h>   // For CComPtr
//#include <mutex>       // For std::mutex
//#include <utility>     // For std::move
//
//// Thread-safe wrapper around CComPtr
//template <typename T>
//class ThreadSafeComPtr
//{
//public:
//    ThreadSafeComPtr() = default;
//
//    // Construct from raw pointer
//    explicit ThreadSafeComPtr(T* ptr) : ptr_(ptr) {}
//
//    // Copy constructor
//    ThreadSafeComPtr(const ThreadSafeComPtr& other)
//    {
//        std::lock_guard<std::mutex> lock(other.mtx_);
//        ptr_ = other.ptr_;
//    }
//
//    // Move constructor
//    ThreadSafeComPtr(ThreadSafeComPtr&& other) noexcept
//    {
//        std::lock_guard<std::mutex> lock(other.mtx_);
//        ptr_ = std::move(other.ptr_);
//    }
//
//    // Copy assignment
//    ThreadSafeComPtr& operator=(const ThreadSafeComPtr& other)
//    {
//        if (this != &other) {
//            // Lock both mutexes without deadlock
//            std::scoped_lock lock(mtx_, other.mtx_);
//            ptr_ = other.ptr_;
//        }
//        return *this;
//    }
//
//    // Move assignment
//    ThreadSafeComPtr& operator=(ThreadSafeComPtr&& other) noexcept
//    {
//        if (this != &other) {
//            std::scoped_lock lock(mtx_, other.mtx_);
//            ptr_ = std::move(other.ptr_);
//        }
//        return *this;
//    }
//
//    // Assign from raw pointer
//    ThreadSafeComPtr& operator=(T* rawPtr)
//    {
//        std::lock_guard<std::mutex> lock(mtx_);
//        ptr_ = rawPtr;
//        return *this;
//    }
//
//    // Get a thread-safe copy of the pointer
//    CComPtr<T> Get() const
//    {
//        std::lock_guard<std::mutex> lock(mtx_);
//        return ptr_;
//    }
//
//    // Reset pointer
//    void Reset()
//    {
//        std::lock_guard<std::mutex> lock(mtx_);
//        ptr_.Release();
//    }
//
//    // Access underlying pointer (read-only)
//    T* operator->() const
//    {
//        std::lock_guard<std::mutex> lock(mtx_);
//        return ptr_;
//    }
//
//    // Check if pointer is not null
//    bool IsValid() const
//    {
//        std::lock_guard<std::mutex> lock(mtx_);
//        return ptr_ != nullptr;
//    }
//
//private:
//    mutable std::mutex mtx_;
//    CComPtr<T> ptr_;
//};
//
//

class CShellFolder :public CShellFile
{
private:
	std::shared_ptr<bool> m_spCancelThread = std::make_shared<bool>(false);
	
	std::future<std::pair<ULARGE_INTEGER, FileSizeStatus>> m_futureSize;
	std::future<std::pair<FileTimes, FileTimeStatus>> m_futureTime;

	mutable std::mutex m_mtxSize;
	mutable std::mutex m_mtxTime;
public:

protected: mutable CThreadSafeComPtr<IShellFolder> m_pShellFolder; public: virtual CThreadSafeComPtr<IShellFolder> GetShellFolderPtr() const;
	DECLARE_LAZY_SHAREDPTR_GETTER(CShellFolder, ParentFolder);

public:
	template<typename... _Args>
	CShellFolder(CThreadSafeComPtr<IShellFolder> pParentShellFolder, const CIDL& parentIdl, const CIDL& childIdl, _Args... args)
		:CShellFile(pParentShellFolder, parentIdl, childIdl, args...)/*, m_pShellFolder(::get(arg<"ishellfolder"_s>(), args..., default_(nullptr)))*/{}

	virtual ~CShellFolder();

	//CShellFolder Clone() const;

	virtual const std::wstring& GetDispName() const override;
	virtual const std::wstring& GetDispNameWithoutExt() const override;
	virtual const std::wstring& GetDispExt() const override;

	virtual void SetFileNameWithoutExt(const std::wstring& wstrNameWoExt, HWND hWnd = NULL) override;
	virtual void SetExt(const std::wstring& wstrExt, HWND hWnd = NULL) override;

	std::shared_ptr<CShellFolder> Clone()const;
	virtual std::pair<ULARGE_INTEGER, FileSizeStatus> GetSize(const FileSizeArgs& args, std::function<void()> changed = nullptr)override;
	virtual std::pair<FileTimes, FileTimeStatus> GetFileTimes(const FileTimeArgs& args, std::function<void()> changed = nullptr)override;
	std::shared_ptr<CShellFile> CreateShExFileFolder(CIDL&& relativeIdl) const;
	//std::shared_ptr<CShellFile> CreateShExFileFolder(const CIDL& relativeIdl) const;
	static std::optional<FileTimes> GetFolderFileTimes(const std::shared_ptr<bool>& cancel,
		CThreadSafeComPtr<IShellFolder> pParentFolder, CThreadSafeComPtr<IShellFolder> pFolder, const CIDL& relativeIdl, const std::wstring& path,
		std::chrono::system_clock::time_point& tp, int limit, bool ignoreFolderTime);
	static std::optional<FileTimes> GetFolderFileTimes(
		const std::shared_ptr<bool>& cancel,
		const CIDL& parentIdl,
		const CIDL& relativeIdl,
		const std::wstring& path,
		std::chrono::system_clock::time_point& tp,
		int limit,
		bool ignoreFolderTime);
	static bool GetFolderSize(ULARGE_INTEGER& size, const std::shared_ptr<bool>& cancel,
		CThreadSafeComPtr<IShellFolder> pFolder, const std::wstring& path,
		const std::chrono::system_clock::time_point& tp, const int limit);
private:
	std::pair<FileTimes, FileTimeStatus> GetLockFileTimes() const;
	std::pair<ULARGE_INTEGER, FileSizeStatus> GetLockSize() const;

protected:
	void SetLockSize(const std::pair<ULARGE_INTEGER, FileSizeStatus>& size);
	void SetLockFileTimes(const std::pair<FileTimes, FileTimeStatus>& times);
};
