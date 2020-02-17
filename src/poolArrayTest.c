/*
 * PoolArray
 *
 * tracks an array of structs, allowing rapid access to an unused item
 * this example demonstrates usage of the PoolArray...
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

#include <stdio.h>
#include <stdbool.h>
#include <time.h>

// one and one only source file in your project should
// define POOLARRAY_IMPL
#define POOLARRAY_IMPL
#include "PoolArray.h"

#define rnd(min, max) rand()%(abs(max - min) + 1) + min

// this might be a struct for an alien or missile for example...
typedef struct testItem {
    int ttl;    // time to live
    int id;
} testItem;

// pool size is deliberatly small in order to demonstrate
// what happens when the array is full
#define poolSize 4

// number of interrations for the demo
#define runs 32


// paranoia! manually count the array items to verify pool->Avialable works !
int manualListCount(PoolArrayNode* l)
{
    int a = 0;
    for(PoolArrayNode* node = l; node; node = node->next) {
        a++;
    }
    return a;
}

int main()
{
    srand(time(NULL));

    PoolArray pool;             // the pool to manage the array
    testItem items[poolSize];   // the array we're managing

    // give our test items some "data"
    for (int i=0; i < poolSize; i++) {
        items[i].id = i;
    }

    // you MUST check the return value
    if (!PoolArrayInit(&pool, &items, sizeof(testItem), poolSize)) {
        printf("unable to initialise pool array\n");
        return -1;
    }

    for (int i=1; i < runs+1; i++) {
        printf("     iteration %i   -   ", i);
        int a = manualListCount(pool.activeList);
        printf("available %li, manual active count %i, balance %li\n",
                pool.Available, a, pool.Size - (a + pool.Available));


        if (i < runs-5) { // just to show the pool emptying at the end
            // try to make an active item each iterration
            // you MUST check the return value!
            testItem* it = PoolArrayGetNextItem(&pool);
            if (it == NULL) {
                printf("!!!! Pool used up couldn't grab a new item\n");
            } else {
                // set up the new item...
                it->ttl = rnd(2,6);  // random time to live
                printf(">>>> Created a new item (id %i) ttl=%i\n", it->id, it->ttl);
            }

        }

        // update the active pool items and retire expired ones

        // you should NOT directly manipulate the lists but you
        // can iterrate the active list, for active items updating
        // and rendering for example
        PoolArrayNode* node = pool.activeList;
        while( node ) {
            // update the item
            testItem* item = (testItem*)node->userData;
            item->ttl --;

            // if we're retiring nodes (while iterating)
            // we must get the next node first....
            PoolArrayNode* nextNode = node->next;
            // retire an expired item
            if (item->ttl < 1) {
                printf("<<<< retiring item id=%i (ttl %i)\n", item->id, item->ttl);
                PoolArrayRetireNode(&pool, node);
            }
            node = nextNode;
        }

        printf("     id's in active list  ");
        for(PoolArrayNode* node = pool.activeList; node; node = node->next) {
            testItem* item = (testItem*)node->userData;
            printf("%i(ttl %i)   ", item->id, item->ttl);
        }
        printf("\n-------------------------------------------------\n");

        if (i == (int)runs/2) {
            // somethings happened! we need to clear down the whole array...
            // for example the player might have lost a life and you want to
            // remove all the active missiles from the game
            printf("\n**** clearing whole pool of active items\n\n");
            PoolArrayRetireAll(&pool);
        }

    }

    // release stuff
    PoolArrayDestroy(&pool);
    // if you malloc'd the array you can now free it....

    return 0;
}
