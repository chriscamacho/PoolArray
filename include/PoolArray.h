/** @file */
/*
 * PoolArray
 *
 * tracks an array of structs, allowing rapid access to an unused item
 *
 *
 * Copyright (c) 2019, Chris Camacho (codifies) http://bedroomcoders.co.uk/captcha/
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
 * IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef POOLARRAY_H
#define POOLARRAY_H

/** @mainpage
 * see @link PoolArray.h PoolArray.h @endlink for the api description
 * quick start</h4>
 * 
 * <p><b>#define POOLARRAY_IMPL</b></p>
 * One source file in your project (only one) must define this
 * to include the actual code for the functions.
 *
 * <p>assuming you have an array of struct
 * @code
 * typedef struct testItem {
 *     int ttl;    // time to live
 *     int id;
 * } testItem;
 * @endcode
 * first initialise the pool
 * @code
 * PoolArray pool;             // the pool to manage the array
 * testItem items[poolSize];   // the array we're managing
 *
 * // you MUST check the return value
 * if (!PoolArrayInit(&pool, &items, sizeof(testItem), poolSize)) {
 *     printf("unable to initialise pool array\n");
 *     return -1;
 * }
 * @endcode
 * When you need to use an item in the pool
 * @code
 * testItem* it = PoolArrayGetNextItem(&pool);
 * if (it == NULL) {
 *     printf("!!!! Pool used up couldn't grab a new item\n");
 * } else {
 *     // set up the new item...
 * }
 * @endcode
 * when you need to update your active item in the pool
 * you can use a simple for loop
 * @code
 * for(PoolArrayNode* node = pool.activeList; node; node = node->next) {
 *     testItem* item = (testItem*)node->userData;
 *     ...
 * }
 * @endcode
 * however if you need to retire a node within a loop you must first
 * find out the next node...
 * @code
 * PoolArrayNode* node = pool.activeList;
 * while( node ) {
 *     // update the item
 *     testItem* item = (testItem*)node->userData;
 *     PoolArrayNode* nextNode = node->next;
 *     ...
 *
 *     if (checkItem(item)) {
 *         PoolArrayRetireNode(&pool, node);
 *     }
 *     node = nextNode;
 * }
 * @endcode
 * don't forget to destroy the pool when you have finished with it...
 */


#include <stdlib.h> /**< for malloc */

// needs to be doubly linked, so as to avoid the need to scan
// through a list when removing a node from the list
typedef struct PoolArrayNode
{
    void* userData;                 /**< pointer to an array item */
    struct PoolArrayNode* next;     /**< pointer to next node in list */
    struct PoolArrayNode* prev;     /**< pointer to previous node in list */
} PoolArrayNode;

typedef struct PoolArray
{
    size_t Available;               /**< number of items in available list */
    size_t Size;                    /**< total number of items in pool */

    PoolArrayNode* availableList;   /**< list of available unused items */
    PoolArrayNode* activeList;      /**< list of in use items */
    PoolArrayNode* nodes;           /**< storage for tracking nodes */

} PoolArray;

bool PoolArrayInit(PoolArray* pa, void* array, size_t size, size_t total);
void PoolArrayDestroy(PoolArray* pa);
void PoolArrayRetireNode(PoolArray* poolArray, PoolArrayNode* node);
void PoolArrayRetireAll(PoolArray* poolArray);
void* PoolArrayGetNextItem(PoolArray* poolArray);



#ifdef POOLARRAY_IMPL

// list macros DL_APPEND and DL_DELETE abstracted from.....
/*
Copyright (c) 2007-2018, Troy D. Hanson   http://troydhanson.github.com/uthash/
All rights reserved.
Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#define DL_APPEND(head,add)                                                    \
    DL_APPEND2(head,add,prev,next)

#define DL_APPEND2(head,add,prev,next)                                         \
do {                                                                           \
  if (head) {                                                                  \
      (add)->prev = (head)->prev;                                              \
      (head)->prev->next = (add);                                              \
      (head)->prev = (add);                                                    \
      (add)->next = NULL;                                                      \
  } else {                                                                     \
      (head)=(add);                                                            \
      (head)->prev = (head);                                                   \
      (head)->next = NULL;                                                     \
  }                                                                            \
} while (0)


#define DL_DELETE(head,del)                                                    \
    DL_DELETE2(head,del,prev,next)


// these assert's have been removed from the start of DL_DELETE2
//  assert((head) != NULL);
//  assert((del)->prev != NULL);


#define DL_DELETE2(head,del,prev,next)                                         \
do {                                                                           \
  if ((del)->prev == (del)) {                                                  \
      (head)=NULL;                                                             \
  } else if ((del)==(head)) {                                                  \
      (del)->next->prev = (del)->prev;                                         \
      (head) = (del)->next;                                                    \
  } else {                                                                     \
      (del)->prev->next = (del)->next;                                         \
      if ((del)->next) {                                                       \
          (del)->next->prev = (del)->prev;                                     \
      } else {                                                                 \
          (head)->prev = (del)->prev;                                          \
      }                                                                        \
  }                                                                            \
} while (0)



/** @brief
*   initialize a pool array
*
*   @details
*   allocates space for the pool array placing each element
*   into the available list
*
*   @param [in] pa pointer to a uninitialized PoolArray struct
*   @param [in] array pointer to start of array to track
*   @param [in] size of each struct in the array
*   @param [in] total the total number of items in the array
*
*   @return false if memory can't be allocated
*/
// initialise the PoolArray
// pa       -   by referance we're modifying it...
// array    -   address of array
// size     -   size of one array item
// total    -   number of items in array
bool PoolArrayInit(PoolArray* pa, void* array, size_t size, size_t total)
{
    // because array could be of any type,
    // we need item size, and the total number of items
    pa->Size = total;
    pa->activeList = NULL;
    pa->availableList = NULL;

    pa->nodes = malloc(pa->Size * sizeof(PoolArrayNode));

    if (pa->nodes == NULL)
    {
        return false;    // end user MUST check for this.
    }

    for (size_t i = 0; i < pa->Size; i++)
    {
        pa->nodes[i].next = NULL;
        pa->nodes[i].prev = NULL;
        // oh the horror, pointer math ...!
        pa->nodes[i].userData = (void*)(((size_t)array) + (size * i));
        DL_APPEND(pa->availableList, (&pa->nodes[i]));
    }
    pa->Available = pa->Size;
    return true;
}

/** @brief
*   frees a pool array
*
*   @details
*   frees memory used to track the array, the array that is tracked
*   will need to be freed if needed
*
*   @param [in] pa pointer to a PoolArray struct
*/
void PoolArrayDestroy(PoolArray* pa) {
    free(pa->nodes);
}

/** @brief
*   retire a item in the pool
*
*   @details
*   moves an item from the active list to the available list
*
*   @param [in] poolArray pointer to PoolArray struct
*   @param [in] node the item
*
*/
// move a pool item from the active to available list
void PoolArrayRetireNode(PoolArray* poolArray, PoolArrayNode* node)
{
    DL_DELETE(poolArray->activeList, node);
    node->next = NULL;
    node->prev = NULL;
    DL_APPEND(poolArray->availableList, node);
    poolArray->Available ++;
}

/** @brief
*   retire all items in the pool
*
*   @details
*   moves all items from the active list to the available list
*
*   @param [in] poolArray pointer to PoolArray struct
*
*/
// retires all items to the available list emptying the active list
void PoolArrayRetireAll(PoolArray* poolArray)
{
    PoolArrayNode* n;
    n = poolArray->activeList;
    while(n) {
        PoolArrayRetireNode(poolArray, n);
        n = poolArray->activeList;
    }
}

/** @brief
*   get an available item from the pool
*
*   @details
*   finds an available item in the pool placing it on the active list
*
*   @param [in] poolArray pointer to PoolArray struct
*
*   @return pointer to item or NULL in nothing available
*/
// gets an item off the available list putting it in the active list
// you MUST check for NULL or at least check Available yourself
void* PoolArrayGetNextItem(PoolArray* poolArray)
{
    // abort if none available
    if (poolArray->Available < 1) {
        return NULL;    // end user MUST check for this
    }

    // enlist an item...
    // take first item from top of availableList
    PoolArrayNode* node = (PoolArrayNode*)poolArray->availableList;
    DL_DELETE(poolArray->availableList, node);
    node->next = NULL;
    node->prev = NULL;
    DL_APPEND(poolArray->activeList, node);
    poolArray->Available --;

    return node->userData;
}
#endif

#endif // POOLARRAY_H
