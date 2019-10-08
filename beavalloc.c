/*
 * @author Timothy O'Rourke
 * @date 4 October 2019
 * @brief CS 444 Operating Systems II Project 2
 */

#include "beavalloc.h"

void *beavalloc(size_t size)
{
    if (size == NULL) {
        return NULL;
    }

    struct block *new = sbrk(MIN_MEM);

    if (heap.head == NULL) {
        new->next = NULL;
        new->prev = NULL;
        new->size = MIN_MEM;
        new->available = TRUE;
        heap.head = heap.tail = new;
    }
    else {
        new->prev = heap.tail;
        new->next = NULL;
        new->size = MIN_MEM;
        new->available = TRUE;
        heap.tail = new;
    }
}

void beavfree(void *ptr)
{

}

void beavalloc_reset(void)
{

}

void beavalloc_set_verbose(uint8_t v)
{

}

void *beavcalloc(size_t nmemb, size_t size)
{

}

void *beavrealloc(void *ptr, size_t size)
{

}

void beavalloc_dump(uint leaks_only)
{
    // struct block_list *curr = NULL;
    // uint i = 0;
    // uint leak_count = 0;
    // uint user_bytes = 0;
    // uint capacity_bytes = 0;
    // uint block_bytes = 0;
    // uint used_blocks = 0;
    // uint free_blocks = 0;

    // if (leaks_only) {
    //     fprintf(stderr, "heap lost blocks\n");
    // }
    // else {
    //     fprintf(stderr, "heap map\n");
    // }
    // fprintf(stderr
    //         , "  %s\t%s\t%s\t%s\t%s" 
    //         "\t%s\t%s\t%s\t%s\t%s\t%s"
    //         "\n"
    //         , "blk no  "
    //         , "block add "
    //         , "next add  "
    //         , "prev add  "
    //         , "data add  "
            
    //         , "blk off  "
    //         , "dat off  "
    //         , "capacity "
    //         , "size     "
    //         , "blk size "
    //         , "status   "
    //     );
    // for (curr = block_list, i = 0; curr != NULL; curr = curr->next, i++) {
    //     if (leaks_only == FALSE || (leaks_only == TRUE && curr->free == FALSE)) {
    //         fprintf(stderr
    //                 , "  %u\t\t%9p\t%9p\t%9p\t%9p\t%u\t\t%u\t\t"
    //                   "%u\t\t%u\t\t%u\t\t%s\t%c\n"
    //                 , i
    //                 , curr
    //                 , curr->next
    //                 , curr->prev
    //                 , curr->data
    //                 , (unsigned) ((void *) curr - lower_mem_bound)
    //                 , (unsigned) ((void *) curr->data - lower_mem_bound)
    //                 , (unsigned) curr->capacity
    //                 , (unsigned) curr->size
    //                 , (unsigned) (curr->capacity + sizeof(struct block_list))
    //                 , curr->free ? "free  " : "in use"
    //                 , curr->free ? '*' : ' '
    //             );
    //         user_bytes += curr->size;
    //         capacity_bytes += curr->capacity;
    //         block_bytes += curr->capacity + sizeof(struct block_list);
    //         if (curr->free == FALSE && leaks_only == TRUE) {
    //             leak_count++;
    //         }
    //         if (curr->free == TRUE) {
    //             free_blocks++;
    //         }
    //         else {
    //             used_blocks++;
    //         }
    //     }
    // }
    // if (leaks_only) {
    //     if (leak_count == 0) {
    //         fprintf(stderr, "  *** No leaks found!!! That does NOT mean no leaks are possible. ***\n");
    //     }
    //     else {
    //         fprintf(stderr
    //                 , "  %s\t\t\t\t\t\t\t\t\t\t\t\t"
    //                   "%u\t\t%u\t\t%u\n"
    //                 , "Total bytes lost"
    //                 , capacity_bytes
    //                 , user_bytes
    //                 , block_bytes
    //             );
    //     }
    // }
    // else {
    //     fprintf(stderr
    //             , "  %s\t\t\t\t\t\t\t\t\t\t\t\t"
    //             "%u\t\t%u\t\t%u\n"
    //             , "Total bytes used"
    //             , capacity_bytes
    //             , user_bytes
    //             , block_bytes
    //         );
    //     fprintf(stderr, "  Used blocks: %u  Free blocks: %u  "
    //          "Min heap: %p    Max heap: %p\n"
    //            , used_blocks, free_blocks
    //            , lower_mem_bound, upper_mem_bound
    //         );
    // }
}