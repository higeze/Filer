#include "ToDoDoc.h"
#include "Debug.h"

CToDoDoc::CToDoDoc()
	:Dummy(std::make_shared<int>(0)),
	Status(FileStatus::None),
	Path(),
	Tasks()
{
	Tasks.subscribe([status = Status](auto notify) mutable {

		auto reactive_task_subscription = [status](std::tuple<MainTask>& src) mutable {
			auto dirty = [status](auto) mutable { status.set(FileStatus::Dirty); };
			std::vector<sigslot::connection> subs;
			subs.push_back(std::get<MainTask>(src).State.subscribe(dirty, status.life()));
			subs.push_back(std::get<MainTask>(src).Name.subscribe(dirty, status.life()));
			subs.push_back(std::get<MainTask>(src).Memo.subscribe(dirty, status.life()));
			subs.push_back(std::get<MainTask>(src).YearMonthDay.subscribe(dirty, status.life()));
			return subs;
		};

		switch (notify.action) {
			case notify_container_changed_action::push_back:
			case notify_container_changed_action::insert:
			{
				status.set(FileStatus::Dirty);
				std::ranges::for_each(notify.new_items, reactive_task_subscription);
				break;
			}
			case notify_container_changed_action::Move:
				THROW_FILE_LINE_FUNC;
				break;
			case notify_container_changed_action::erase:
				status.set(FileStatus::Dirty);
				break;
			case notify_container_changed_action::replace:
				THROW_FILE_LINE_FUNC;
				break;
			case notify_container_changed_action::reset:
				status.set(FileStatus::Dirty);
				std::ranges::for_each(notify.new_items, reactive_task_subscription);
				break;
		}
	}, Status.life());
}

void CToDoDoc::Open(const std::wstring& path)
{
	if (::PathFileExists(path.c_str())){
		Path.set(path);
		Tasks.clear();

		//Deserialize
		std::vector<MainTask> tempTasks;
		std::ifstream i(path);
		json j;
		i >> j;
		j.get_to(tempTasks);

		for (const auto& item : tempTasks) {
			Tasks.push_back(std::make_tuple(item));
		}
		Status.set(FileStatus::Saved);
	}
}
void CToDoDoc::Save(const std::wstring& path)
{
	Path.set(path);
	//Serialize
	try {
		std::vector<MainTask> tempTasks;
		for (auto iter = Tasks->cbegin(); iter != Tasks->cend(); ++iter) {
			tempTasks.push_back(std::get<MainTask>(*iter));
		}
		json j = tempTasks;
		std::ofstream o(path);
		o << std::setw(4) << j << std::endl;
		Status.set(FileStatus::Saved);
	}
	catch (/*_com_error &e*/...) {
	}

}
