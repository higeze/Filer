#pragma once

enum class notify_container_changed_action
{
	push_back,
	insert,
	erase,
	replace,
	reset,
	Move
};

template<typename TContainer>
struct notify_container_changed_event_args
{
	notify_container_changed_action action;
	TContainer new_items;
	int new_starting_index = -1;
	TContainer old_items;
	int old_starting_index = -1;
	TContainer all_items;
};
