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

template<template<typename...> typename tcontainer_type, typename tvalue_type, typename... targs_type >
struct notify_container_changed_event_args
{
	notify_container_changed_action action;
	tcontainer_type<tvalue_type, targs_type...> new_items;
	int new_starting_index = -1;
	tcontainer_type<tvalue_type, targs_type...> old_items;
	int old_starting_index = -1;
	tcontainer_type<tvalue_type, targs_type...>& all_items;
};

//template<template<typename...> typename tcontainer_type, typename tvalue_type, typename... targs_type >
//struct notify_container_changed_event_args<tcontainer_type, tvalue_type*, targs_type...>
//{
//	notify_container_changed_action action;
//	tcontainer_type<tvalue_type*, targs_type...> new_items;
//	int new_starting_index = -1;
//	tcontainer_type<tvalue_type*, targs_type...> old_items;
//	int old_starting_index = -1;
//	tcontainer_type<tvalue_type*, targs_type...>& all_items;
//};
