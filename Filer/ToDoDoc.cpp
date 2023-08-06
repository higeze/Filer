#include "ToDoDoc.h"
#include "Debug.h"

CToDoDoc::CToDoDoc()
	:Status(make_reactive_property<FileStatus>(FileStatus::None)),
	Path(make_reactive_property<std::wstring>()),
	Tasks(make_reactive_vector<std::tuple<MainTask>>())
{
	auto reactive_task_subscription = [this](MainTask& src)->std::vector<sigslot::connection> {
		auto dirty = [this](auto) { Status->set(FileStatus::Dirty); };
		std::vector<sigslot::connection> subs;
		subs.push_back(src.State->subscribe(dirty, Status));
		subs.push_back(src.Name->subscribe(dirty, Status));
		subs.push_back(src.Memo->subscribe(dirty, Status));
		subs.push_back(src.YearMonthDay->subscribe(dirty, Status));
		return subs;
	};

	Tasks->subscribe([this, reactive_task_subscription](const notify_container_changed_event_args<std::vector<std::tuple<MainTask>>>& notify) {
		switch (notify.action) {
			case notify_container_changed_action::push_back:
			case notify_container_changed_action::insert:
			{
				Status->set(FileStatus::Dirty);
				reactive_task_subscription(std::get<MainTask>(Tasks->get_unconst().operator[](notify.new_starting_index)));
				break;
			}
			case notify_container_changed_action::Move:
				THROW_FILE_LINE_FUNC;
				break;
			case notify_container_changed_action::erase:
				Status->set(FileStatus::Dirty);
				break;
			case notify_container_changed_action::replace:
				THROW_FILE_LINE_FUNC;
				break;
			case notify_container_changed_action::reset:
				Status->set(FileStatus::Dirty);
				for (size_t i = 0; i < notify.new_items.size(); i++) {
					reactive_task_subscription(std::get<MainTask>(Tasks->get_unconst().operator[](i)));
				}
				break;
		}
	}, Status);
}

void CToDoDoc::Open(const std::wstring& path)
{
	if (::PathFileExists(path.c_str())){
		Path->set(path);
		Tasks->clear();

		//Deserialize
		std::vector<MainTask> tempTasks;
		std::ifstream i(path);
		json j;
		i >> j;
		j.get_to(tempTasks);

		for (const auto& item : tempTasks) {
			Tasks->push_back(std::make_tuple(item));
		}
		Status->set(FileStatus::Saved);
	}
}
void CToDoDoc::Save(const std::wstring& path)
{
	Path->set(path);
	//Serialize
	try {
		std::vector<MainTask> tempTasks;
		for (auto iter = Tasks->cbegin(); iter != Tasks->cend(); ++iter) {
			tempTasks.push_back(std::get<MainTask>(*iter));
		}
		json j = tempTasks;
		std::ofstream o(path);
		o << std::setw(4) << j << std::endl;
		Status->set(FileStatus::Saved);
	}
	catch (/*_com_error &e*/...) {
	}

}
