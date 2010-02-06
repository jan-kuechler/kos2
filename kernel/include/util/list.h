#ifndef UTIL_LIST_H
#define UTIL_LIST_H

#include <stdbool.h>
#include <stddef.h>

/** A list entry */
typedef struct list_entry
{
	void *data;
	struct list_entry *next, *prev;
} list_entry_t;

/** A list */
typedef struct list
{
	list_entry_t *front, *back;
	size_t size;
} list_t;

#define STATIC_LIST(name) \
	static list_t __##name = {NULL, NULL, 0}; \
	static list_t *name = &__##name

/**
 * Initializes a list.
 * @param list The list
 */
inline void list_init(list_t *list)
{
	list->front = NULL;
	list->back = NULL;
	list->size = 0;
}

/**
 * Creates and initializes a new list.
 * @return The list
 */
list_t *list_create();

/**
 * Destroys a list
 * @param list The list
 */
void list_destroy(list_t *list);

bool list_empty(list_t *list);
size_t list_size(list_t *list);

list_t *list_add_front(list_t *list, void *data);
list_t *list_add_back(list_t *list, void *data);
list_t *list_add_after(list_t *list, list_entry_t *entry, void *data);

void *list_del_front(list_t *list);
void *list_del_back(list_t *list);
void *list_del_entry(list_t *list, list_entry_t *entry);

void *list_front(list_t *list);
void *list_back(list_t *list);

#define list_iterate(pos, list) for (pos = (list)->front; pos != 0; pos = pos->next)

#define list_riterate(pos, list) for (pos = (list)->back; pos != 0; pos = pos->prev)

#endif /*UTIL_LIST_H*/
