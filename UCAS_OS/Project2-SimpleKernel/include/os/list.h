/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * Copyright (C) 2018 Institute of Computing
 * Technology, CAS Author : Han Shukai (email :
 * hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * Changelog: 2019-8 Reimplement queue.h.
 * Provide Linux-style doube-linked list instead of original
 * unextendable Queue implementation. Luming
 * Wang(wangluming@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * * * * * * * * * * * */

#ifndef INCLUDE_LIST_H_
#define INCLUDE_LIST_H_

#include <type.h>
#include <os/mm.h>

// double-linked list
//   TODO: use your own list design!!
typedef struct list_node
{
    struct list_node *next, *prev;
} list_node_t;

typedef list_node_t list_head;


#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	list_head name = LIST_HEAD_INIT(name)

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({          \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *)( (char *)__mptr - offsetof(type,member) );})


#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

static inline void init_list_head(list_head *list)
{
	list->next = list;
	list->prev = list;
}

// add
static inline void __list_add(list_head *new,list_head *prev,list_head *next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void list_add(list_head *new, list_head *head)
{
	__list_add(new, head, head->next);
}

static inline void list_add_tail(list_head *new, list_head *head)
{
	__list_add(new, head->prev, head);
}

// del
static inline void __list_del(list_head * prev, list_head * next)
{
	next->prev = prev;
	prev->next = next;
}

static inline void __list_del_entry(list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

static inline void list_del(list_head *entry)
{
	__list_del_entry(entry);
	entry->next = NULL;
	entry->prev = NULL;
}

static inline void list_move(list_head *list, list_head *head)
{
	__list_del_entry(list);
	list_add(list, head);
}

static inline void list_move_tail(list_head *list, list_head *head)
{
	__list_del(list->prev, list->next);
	list_add_tail(list, head);
}


// judge
static inline int list_is_first(list_head *list, const list_head *head)
{
	return list->prev == head;
}

static inline int list_is_last(list_head *list, list_head *head)
{
	return list->next == head;
}

static inline int list_empty(list_head *head)
{
	return head->next == head;
}

// fetch the keyth element from list
static inline list_head * list_fetch(list_head *head, long key){
	if(list_empty(head) || key == 0){
		return NULL;
	}
	list_head *iterator = (list_head *)kmalloc(sizeof(list_head));
	*iterator = *head;
	while (key-- && iterator->next != head->prev)
	{
		iterator = iterator->next;
	}
	return iterator->next;
}

#endif
