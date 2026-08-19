#ifndef _LIST_H
#define _LIST_H

/*
VS2019 not support typeof()
*/
/* Stripped down implementation of linked list taken
 * from the Linux Kernel.
 */

/*
 * Simple doubly linked list implementation.
 *
 * Some of the internal functions ("__xxx") are useful when
 * manipulating whole lists rather than single entries, as
 * sometimes we already know the next/prev entries and we can
 * generate better code by using them directly rather than
 * using the generic single-entry routines.
 */
#include <stddef.h>
struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head *list)
{
	list->next = list;
	list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_add(struct list_head *pnew,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = pnew;
	pnew->next = next;
	pnew->prev = prev;
	prev->next = pnew;
}

/**
 * list_add - add a new entry
 * @pnew: pnew entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
static inline void list_add(struct list_head *pnew, struct list_head *head)
{
	__list_add(pnew, head, head->next);
}

/**
 * list_add_tail - add a new entry
 * @pnew: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void list_add_tail(struct list_head *pnew, struct list_head *head)
{
    __list_add(pnew, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __list_del(struct list_head * prev, struct list_head * next)
{
	next->prev = prev;
	prev->next = next;
}


/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void __list_del_entry(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
}

static inline void list_del(struct list_head *entry)
{
	__list_del(entry->prev, entry->next);
	INIT_LIST_HEAD(entry);
}

/**
 * list_replace - replace old entry by new one
 * @old: the element to be replaced
 * @pnew: the new element to insert
 *
 * If @old was empty, it will be overwritten.
 */
static inline void list_replace(struct list_head* old,
	struct list_head* pnew)
{
	pnew->next = old->next;
	pnew->next->prev = pnew;
	pnew->prev = old->prev;
	pnew->prev->next = pnew;
}

/**
 * list_is_first -- tests whether @list is the first entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int list_is_first(const struct list_head* list,
	const struct list_head* head)
{
	return list->prev == head;
}

/**
 * list_is_last - tests whether @list is the last entry in list @head
 * @list: the entry to test
 * @head: the head of the list
 */
static inline int list_is_last(const struct list_head* list,
	const struct list_head* head)
{
	return list->next == head;
}

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int list_empty(const struct list_head* head)
{
	return head->next == head;
}
#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/**
  * container_of - cast a member of a structure out to the containing structure
  * @ptr:	the pointer to the member.
  * @type:	the type of the container struct this is embedded in.
  * @member:	the name of the member within the struct.
  *
  */
/*
#define container_of(ptr, type, member) ({			\
	const typeof( ((type *)0)->member ) *__mptr = (ptr);	\
	(type *)( (char *)__mptr - offsetof(type,member) );})
*/

#define container_of(ptr, type, member) (			\
	(type *)( (char *)(ptr) - offsetof(type,member) ))

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_head within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)
/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

 /**
  * list_for_each_safe - iterate over a list safe against removal of list entry
  * @pos:	the &struct list_head to use as a loop cursor.
  * @n:		another &struct list_head to use as temporary storage
  * @head:	the head for your list.
  */
#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
		pos = n, n = pos->next)

/**
 * list_for_each_entry    -    iterate over list of given type
 * @pos:    the type * to use as a loop cursor.
 * @head:    the head for your list.
 * @member:    the name of the list_struct within the struct.
 */
#define list_for_each_entry(pos, head, member, type)                \
    for (pos = list_entry((head)->next, type, member);    \
         &pos->member != (head);     \
         pos = list_entry(pos->member.next, type , member))

 /**
  * list_for_each_entry_safe - iterate over list of given type safe against removal of list entry
  * @pos:	the type * to use as a loop cursor.
  * @n:		another type * to use as temporary storage
  * @head:	the head for your list.
  * @member:	the name of the list_head within the struct.
  */
#define list_for_each_entry_safe(pos, n, head, member, type)			\
	for (pos = list_first_entry(head, type, member),	\
		n = list_next_entry(pos, type, member);			\
	     !list_entry_is_head(pos, head, member); 			\
	     pos = n, n = list_next_entry(n, type, member))


 
 /**
  * list_first_entry - get the first element from a list
  * @ptr:	the list head to take the element from.
  * @type:	the type of the struct this is embedded in.
  * @member:	the name of the list_head within the struct.
  *
  * Note, that list is expected to be not empty.
  */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

  /**
   * list_last_entry - get the last element from a list
   * @ptr:	the list head to take the element from.
   * @type:	the type of the struct this is embedded in.
   * @member:	the name of the list_head within the struct.
   *
   * Note, that list is expected to be not empty.
   */
#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

   /**
	* list_next_entry - get the next element in list
	* @pos:	the type * to cursor
	* @member:	the name of the list_head within the struct.
	*/
#define list_next_entry(pos, type, member) \
	list_entry((pos)->member.next, type, member)

	/**
	 * list_prev_entry - get the prev element in list
	 * @pos:	the type * to cursor
	 * @member:	the name of the list_head within the struct.
	 */

#define list_prev_entry(pos, type, member) \
	list_entry((pos)->member.prev, type, member)

	 /**
	  * list_entry_is_head - test if the entry points to the head of the list
	  * @pos:	the type * to cursor
	  * @head:	the head for your list.
	  * @member:	the name of the list_head within the struct.
	  */
#define list_entry_is_head(pos, head, member)				\
	(&pos->member == (head))

#endif