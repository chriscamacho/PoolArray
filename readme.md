<h4>quick start</h4>
 
<p><b>#define POOLARRAY_IMPL</b></p>
One source file in your project (only one) must define this
to include the actual code for the functions.

<p>assuming you have an array of struct
"""
typedef struct testItem {
    int ttl;    // time to live
    int id;
} testItem;
"""
first initialise the pool
"""
PoolArray pool;             // the pool to manage the array
testItem items[poolSize];   // the array we're managing

// you MUST check the return value
if (!PoolArrayInit(&pool, &items, sizeof(testItem), poolSize)) {
    printf("unable to initialise pool array\n");
    return -1;
}
"""
When you need to use an item in the pool
"""
testItem* it = PoolArrayGetNextItem(&pool);
if (it == NULL) {
    printf("!!!! Pool used up couldn't grab a new item\n");
} else {
    // set up the new item...
}
"""
when you need to update your active item in the pool
you can use a simple for loop
"""
for(PoolArrayNode* node = pool.activeList; node; node = node->next) {
    testItem* item = (testItem*)node->userData;
    ...
}
"""
however if you need to retire a node within a loop you must first
find out the next node...
"""
PoolArrayNode* node = pool.activeList;
while( node ) {
    // update the item
    testItem* item = (testItem*)node->userData;
    PoolArrayNode* nextNode = node->next;
    ...

    if (checkItem(item)) {
        PoolArrayRetireNode(&pool, node);
    }
    node = nextNode;
}
"""
don't forget to destroy the pool when you have finished with it...
