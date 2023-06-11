#include "ToDoDoc.h"

void CToDoDoc::Open(const std::wstring& path)
{
	if (::PathFileExists(path.c_str())){
		Path.set(path);
		Tasks.clear();

		//Deserialize
		try {
			//Serialize
			std::vector<MainTask> tempTasks;
			std::ifstream i(path);
			json j;
			i >> j;
			j.get_to(tempTasks);

			for (const auto& item : tempTasks) {
				Tasks.push_back(std::make_tuple(item));
			}
			Status = FileStatus::Saved;
		}
		catch (/*_com_error &e*/...) {
		}
	}
}
void CToDoDoc::Save(const std::wstring& path)
{
	Path.set(path);
	//Serialize
	try {
		std::vector<MainTask> tempTasks;
		for (auto iter = Tasks.cbegin(); iter != Tasks.cend(); ++iter) {
			tempTasks.push_back(std::get<MainTask>(*iter));
		}
		json j = tempTasks;
		std::ofstream o(path);
		o << std::setw(4) << j << std::endl;
		Status = FileStatus::Saved;
	}
	catch (/*_com_error &e*/...) {
	}

}
