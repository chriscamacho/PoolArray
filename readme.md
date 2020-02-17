<h4>quick start</h4>
 
<p><b>#define POOLARRAY_IMPL</b></p>
One source file in your project (only one) must define this
to include the actual code for the functions.

<p>assuming you have an array of struct
@code
typedef struct testItem {
    int ttl;    // time to live
    int id;
} testItem;
@endcode
first initialise the pool
@code
PoolArray pool;             // the pool to manage the array
testItem items[poolSize];   // the array we're managing
 *
// you MUST check the return value
if (!PoolArrayInit(&pool, &items, sizeof(testItem), poolSize)) {
    printf("unable to initialise pool array\n");
    return -1;
}
@endcode
When you need to use an item in the pool
@code
testItem* it = PoolArrayGetNextItem(&pool);
if (it == NULL) {
    printf("!!!! Pool used up couldn't grab a new item\n");
} else {
    // set up the new item...
}
@endcode
when you need to update your active item in the pool
you can use a simple for loop
@code
for(PoolArrayNode* node = pool.activeList; node; node = node->next) {
    testItem* item = (testItem*)node->userData;
    ...
}
@endcode
however if you need to retire a node within a loop you must first
find out the next node...
@code
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
@endcode
don't forget to destroy the pool when you have finished with it...
