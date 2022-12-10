
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "b-tree.h"

btree init_BTree(void)
/* firstly let's create tree */
{
    btree b = (btree) malloc(sizeof(*b));
    // if it is created firstly - it is a leaf, empty leaf
    b->is_leaf = 1;
    b->size = 0;
    // init children and data as NULL
    for(int i =0; i< MAX_SIZE ; i++)
    {
        b->child[i] = NULL;
        b->data[i].key = NULL;
        b->data[i].value = NULL;
        b->data[i].exist = 0;
    }
    b->child[MAX_SIZE] = NULL;
    return b;
}


void free_BTree(btree b)
/* if i made it, i can kill it
 * of courese destroy not only one nide, but all of them recursively
 * */
{
    if (!(b->is_leaf))
        /* if there are more nodes, let's make them free */
    {
        for (int i = 0; i < MAX_SIZE+1; i++)
        {
            if(b->child[i] != NULL)
            {
                free_BTree(b->child[i]);
            }
        }
    }
    /*let's free dad */
    free(b);
}


int array_search__key(int size, const struct Data *array, uint64_t key)
// in sorted array we can find key with binary search
{
    int a  = -1;
    int b = 0;
    int c = size;
    // classics
    while (a + 1 < c) {
        b = (a + c) / 2;
        if (array[b].key == key)
            return b;
        else if (array[b].key < key)
            a = b;
        else
            c = b;
    }

    return c;
}


static btree btree_insert(btree b, struct Data elem, struct Data *median)
// whoo, inserting
{
    // calulating position to insert
    int position = array_search__key(b->size, b->data, elem.key);


    //printf("key = %lu\n pos=%lu\n", elem.key, position);
    //for(int j = 0; j < b->size; j++)
    //{
        //printf("%d, ",b->data[j]);
    //}
    //printf("b->size, %d\n\n", b->size);

    if (position < b->size && b->data[position].key == elem.key)
    {
        b->data[position] = elem;
        return NULL;
    }

    // for better understanding follow points
    if (b->is_leaf)
        // 1) no way, we got a leaf, let's just add to data, about overflow later
    {
        memmove(&b->data[position + 1], &b->data[position], sizeof(struct Data) * (b->size - position));
        b->data[position] = elem;
        b->size++;
    }
    else
        // 6) wait, if we are not in leaf
    {
        struct Data child_median;
        // no way, insert to child
        // b->child[position] due to algorithm
        if (b->child[position] == NULL)
            // here we got (maybe) NULL child, so let,s create a child
        {
            b->child[position] = init_BTree();
        }
        btree B_tree_child = btree_insert(b->child[position], elem, &child_median);

        if (B_tree_child)
        // if it goes down and we have a separated tree by point (3)
        {
            // we just insert median into our node as median
            memmove(&b->data[position + 1], &b->data[position], sizeof(struct Data) * (b->size - position));
            b->data[position] = child_median;
            // and creating child for this median
            memmove(&b->child[position + 2], &b->child[position + 1], sizeof(struct Node) * (b->size - position));
            b->child[position + 1] = B_tree_child;
            // ok finally dont forget to take into account that there is a new child
            b->size++;
            // now our tree is ready to be checked for over size, go to point (3)
        }
    }

    if (b->size > MAX_SIZE)
    // 3) if we are oversize
    {
        // creating new node
        btree b_new = init_BTree();
        // median value is special one, it is to be returned for pushing up
        int mid = b->size / 2;
        *median = b->data[mid];
        // new node
        b_new->size = b->size - mid - 1;
        // shorting node, ignored data is to be passed to new node
        b->size = mid;
        b_new->is_leaf = b->is_leaf;
        // data from old node to new one
        memmove(b_new->data, &b->data[mid + 1], sizeof(struct Data) * b_new->size);
        // ok, if we want not to observe this data let's set it to null
        for(int i = b->size; i < mid + 1 + b_new->size; i++)
        {
            b->data[i].key = NULL;
        }

        if (!(b->is_leaf))
            // if not leaf we shold copy not only data, but children too
        {
            memmove(b_new->child, &b->child[mid + 1], sizeof(btree) * (b_new->size + 1));
            // and again let's set NULL to all children we dont' want to see
            for(int i = mid + 1; i < mid + 1 + b_new->size + 1; i++)
            {
                b->child[i] = NULL;
            }
        }

        // passing new node. next point is in other function
        return b_new;
    } else
        // 2) if we are not over sized let's go do nothing and this is the end
    {
        return NULL;
    }
}

void B_tree_insert(btree b, struct Data elem)
/* if we got a tree, let's put smth in! */
{
    // data-struct for data we want to insert
    struct Data median;
    // wow, we need to insert in only one direction(not like binary tree), so may be there is no direction
    btree b_new = btree_insert(b, elem, &median);
    // so we got a
    if (b_new)
        // 5) we got a new tree with data to be reconstructed. we need to unite them
    {
        // we copy data to new tree
        btree b_final = (btree) malloc(sizeof(*b_new));
        memmove(b_final, b, sizeof(*b));
        // and median value to the top
        b->size = 1;
        b->data[0] = median;
        b->is_leaf = 0;
        // and now final has 2 children
        b->child[0] = b_final;
        b->child[1] = b_new;
        // set useless data to NULL

        b->data[1].key = NULL;
        b->child[MAX_SIZE] = NULL;
        for(int i = 2; i < MAX_SIZE; i++)
        {
            b->data[i].key = NULL;
            b->child[i] = NULL;

        }
    }
}


uint64_t B_tree_search(btree b, uint64_t key)
// seems to be easy.
{

    if (b->size == 0)
        //empty tree doestn seems to be good
    {
        printf("no data in tree\n");
        return -1;
    }

    // search for position
    int position = array_search__key(b->size, b->data, key);
    //if key is founded
    if (position < b->size && b->data[position].key == key)
    {
        if (b->data[position].exist != 0)
        {
            return b->data[position].value;
        }else
        {
            printf("it seems that data is deleted\n");
            return -1;
        }

    } else if (b->is_leaf)
        // we need to go down, but can we?
    {
        return -1;
    }
    // if we can let's go down
    return B_tree_search(b->child[position], key);
}


void B_tree_delete(btree b, uint64_t key)
//if we only have to insert del marker, ok
{
    struct Data elem;
    elem.key = key; elem.value = -1;
    B_tree_insert(b, elem);
}

void get_number_of_elems(btree b, uint64_t *sum)
// here we try to find total length of tree for allocating memory before merge
{
    int size = 0;
    for(int i = 0; i<MAX_SIZE; i++)
    {
        if (b->data[i].key != NULL)
        {
            size ++;
        }
    }
    *sum += size;
    for(uint64_t i = 0; i< MAX_SIZE + 1; i++)
    {
        if (b->is_leaf != 1 && b->child[i] != NULL)
        {
            get_number_of_elems(b->child[i], sum);
        }
    }
}

static void tree_to_array(btree b, int *curr_pos, struct Data *array)
{
    for(int i = 0; i < MAX_SIZE; i++)
    {
        if (b->data[i].key != NULL)
        {
            array[*curr_pos] = b->data[i];
            *curr_pos = *curr_pos + 1;
        }
        if(b->is_leaf != 1 && b->child[i] != NULL)
        {
            tree_to_array(b->child[i], curr_pos, array);
        }
    }
    if(b->is_leaf != 1 && b->child[MAX_SIZE] != NULL)
    {
        tree_to_array(b->child[MAX_SIZE], curr_pos, array);
    }
}


btree merge(btree b1, btree b2)
// return a btree as result of merging
{
    // let's got length of each trees
    uint64_t l1 = 0;
    uint64_t l2 = 0;
    get_number_of_elems(b1, &l1);
    get_number_of_elems(b2, &l2);
    // then let's got all the elems from each tree to huge array
    int currpos = 0;

    struct Data data[l1+l2];
    // let's collect data from all the tree
    tree_to_array(b1, &currpos, data);
    tree_to_array(b2, &currpos, data);
    // and finaly push all of them to brand new tree
    btree out = init_BTree();
    for(uint64_t i = 0; i < l1+l2; i++)
    {
        B_tree_insert(out, data[i]);
    }
    return out;
