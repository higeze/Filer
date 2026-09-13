#include "ShellThread.h"

CShellThread::CShellThread()
    : m_thread(), m_threadId(0), m_stop(false)
{
    m_thread = std::thread([this]() { ThreadProc(); });
    // スレッド起動後にスレッドID を取得するため、メッセージで応答を受け取る
    // ThreadProc は最初に PostThreadMessage を使うので、少し待つ
    // 遅延なしで m_threadId を待つ簡単なループ
    for (int i = 0; i < 50 && m_threadId == 0; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

CShellThread::~CShellThread()
{
    Stop();
    if (m_thread.joinable()) m_thread.join();
}

void CShellThread::Stop()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_stop) return;
        m_stop = true;
    }
    // Wake the thread so it breaks out
    if (m_threadId != 0) ::PostThreadMessage(m_threadId, WM_QUIT, 0, 0);
}

CShellThread* CShellThread::GetInstance()
{
    static CShellThread thread;
    return &thread;
}
