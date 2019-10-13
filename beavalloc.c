/*
 * @author Timothy O'Rourke
 * @date 4 October 2019
 * @brief CS 444 Operating Systems II Project 2
 */

#include "beavalloc.h"

static void *lower_mem_bound = NULL;
static void *upper_mem_bound = NULL;
static struct linked_list heap = {.head = NULL, .tail = NULL};

static uint8_t DEBUG = FALSE;

static void *make_block(size_t size);
static size_t determine_needed_bytes(size_t size);
static void initialize_new_block(struct block *new, size_t size, size_t bytes);
static int free_block_exists(size_t size);
static void *get_free_block(size_t size);
static void split_free_block(struct block *curr, size_t size);
static void coalesce_blocks(struct block *curr);
static void coalesce_right(struct block *curr);
static void coalesce_left(struct block *curr);
static void diagnostic_message(const char *message);

void *beavalloc(size_t size)
{
    void *data = NULL;
    if (size == (size_t)NULL) {
        if (DEBUG) { diagnostic_message("beavalloc: size = NULL"); }
        return NULL;
    }

    if (lower_mem_bound == NULL) {
        if (DEBUG) { diagnostic_message("beavalloc: base memory location set"); }
        lower_mem_bound = sbrk(0);
    }

    if (free_block_exists(size)) {
        data = get_free_block(size);
    }
    else {
        data = make_block(size);
    }

    return data;
}

static void *make_block(size_t size)
{
    size_t bytes = determine_needed_bytes(size);
    struct block *new = sbrk(bytes);

    if (DEBUG) { diagnostic_message("making new block..."); }

    if (new == (void *)-1) {
        if (DEBUG) { diagnostic_message("failed to allocate memory"); }
        errno = ENOMEM;
        return NULL;
    }
    
    upper_mem_bound = sbrk(0);
    
    initialize_new_block(new, size, bytes);
    
    if (DEBUG) { diagnostic_message("new block made!"); }
    return new->data;
}

static size_t determine_needed_bytes(size_t size)
{
    int remainder = (size + META_DATA) % MIN_MEM;
    int multiplier = (size + META_DATA) / MIN_MEM;
    if (remainder) {
        multiplier++;
    }

    if (DEBUG) { diagnostic_message("determing number of bytes required..."); }
    
    return MIN_MEM * multiplier;
}

static void initialize_new_block(struct block *new, size_t size, size_t bytes)
{
    if (DEBUG) { diagnostic_message("initializing new block..."); }
    new->next = NULL;
    new->free = FALSE;
    new->size = size;
    new->capacity = bytes - META_DATA;

    if (heap.head == NULL) {
        new->prev = NULL;
        heap.head = heap.tail = new;
    }
    else {
        new->prev = heap.tail;
        heap.tail->next = new;
        heap.tail = new;
    }

    new->data = new + 1;
}

static int free_block_exists(size_t size)
{
    struct block *curr = heap.head;
    while (curr != NULL) {
        if (curr->free && (curr->capacity - size > 0)) {
            if (DEBUG) { diagnostic_message("free block exists!"); }
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

            // If significant amount of extra memory, split into another free block.
            if (curr->capacity - size > size) {
                split_free_block(curr, size);
            }

            return curr->data;
        }
        curr = curr->next;
    }
    return NULL;
}

static void split_free_block(struct block *curr, size_t size)
{
    struct block *new_block = NULL;
    char *block_finder = (char *)curr;

    block_finder += META_DATA + size;
    new_block = (struct block *)block_finder;

    new_block->size = 0;
    new_block->capacity = curr->capacity - size - META_DATA;
    new_block->free = TRUE;
    new_block->prev = curr;
    new_block->next = curr->next;
    new_block->data = new_block + 1;

    curr->next = new_block;
    curr->size = size;
    curr->capacity = size;

    if (new_block->next == NULL)
        heap.tail = new_block;

    if (DEBUG) { diagnostic_message("free block split!"); }
}

void beavfree(void *ptr)
{
    if (ptr == NULL) {
        if (DEBUG) { diagnostic_message("beavfree: NULL pointer passed"); }
        return;
    }
    else {
        struct block *curr = heap.head;
        while (curr != NULL) {
            if (curr->data == ptr) {
                curr->free = TRUE;
                curr->size = 0;

                if (DEBUG) { diagnostic_message("beavfree: memory block freed!"); }

                if ( ((curr->prev != NULL) && (curr->prev->free == TRUE)) || ((curr->next != NULL) && (curr->next->free == TRUE)) ) {
                    if (DEBUG) { diagnostic_message("coalescing free blocks..."); }
                    coalesce_blocks(curr);
                }
                return;
            }
            curr = curr->next;
        }
    }
}

static void coalesce_blocks(struct block * curr)
{
    if ( (curr->prev != NULL && curr->prev->free == TRUE) && (curr->next != NULL && curr->next->free == TRUE) ) {   // Coalesce left and right.
        coalesce_right(curr);
        coalesce_left(curr);
    }
    else if (curr->prev != NULL && curr->prev->free == TRUE) {                                                      // Coalesce left.
        coalesce_left(curr);
    }
    else if (curr->next != NULL && curr->next->free == TRUE) {                                                      // Coalesce right.
        coalesce_right(curr);
    }
}

static void coalesce_right(struct block *curr)
{
    if (DEBUG) { diagnostic_message("coalesce right..."); }
    if (curr->next->next == NULL) {
        heap.tail = curr;
    }
    else {
        curr->next->next->prev = curr;
    }
    curr->capacity += curr->next->capacity + META_DATA;
    curr->next = curr->next->next;
}

static void coalesce_left(struct block *curr)
{
    if (DEBUG) { diagnostic_message("coalesce left..."); }
    if (curr->next == NULL) {
        heap.tail = curr->prev;
    }
    else {
        curr->next->prev = curr->prev;
    }
    curr->prev->capacity += curr->capacity + META_DATA;
    curr->prev->next = curr->next;
}

void beavalloc_reset(void)
{
    brk(lower_mem_bound);
    lower_mem_bound = NULL;
    upper_mem_bound = NULL;
    heap.head = NULL;
    heap.tail = NULL;

    if (DEBUG) { diagnostic_message("heap reset!"); }
}

void beavalloc_set_verbose(uint8_t v)
{
    DEBUG = v;
}

void *beavcalloc(size_t nmemb, size_t size)
{
    void *data = NULL;
    if (nmemb == 0 || size == 0)
        return NULL;

    data = beavalloc(nmemb * size);
    memset(data, 0, nmemb * size);
    return data;
}

void *beavrealloc(void *ptr, size_t size)
{
    void *new_data = NULL;
    struct block *ptr_block = NULL;
    struct block *curr = NULL;

    if (size == (size_t)NULL)
        return NULL;

    if (ptr == NULL) {
        new_data = beavalloc(size * 2);
    }
    else {
        for (curr = heap.head; curr != NULL; curr = curr->next) {   // Find block that owns this data.
            if (curr->data == ptr) {
                ptr_block = curr;
            }
        }

        if (ptr_block == NULL) {
            if (DEBUG) { diagnostic_message("beavrealloc: invalid address given"); }
            return NULL;
        }

        if (ptr_block->capacity >= size) {  // Can just decrease used space.
            if (DEBUG) { diagnostic_message("beavrealloc: decreasing used space of block..."); }
            ptr_block->size = size;
            new_data = ptr_block->data;
        }
        else {                              // Allocate new block.
            if (DEBUG) { diagnostic_message("beavrealloc: allocating new block..."); }
            new_data = beavalloc(size);
            memcpy(new_data, ptr, ptr_block->size);
            beavfree(ptr);
        }
        
    }
    
    return new_data;
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

static void diagnostic_message(const char *message)
{
    fprintf(stderr, message);
    fprintf(stderr, "\n");
}