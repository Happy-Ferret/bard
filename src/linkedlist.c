#include <string.h>

#include "linkedlist.h"

void ll_init(LinkedList* list, size_t elementSize)
{
	list->elementSize = elementSize;
	list->length = 0;
	list->first = list->last;
}

void ll_delete(LinkedList* list)
{
	for(size_t i = 0; i < list->length; i++)
		ll_remove(list, 0);
}

static struct llElement* construct(size_t elementSize, void* data)
{
	void* newDat = malloc(elementSize);
	memcpy(newDat, data, elementSize);
	struct llElement* elem = (struct llElement*)calloc(1, sizeof(struct llElement));
	if(elem == NULL)
		return NULL;
	elem->data = newDat;
	return elem;
}

int ll_insert(LinkedList* list, size_t index, void* data)
{
	struct llElement* elem = construct(list->elementSize, data);
	if(elem == NULL)
		return 1;

	struct llElement* prev = NULL;
	struct llElement* cur = list->first;
	for(size_t i = 0; i < index; i++)
	{
		prev = cur;
		cur = cur->next;
	}
	if(prev == NULL)
		list->first = elem;
	else
		prev->next = elem;
	elem->next = cur;
	list->length++;

	return 0;
}

void* ll_get(LinkedList* list, size_t index)
{
	struct llElement* cur = list->first;
	for(size_t i = 0; i < index; i++)
		cur = cur->next;
	return cur->data;
}

void ll_foreach(LinkedList* list, Callback cb, void* userdata)
{
	struct llElement* cur = list->first;
	for(size_t i = 0; i < list->length; i++)
	{
		if(!cb(cur->data, userdata))
			break;
		cur = cur->next;
	}
}

void ll_remove(LinkedList* list, size_t index)
{
	struct llElement* prev = NULL;
	struct llElement* cur = list->first;
	for(size_t i = 0; i < index; i++)
	{
		prev = cur;
		cur = cur->next;
	}
	if(prev == NULL)
		list->first = cur->next;
	else
		prev->next = cur->next;
	free(cur->data);
	free(cur);
	list->length--;
}

int ll_size(LinkedList* list)
{
	return list->length;
}