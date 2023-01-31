#include "PDFiumSingleThread.h"

#include "ThreadPool.h"
#include "se_exception.h"
#include <fmt/format.h>

// the constructor just launches some amount of workers
CPDFiumMultiThread::CPDFiumMultiThread(size_t threads)
	: stop(false), spetasks(threads)
{
	for (size_t i = 0; i < threads; ++i) {
		workers.emplace_back([this]
		{
			 //CoInitialize
			 CCoInitializer coinit;
			 //Catch SEH exception as CEH
			 scoped_se_translator se_trans;
			 //ID
			 std::thread::id id = std::this_thread::get_id();
			 //PDF
			 PDFObject pdf;
			 for (;;) {
				 std::function<void(PDFObject&)> task;
				 {
					 std::unique_lock<std::mutex> lock(this->queue_mutex);
					 this->condition.wait(lock,
						 [this, id] { return this->stop || !this->tasks.empty() || !this->spetasks[id].empty(); });
					 if (this->stop && this->tasks.empty() && this->spetasks[id].empty()) {
						 return;
					 }else if (!spetasks[id].empty()) {
						 task = std::move(this->spetasks[id].front());
						 this->spetasks[id].pop();
					 } else if (!tasks.empty()) {
						 task = std::move(this->tasks.front());
						 this->tasks.pop();
					 }
				 }

				 activeCount++;
				 try {
					 task(pdf);
				 }
				 catch (std::exception& ex) {
					 std::string msg = fmt::format(
						 "What:{}\r\n"
						 "Last Error:{}\r\n",
						 ex.what(), GetLastErrorString());

					 ::MessageBoxA(nullptr, msg.c_str(), "Exception in Thread Pool", MB_ICONWARNING);
				 }
				 catch (...) {
					 std::string msg = fmt::format(
						 "Last Error:{}",
						 GetLastErrorString());

					 MessageBoxA(nullptr, msg.c_str(), "Unknown Exception in Thread Pool", MB_ICONWARNING);
				 }
				 activeCount--;
			 }
		});
	}
}

// the destructor joins all threads
CPDFiumMultiThread::~CPDFiumMultiThread()
{
	{
		std::unique_lock<std::mutex> lock(queue_mutex);
		stop = true;
	}
	condition.notify_all();
	for (std::thread &worker : workers)
		worker.join();
}




