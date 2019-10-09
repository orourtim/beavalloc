/*
 * @author Timothy O'Rourke
 * @date 4 October 2019
 * @brief CS 444 Operating Systems II Project 2
 */

#include "beavalloc.h"

static void *lower_mem_bound = NULL;
static void *upper_mem_bound = NULL;
static struct linked_list heap = {.head = NULL, .tail = NULL};

static void *make_block(size_t size);
static int free_block_exists(size_t size);
static void *get_free_block(size_t size);

void *beavalloc(size_t size)
{
    void *data = NULL;
    if (size == (size_t)NULL) {
        return NULL;
    }

    if (lower_mem_bound == NULL) {
        lower_mem_bound = sbrk(0);
    }

    // Check for free block.
    if (free_block_exists(size)) {
        data = get_free_block(size);
    }
    else { // If no free block, sbrk.
        data = make_block(size);
    }

    return data;
}

static void *make_block(size_t size)
{
    struct block *new = sbrk(MIN_MEM);
    
    upper_mem_bound = new + sizeof(*new);

    new->next = NULL;
    new->free = FALSE;
    new->size = size;
    new->capacity = MIN_MEM - META_DATA;
    if (heap.head == NULL) {
        new->prev = NULL;
        heap.head = heap.tail = new;
    }
    else {
        new->prev = heap.tail;
        heap.tail->next = new;
        heap.tail = new;
    }

    new->data = new + META_DATA;
    return new->data;
}

static int free_block_exists(size_t size)
{
    struct block *curr = heap.head;
    while (curr != NULL) {
        if (curr->free && (curr->capacity - size > 0)) {
            return TRUE;
        }
        curr = curr->next;
    }
    return FALSE;
}

static void *get_free_block(size_t size)
{
    struct block *curr = heap.head;
    while (curr != NULL) {
        if (curr->free && (curr->capacity - size > 0)) {
            curr->free = FALSE;
            curr->size = size;

            // If significant amount of extra memory, create another free block.
            if (curr->capacity - size > size) {
                void *end = curr + sizeof(*curr);
                end -= curr->capacity - size;
                struct block *new_block = (struct block *)end;
                new_block->size = 0;
                new_block->free = TRUE;
                new_block->prev = curr;
                new_block->next = curr->next;
                curr->next = new_block;
                new_block->capacity = curr->capacity - size - META_DATA;
                curr->capacity = size;
                new_block->data = new_block + META_DATA;
            }
            else {
                return curr->data;
            }
        }
    }
    return NULL;
}

void beavfree(void *ptr)
{
    if (ptr == NULL) {
        return;
    }
    else {
        struct block *curr = heap.head;
        while (curr != NULL) {
            if (curr->data == ptr) {
                curr->free = TRUE;
                curr->size = 0;
                return;
            }
            curr = curr->next;
        }
    }
}

void beavalloc_reset(void)
{
    brk(lower_mem_bound);
    lower_mem_bound = NULL;
    upper_mem_bound = NULL;
    heap.head = NULL;
    heap.tail = NULL;
}

void beavalloc_set_verbose(uint8_t v)
{

}

void *beavcalloc(size_t nmemb, size_t size)
{
    return NULL;
}

void *beavrealloc(void *ptr, size_t size)
{
    return NULL;
}

void beavalloc_dump(uint leaks_only)
{
    struct block *curr = NULL;
    uint i = 0;
    uint leak_count = 0;
    uint user_bytes = 0;
    uint capacity_bytes = 0;
    uint block_bytes = 0;
    uint used_blocks = 0;
    uint free_blocks = 0;

    if (leaks_only) {
        fprintf(stderr, "heap lost blocks\n");
    }
    else {
        fprintf(stderr, "heap map\n");
    }
    fprintf(stderr
            , "  %s\t%s\t%s\t%s\t%s" 
            "\t%s\t%s\t%s\t%s\t%s\t%s"
            "\n"
            , "blk no  "
            , "block add "
            , "next add  "
            , "prev add  "
            , "data add  "
            
            , "blk off  "
            , "dat off  "
            , "capacity "
            , "size     "
            , "blk size "
            , "status   "
        );
    for (curr = heap.head, i = 0; curr != NULL; curr = curr->next, i++) {
        if (leaks_only == FALSE || (leaks_only == TRUE && curr->free == FALSE)) {
            fprintf(stderr
                    , "  %u\t\t%9p\t%9p\t%9p\t%9p\t%u\t\t%u\t\t"
                      "%u\t\t%u\t\t%u\t\t%s\t%c\n"
                    , i
                    , curr
                    , curr->next
                    , curr->prev
                    , curr->data
                    , (unsigned) ((void *) curr - lower_mem_bound)
                    , (unsigned) ((void *) curr->data - lower_mem_bound)
                    , (unsigned) curr->capacity
                    , (unsigned) curr->size
                    , (unsigned) (curr->capacity + META_DATA)
                    , curr->free ? "free  " : "in use"
                    , curr->free ? '*' : ' '
                );
            user_bytes += curr->size;
            capacity_bytes += curr->capacity;
            block_bytes += curr->capacity + sizeof(struct block);
            if (curr->free == FALSE && leaks_only == TRUE) {
                leak_count++;
            }
            if (curr->free == TRUE) {
                free_blocks++;
            }
            else {
                used_blocks++;
            }
        }
    }
    if (leaks_only) {
        if (leak_count == 0) {
            fprintf(stderr, "  *** No leaks found!!! That does NOT mean no leaks are possible. ***\n");
        }
        else {
            fprintf(stderr
                    , "  %s\t\t\t\t\t\t\t\t\t\t\t\t"
                      "%u\t\t%u\t\t%u\n"
                    , "Total bytes lost"
                    , capacity_bytes
                    , user_bytes
                    , block_bytes
                );
        }
    }
    else {
        fprintf(stderr
                , "  %s\t\t\t\t\t\t\t\t\t\t\t\t"
                "%u\t\t%u\t\t%u\n"
                , "Total bytes used"
                , capacity_bytes
                , user_bytes
                , block_bytes
            );
        fprintf(stderr, "  Used blocks: %u  Free blocks: %u  "
             "Min heap: %p    Max heap: %p\n"
               , used_blocks, free_blocks
               , lower_mem_bound, upper_mem_bound
            );
    }
}