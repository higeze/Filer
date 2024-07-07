#include "ThreadMonitorView.h"
#include "ThreadPool.h"
#include "Dispatcher.h"

void CThreadMonitorView::OnCreate(const CreateEvt& e)
{
	CColoredTextBox::OnCreate(e);

	auto update = [this] {
		Text.set(CThreadPool::GetInstance()->OutputString());
		GetWndPtr()->InvalidateRect(NULL, FALSE);
	};

	IsEnabled.subscribe([this, update](auto value) {
		if (value) {
			update();
			m_update_timer.run([this, update]()->void {
				GetWndPtr()->GetDispatcherPtr()->PostInvoke(update);
			}, std::chrono::milliseconds(3000));

		} else {
			m_update_timer.stop();
		}
	}, shared_from_this());

	IsEnabled.force_notify_set(*IsEnabled);
}