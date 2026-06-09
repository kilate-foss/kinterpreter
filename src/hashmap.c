#include "kilate/hashmap.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kilate/error.h"
#include "kilate/string.h"

hashmap_t *
hm_make (size_t item_size)
{
        hashmap_t *hm = malloc (sizeof (hashmap_t));
        hm->item_size = item_size;
        hm->capacity = 64;
        hm->itens = vector_make (sizeof (hm_entry_t *));
        for (size_t i = 0; i < hm->capacity; i++)
        {
                hm_entry_t *null_ptr = NULL;
                vector_push_back (hm->itens, &null_ptr);
        }
        return hm;
}

void
hm_delete (hashmap_t *self)
{
        for (size_t i = 0; i < self->itens->size; ++i)
        {
                hm_entry_t *item = *(hm_entry_t **)vector_get (self->itens, i);
                if (item != NULL)
                {
                        free (item->key);
                        free (item->value);
                        free (item);
                }
        }
        vector_delete (self->itens);
        free (self);
}

unsigned int
hm_hash (hashmap_t *self, char *key)
{
        if (self == NULL)
                error_fatal ("Hashmap is null.");
        if (key == NULL)
                error_fatal ("Key is null.");

        unsigned int hash = 5381;
        int c;
        while ((c = *key++))
        {
                hash = ((hash << 5) + hash) + c; // hash * 33 + c
        }
        return hash % self->capacity;
}
void *
hm_get (hashmap_t *self, char *key)
{
        if (self == NULL)
                error_fatal ("Hashmap is null.");
        if (key == NULL)
                error_fatal ("Key is null.");

        unsigned int index = hm_hash (self, key);

        hm_entry_t **item_ptr = (hm_entry_t **)vector_get (self->itens, index);
        hm_entry_t *item = *item_ptr;

        while (item)
        {
                if (str_equals (item->key, key))
                {
                        return item->value;
                }
                item = item->next;
        }
        return NULL;
}

void
hm_put (hashmap_t *self, char *key, void *value)
{
        if (self == NULL)
                error_fatal ("Hashmap is null.");
        if (key == NULL)
                error_fatal ("Key is null.");

        unsigned int index = hm_hash (self, key);
        hm_entry_t **head_ptr = (hm_entry_t **)vector_get (self->itens, index);
        hm_entry_t *head = *head_ptr;

        hm_entry_t *item = head;
        while (item)
        {
                if (str_equals (item->key, key))
                {
                        memcpy (item->value, value, self->item_size);
                        return;
                }
                item = item->next;
        }

        hm_entry_t *new_item = malloc (sizeof (hm_entry_t));
        new_item->key = strdup (key);
        new_item->value = malloc (self->item_size);
        memcpy (new_item->value, value, self->item_size);
        new_item->next = head;

        *head_ptr = new_item;
}
