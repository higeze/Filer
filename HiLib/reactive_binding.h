#pragma once
#include "reactive_property.h"
#include "reactive_command.h"
#include "reactive_vector.h"
#include <functional>

template<class T>
auto reactive_binding(reactive_property_ptr<T>& src, reactive_property_ptr<T>& dst) -> std::pair<sigslot::connection, sigslot::connection>
{
    dst->set(src->get_const());

    return std::make_pair(
        dst->subscribe(&reactive_property<T>::set, src),
        src->subscribe(&reactive_property<T>::set, dst));
}

template<class T>
auto reactive_command_binding(reactive_command_ptr<T> src, reactive_command_ptr<T> dst)->sigslot::connection
{
	return dst->subscribe(&reactive_command<T>::execute, src);
}

template<class T, class Allocator>
auto reactive_vector_binding(reactive_vector_ptr<T, Allocator>& src, reactive_vector_ptr<T, Allocator>& dst)
-> std::pair<sigslot::connection, sigslot::connection>
{
		//auto one_side_binding = [item_clone = std::forward<ItemClone>(clone), item_bind = std::forward<ItemClone>(clone)](
		//	reactive_vector_ptr<std::tuple<MainTask>>& source,
		//	reactive_vector_ptr<std::tuple<MainTask>>& destination)->sigslot::connection {

		//	return source->subscribe(
	//auto observer = [item_clone = std::forward<ItemClone>(clone), item_bind = std::forward<ItemBind>(bind), destination = dst]
	//(notify_vector_changed_event_args<T> notify)->void
	//{
	//	destination->block_subject();

	//	switch (notify.action) {
	//		case notify_vector_changed_action::push_back:
	//			destination->push_back(item_clone(notify.new_items.front()));
	//			item_bind(
	//				notify.new_items.front(),
	//				destination->get_unconst().operator[](notify.new_starting_index));
	//			break;
	//		case notify_vector_changed_action::insert:
	//			destination->insert(destination->get_const().cbegin() + notify.new_starting_index, item_clone(notify.new_items.front()));
	//			item_bind(
	//				notify.new_items.front(),
	//				destination->get_unconst().operator[](notify.new_starting_index));
	//			break;
	//		case notify_vector_changed_action::Move:
	//			THROW_FILE_LINE_FUNC;
	//			break;
	//		case notify_vector_changed_action::erase:
	//			destination->erase(destination->cbegin() + notify.old_starting_index);
	//			break;
	//		case notify_vector_changed_action::replace:
	//			THROW_FILE_LINE_FUNC;
	//			break;
	//		case notify_vector_changed_action::reset:
	//			for (size_t i = 0; i < notify.new_items.size(); i++) {
	//				destination->push_back(item_clone(notify.new_items[i]));
	//				item_bind(
	//					notify.new_items.operator[](i),
	//					destination->get_unconst().operator[](i));
	//			}
	//			break;
	//	}
	//	destination->unblock_subject();
	//};
		//	);
		//};

	return std::make_pair(
		dst->subscribe(&reactive_vector<T, Allocator>::observe, src),
        src->subscribe(&reactive_vector<T, Allocator>::observe, dst));
}