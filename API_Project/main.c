#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#define len_instr 200
#define len_text 1030

typedef struct{
    char **text_arr;

    size_t size;
    size_t used;
}Text;

// Generic Tree Node
struct Node{
    int row;                                       // start from 1
    int index;
    int n_instr;
    Text *text;
    char color;                                 // red or black
    struct Node *left, *right, *parent;         // pointers to other nodes
};

// Block that indicate a placeholder
struct Block_index{
    int start;
    int finish;

    struct Block_index *next, *prev;
};

typedef struct{
    unsigned short *num_text;

    int old_addr;
    int used;
    int size;
} Text_array;

typedef struct{
    struct Block_index **Index_list_first;

    Text_array **t_arr;

    int *num_rows;
    int *last_indexes;
    int *num_ope;

    size_t used;
    size_t size;
}Index_page;

// ROTATIONS

// right rotate the node to fix the tree balance
void Right_rotate(struct Node **root, struct Node *x){
    struct Node *y = x->parent;

    // fix Beta
    y->left = x->right;
    if (x->right != NULL)
        x->right->parent = y;

    // put x as right or left child of y parent
    x->parent = y->parent;
    if (x->parent == NULL)
        (*root) = x;
    else if (y == y->parent->left)
        y->parent->left = x;
    else y->parent->right = x;

    // fix y and x
    x->right = y;
    y->parent = x;
}

// left rotate the node to fix the tree balance
void Left_rotate(struct Node **root, struct Node *x){
    struct Node *y = x->right;

    // fix Beta
    x->right = y->left;
    if(y->left != NULL)
        y->left->parent = x;

    // put y as right or left child of x parent
    y->parent = x->parent;
    if (y->parent == NULL)
        (*root) = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else x->parent->right = y;

    // fix y and x
    y->left = x;
    x->parent = y;
}

// FIXING

// Fix the tree after an insert
void Fix_insert(struct Node **root, struct Node *fix_node){

    //check if fix is necessary
    while (fix_node != (*root) && fix_node->parent->color == 'r'){

        struct Node *uncle = NULL;
        struct Node *parent = fix_node->parent;
        struct Node *grand_parent = fix_node->parent->parent;

        if (parent == grand_parent->left)
            uncle = grand_parent->right;
        else
            uncle = grand_parent->left;

        // case 1 : the uncle is red
        if (uncle != NULL && uncle->color == 'r')
        {
            parent->color = 'b';
            uncle->color = 'b';
            grand_parent->color = 'r';
            fix_node = grand_parent;
        }
        // case 2,3 : uncle black
        else{
            // case 2.1 : fix_node is the left child and parent is the left child (LL)
            if (fix_node == parent->left && parent == grand_parent->left){
                parent->color = 'b';
                grand_parent->color = 'r';
                Right_rotate(root, parent);

            }
            else{
                // case 2.2 : fix_node is the right child and parent is the right child (RR)
                if (fix_node == parent->right && parent == grand_parent->right){
                    parent->color = 'b';
                    grand_parent->color = 'r';
                    Left_rotate(root,grand_parent);
                }
                // case 3 : fix_node and parent are opposite children (LR) (RL)
                else{
                    // case 3.1 : fix_node right child and parent left child
                    if (fix_node == parent->right && parent == grand_parent->left){

                        Left_rotate(root,parent);

                        //fix_node->color = 'b';
                        fix_node = fix_node->left;
                        fix_node->parent->color = 'b';

                        grand_parent->color = 'r';

                        //Right_rotate(root, fix_node);
                        Right_rotate(root, fix_node->parent);

                    }
                    else{
                        //case 3.2 : fix_node left child and parent right child
                        if(fix_node == parent->left && parent == grand_parent->right){
                            Right_rotate(root, fix_node);

                            //fix_node->color = 'b';
                            fix_node = fix_node->right;
                            fix_node->parent->color = 'b';

                            grand_parent->color = 'r';

                            Left_rotate(root,grand_parent);
                        }
                    }
                }
            }
        }
    }
    (*root)->color = 'b';
}

// Fix the index list
void Fix_list_index(struct Block_index **first_el_index, struct Block_index *curr, int ind1, int ind2, int n_rows, int ind1_1){

    if(ind2 > n_rows)
        ind2 = n_rows;

    if(curr == NULL)
        curr = *first_el_index;

    // empty list add a new block place holder
    if (*first_el_index == NULL) {
        struct Block_index *new_block = malloc(sizeof(struct Block_index));

        new_block->start = ind1;
        new_block->finish = ind2;
        new_block->next = NULL;
        new_block->prev = NULL;

        *first_el_index = new_block;
    }
    // not empty list
    else{
        int space = ind2 - ind1 + 1;
        int curr_space;

        // can reduce the start of the current block
        if (ind1_1 < curr->start) {
            if (curr->start-ind1_1 <= space) {
                space = space - curr->start + ind1_1;
                curr->start = ind1_1;
                ind1_1 = curr->finish + 1;

                // check if 2 blocks are adjacent
                if (curr->prev != NULL){
                    if (curr->prev->finish == curr->start - 1){
                        curr->start = curr->prev->start;
                        curr->prev = curr->prev->prev;
                        if (curr->prev != NULL)
                            curr->prev->next = curr;
                        else
                            *first_el_index = curr;
                    }
                }

            } else
            {
                struct Block_index *new_block = malloc(sizeof(struct Block_index));

                new_block->start = ind1_1;
                new_block->finish = ind1_1 + (ind2 - ind1);
                new_block->next = curr;
                new_block->prev = curr->prev;
                if(curr->prev != NULL)
                    curr->prev->next = new_block;
                curr->prev = new_block;
                space = 0;
                if (*first_el_index == curr)
                    *first_el_index = new_block;
            }
        }

        // can NOT increase the finish of the current block
        if (ind1_1 != curr->finish +1) {

            if (space > 0) {
                // ind1_1 and ind2_2 do NOT touch adiacent blocks
                if (curr->next == NULL || curr->next->start > (ind1_1 + space)) {
                    struct Block_index *new_block = malloc(sizeof(struct Block_index));

                    new_block->start = ind1_1;
                    new_block->finish = ind1_1 + (ind2 - ind1);
                    new_block->next = curr->next;
                    new_block->prev = curr;
                    if (curr->next != NULL)
                        curr->next->prev = new_block;
                    curr->next = new_block;

                    space = 0;

                }
                // can decrease the next block start
                else{
                    space = space - curr->next->start + ind1_1;

                    curr->next->start = ind1_1;
                    curr = curr->next;


                    // check if 2 blocks are adjacent
                    if (curr->prev->finish == curr->start - 1) {
                        struct Block_index *to_free = curr->prev;
                        curr->start = curr->prev->start;
                        curr->prev = curr->prev->prev;
                        if (curr->prev != NULL)
                            curr->prev->next = curr;
                        else
                            *first_el_index = curr;
                        free(to_free);
                    }


                }
            }
        }
        int check_if_next = 1;
        // fill all the possible gaps
        while (space > 0) {
            check_if_next = 1;
            if (curr->next != NULL)
                curr_space = curr->next->start - curr->finish - 1;
            else curr_space = -1;
            if (curr_space >= space || curr_space == -1) {

                curr->finish = curr->finish + space;
                space = 0;

                // check if 2 blocks are adjacent
                if(curr->next != NULL) {
                    if (curr->finish == curr->next->start - 1) {
                        struct Block_index *to_free = curr->next;
                        curr->finish = curr->next->finish;
                        curr->next = curr->next->next;
                        if (curr->next != NULL)
                            curr->next->prev = curr;
                        free(to_free);
                    }
                    check_if_next = 0;
                }


            } else {

                curr->finish = curr->finish + curr_space;
                space = space - curr_space;

                // check if 2 blocks are adjacent
                if(curr->next != NULL) {
                    if (curr->finish == curr->next->start - 1) {
                        struct Block_index *to_free = curr->next;
                        curr->finish = curr->next->finish;
                        curr->next = curr->next->next;
                        if (curr->next != NULL)
                            curr->next->prev = curr;
                        free(to_free);
                    }
                    check_if_next = 0;
                }
            }
            if (check_if_next == 1)
                curr = curr->next;
        }

    }
}

int Fix_index_u_r(Index_page indexPage, struct Block_index **first_el_index, int curr_ind_page, int n_undo, int *n_rows, int *last_index, Text_array **textArray){
    if(n_undo != 0) {
        int address = curr_ind_page - n_undo - 1;
        *first_el_index = indexPage.Index_list_first[address];
        //*last_el_index = indexPage.Index_list_last[address];
        *n_rows = indexPage.num_rows[address];
        *last_index = indexPage.last_indexes[address];

        Text_array *to_elaborate = NULL;

        if (indexPage.t_arr[address]->old_addr == -1) {
            to_elaborate = indexPage.t_arr[address];
        }
        else {
            to_elaborate = indexPage.t_arr[indexPage.t_arr[address]->old_addr];
        }

        *textArray = malloc(sizeof(Text_array));
        (*textArray)->size = to_elaborate->size;
        (*textArray)->used = to_elaborate->used;
        (*textArray)->old_addr = to_elaborate->old_addr;
        (*textArray)->num_text = malloc(to_elaborate->size * sizeof(unsigned short ));

        (*textArray)->num_text[0] = 0;
        int size = 1;
        int index = 1;
        int curr = 1;
        unsigned short type = 0;

        while(size != to_elaborate->used){
            size += to_elaborate->num_text[index];
            type = to_elaborate->num_text[index + 1];
            for (int i = curr; i < size; ++i) {
                (*textArray)->num_text[i] = type;
            }
            index += 2;
            curr = size;
        }
        memset(&(*textArray)->num_text[curr],0,(to_elaborate->size - curr) * sizeof(unsigned short ));
    }
    return curr_ind_page - n_undo;
}

Text_array *Copy_list_index(struct Block_index **first_el_index, int curr_ind_page, Index_page *indexPage, Text_array *textArray, int old_rows){

    if (indexPage->used >= indexPage->size){
        indexPage->size += 1000;
        indexPage->Index_list_first = realloc(indexPage->Index_list_first, indexPage->size * sizeof(struct Block_index *));
        indexPage->t_arr = realloc(indexPage->t_arr, indexPage->size * sizeof(Text_array *));
        indexPage->num_rows = realloc(indexPage->num_rows, indexPage->size * sizeof(int ));
        indexPage->last_indexes = realloc(indexPage->last_indexes, indexPage->size * sizeof(int ));
        indexPage->num_ope = realloc(indexPage->num_ope, indexPage->size * sizeof(int ));
    }

    if (curr_ind_page < indexPage->used){
        if (indexPage->t_arr[curr_ind_page]->num_text != NULL)
            free(indexPage->t_arr[curr_ind_page]->num_text);
        free(indexPage->t_arr[curr_ind_page]);

        struct Block_index *first_index = indexPage->Index_list_first[curr_ind_page];
        while(first_index != NULL){
            if(first_index->next != NULL) {
                first_index = first_index->next;
                free(first_index->prev);
            } else{
                free(first_index);
                first_index = NULL;
            }
        }
    }

    indexPage->Index_list_first[curr_ind_page] = *first_el_index;

    struct Block_index *curr = *first_el_index;
    struct Block_index *prev = NULL;
    while (curr != NULL){
        struct Block_index *new_block = malloc(sizeof(struct Block_index));
        new_block->start = curr->start;
        new_block->finish = curr->finish;
        if(curr->prev == NULL){
            new_block->prev = NULL;
            *first_el_index = new_block;
        }else{
            new_block->prev = prev;
            prev->next = new_block;
        }
        if(curr->next == NULL){
            new_block->next = NULL;
        }

        prev = new_block;
        curr = curr->next;
    }

    if (old_rows <= 0) {
        Text_array *new_array = malloc(sizeof( Text_array));
        new_array->num_text = NULL;
        if (indexPage->t_arr[curr_ind_page - 1] != NULL && indexPage->t_arr[curr_ind_page - 1]->old_addr != -1 )
            new_array->old_addr = indexPage->t_arr[curr_ind_page - 1]->old_addr;
        else
            new_array->old_addr = curr_ind_page - 1;
        indexPage->t_arr[curr_ind_page] = new_array;
    }
    else{
        int size = 1;

        Text_array *new_array = malloc(sizeof( Text_array));
        new_array->size = 11;
        new_array->used = 1;
        new_array->old_addr = textArray->old_addr;
        new_array->num_text = malloc(new_array->size * sizeof new_array->num_text);

        int counter = 0;
        unsigned short type = 1;
        new_array->num_text[0] = 0;
        while(size < textArray->size && type != 0) {

            counter = 0;
            type = textArray->num_text[size];

            if (type != 0) {
                while (textArray->num_text[size] == type) {
                    counter++;
                    size++;
                }

                new_array->num_text[new_array->used] = counter;
                new_array->num_text[new_array->used + 1] = type;

                new_array->used += 2;

                if (new_array->size == new_array->used) {
                    new_array->size = (new_array->size * 2) - 1;
                    new_array->num_text = realloc(new_array->num_text, new_array->size * sizeof new_array->num_text);
                }
            }
        }

        new_array->size = textArray->size;
        new_array->used = textArray->used;

        indexPage->t_arr[curr_ind_page] = new_array;
    }
    return textArray;
}

// find successor
struct Node *Successor(struct Node *node) {
    struct Node *succ_node = NULL;

    // find the successor by going to the parent, right child does NOT exist
    if (node->right == NULL) {
        succ_node = node->parent;
        while (succ_node != NULL && node == succ_node->right){
            node = succ_node;
            succ_node = succ_node->parent;
        }
        return succ_node;
    }
    else{
        succ_node = node->right;                // go to the right child
        while (succ_node->left != NULL)         //find the most left node of the right tree
            succ_node = succ_node->left;
        return succ_node;
    }
}

// INSERTION AND DELETION

// Insert a new tree node
struct Node *Insert(struct Node **root, struct Node *new_node){

    // the tree is empty
    if ((*root) == NULL){
        new_node->color = 'b';
        (*root) = new_node;
        return new_node;
    }
    // the tree is not empty so find the right spot for the new node
    else{
        new_node->color = 'r';                     //color of the new node is Red

        struct Node *parent = NULL;                //it will points to the parent of the new node
        struct Node *probe = (*root);              //it's used to inspect the tree top->bottom

        // binary search
        while (probe != NULL){
            parent = probe;                        //update the parent
            if(new_node->row < probe->row)
                probe = probe->left;
            else
                probe = probe->right;
        }

        // insert the new node in the right spot
        new_node->parent = parent;
        if(new_node->row < parent->row)
            parent->left = new_node;
        else
            parent->right = new_node;

        // Fix the tree
        Fix_insert(root, new_node);
        return new_node;
    }
}

// Insert the successor of a node just inserted
struct Node *Insert_successor(struct Node **root, struct Node *node, struct Node *new_node){

    // node does NOT have the right child so I can place the new node there
    if(node->right == NULL){
        new_node->parent = node;
        node->right = new_node;
    }
    // I need to find the successor and attach the new node there
    else{
        node = Successor(node);
        new_node->parent = node;
        node->left = new_node;
    }

    Fix_insert(root, new_node);
    return new_node;
}

// change text
void Change_text(struct Node *node, char *new_text, Text_array textArray){

    unsigned long text_len = strlen(new_text);
    unsigned long old_text_len;

    int index = textArray.num_text[node->index];

    if (index < node->text->used)
        old_text_len = strlen(node->text->text_arr[index]);
    else
        old_text_len = 0;

    if (old_text_len == 0){
        node->text->text_arr[index] = malloc(text_len + 1);
    }
    else {
        if (old_text_len < text_len) {
            node->text->text_arr[index] = realloc(node->text->text_arr[index], text_len + 1);
        }
    }
    strcpy(node->text->text_arr[index], new_text);
}

// find node by row
struct Node *Find_node(struct Node **root, int f_row) {
    if ((*root)== NULL)
        return NULL;
    struct Node *f_node = *root;
    if (f_node->row == f_row)
        return f_node;
    else {
        while (f_node != NULL && f_node->row != f_row) {
            if (f_node->row < f_row)
                f_node = f_node->right;
            else
                f_node = f_node->left;
        }
        return f_node;
    }
}

// find the first node of that have a row between the two index
struct Node *Find_first(struct Node *root,int start,int end){
    if (root == NULL)
        return root;
    else {
        while (root->row < start || root->row > end) {
            if (root->row < start)
                root = root->right;
            else
                root = root->left;
        }
        return root;
    }
}

// FUNCTIONS FOR OPERATION

// finding the right first index
struct Block_index *Find_index(struct Block_index *first_el, int *ind1_1, int ind1, int n_rows, int last_index){

    if(first_el == NULL){
        return first_el;
    }
    else {
        struct Block_index *curr = first_el;

        int step = ind1 - curr->start + 1;

        while (step > 0) {
            if (curr->next != NULL && curr->next->start - curr->finish - 1 < step)
                step = step - curr->next->start + curr->finish + 1;
            else {
                *ind1_1 = curr->finish + step;
                step = 0;
            }
            if (step > 0)
                curr = curr->next;
        }
        return curr;
    }
}

// finding the right second index
int Find_second_index(struct Block_index *curr, int ind1_1, int step){
    int ind2_2 = ind1_1;
    if (curr == NULL || (curr->next == NULL && ind1_1 > curr->start))
        return ind2_2 + step;
    else{
        if (ind1_1 > curr->start) {
            if(curr->next->start - ind1_1 - 1 < step) {
                step = step - (curr->next->start - ind1_1 - 1);
                curr = curr->next;
            } else{
                return ind2_2 + step;
            }
        } else{
            if (curr->start - ind1_1 - 1 < step)
                step = step - (curr->start - ind1_1 - 1);
            else
                return ind2_2 + step;
        }
        while(step > 0){
            if (curr->next != NULL && curr->next->start - curr->finish - 1 < step)
                step = step - (curr->next->start - curr->finish - 1);
            else {
                ind2_2 = curr->finish + step;
                step = 0;
            }
            if (step > 0)
                curr = curr->next;
        }
        return ind2_2;
    }
}

// all the operations that's need to be done when a change operation occur
int C_operation(struct Node **root, char *text_row, int n_rows, int ind1, int ind2, int old_rows,int new_rows, int ind1_1,
        int *last_index, struct Block_index *curr, int last_root_index, Text_array *t_arr, int num_ope, int first_op, int last_op){

    char dot[3];

    struct Node *new_node;

    if (old_rows > 0) {

        int ind2_2 = Find_second_index(curr,ind1_1,old_rows);
        struct Node *root_change = Find_first(*root,ind1_1,ind2_2);
        // update the text of the existing rows
        struct Node *change_node = Find_node(&root_change, ind1_1);


        //int last_used = t_arr->used;

        for (; old_rows > 0; old_rows--) {

            if (t_arr->used == t_arr->size){
                int old_size = t_arr->size;
                t_arr->size += 50;
                t_arr->num_text = realloc(t_arr->num_text, t_arr->size * sizeof t_arr->num_text);
                memset(&(t_arr->num_text[old_size]),0,(t_arr->size - old_size)*sizeof t_arr->num_text);
            }

            if (change_node->n_instr == -1) {
                change_node->index = t_arr->used;
                change_node->n_instr = num_ope;
                t_arr->used++;
            }

            t_arr->num_text[change_node->index]++ ;

            if (change_node->text->used == change_node->text->size){
                change_node->text->size *= 2;
                change_node->text->text_arr = realloc(change_node->text->text_arr, change_node->text->size * sizeof(char *));
            }

            if(fgets(text_row, len_text, stdin) == NULL){printf("error");}    // get the new text
            Change_text(change_node, text_row, *t_arr);    // update the text

            if (t_arr->num_text[change_node->index] == change_node->text->used)
                change_node->text->used++;

            if (old_rows > 1) {  // find the next node
                if (curr == NULL) {
                    change_node = Successor(change_node);
                } else {
                    if (ind1_1 < curr->start) {
                        if (ind1_1 != curr->start - 1) {
                            change_node = Successor(change_node);
                            ind1_1++;
                        } else {
                            ind1_1 = curr->finish + 1;
                            change_node = Find_node(&root_change, ind1_1);
                            curr = curr->next;
                        }
                    } else {
                        if (curr->next == NULL) {
                            change_node = Successor(change_node);
                            curr = curr->next;
                        } else {
                            curr = curr->next;
                            if (ind1_1 != curr->start - 1) {
                                change_node = Successor(change_node);
                                ind1_1++;
                            } else {
                                ind1_1 = curr->finish + 1;
                                change_node = Find_node(&root_change, ind1_1);
                                curr = curr->next;
                            }
                        }
                    }
                }
            }
        }
    }
    // add new rows
    int n_node_avaiable = last_root_index - *last_index;
    if (n_node_avaiable > 0) {
        struct Node *ava_node;
        ava_node = Find_node(root, *last_index + 1);
        while (new_rows > 0 && n_node_avaiable > 0) {
            if (fgets(text_row, len_text, stdin) == NULL) { printf("error"); }

            n_rows++;
            *last_index = *last_index + 1;

            for (int i = 0; i < ava_node->text->used; ++i) {
                free(ava_node->text->text_arr[i]);
            }
            ava_node->text->used = 0;
            ava_node->index = 0;
            ava_node->n_instr = -1;

            Change_text(ava_node, text_row, *t_arr);

            if (t_arr->num_text[ava_node->index] == ava_node->text->used)
                ava_node->text->used++;

            new_rows--;
            n_node_avaiable--;

            if (n_node_avaiable > 0)
                ava_node = Successor(ava_node);
        }
    }

    if(new_rows > 0) {
        if(fgets(text_row, len_text, stdin) == NULL){printf("error");}       // get the text
        n_rows++;                                 // update n of rows
        *last_index = *last_index + 1;

        //Create the new node
        struct Node *insert_node = malloc(sizeof *insert_node);
        insert_node->row = *last_index;
        insert_node->index = 0;
        insert_node->n_instr = -1;
        Text *text = malloc(sizeof(Text));
        text->size = 10;
        text->used = 1;
        text->text_arr = malloc(10 * sizeof text->text_arr);
        insert_node->text = text;
        text->text_arr[0] = malloc(strlen(text_row) + 2);
        strcpy(insert_node->text->text_arr[0], text_row);
        insert_node->left = insert_node->right = insert_node->parent = NULL;
        new_node = Insert(root, insert_node);
        new_rows--;
    }
    for (; new_rows > 0; new_rows--) {
        if(fgets(text_row, len_text, stdin) == NULL){printf("error");}       // get the text
        n_rows++;                                 // update n of rows
        *last_index = *last_index + 1;

        struct Node *insert_node = malloc(sizeof(struct Node));
        insert_node->row = *last_index;
        insert_node->index = 0;
        insert_node->n_instr = -1;
        insert_node->color = 'r';
        Text *text = malloc(sizeof(Text));
        text->size = 10;
        text->used = 1;
        text->text_arr = malloc(10 * sizeof(char *));
        insert_node->text = text;
        text->text_arr[0] = malloc(strlen(text_row) + 2);
        strcpy(insert_node->text->text_arr[0], text_row);
        insert_node->left = insert_node->right = insert_node->parent = NULL;
        new_node = Insert_successor(root, new_node, insert_node);          // insert the node
    }
    // check the dot at the end of the lines
    if(fgets(dot, 3, stdin) == NULL){printf("error");}
    if (dot[0] != '.')
        printf("error");
    return n_rows;
}

// all the operations that's need to be done when a print operation occur
void P_operation(struct Node *root, int ind1, int ind2, int n_rows, struct Block_index* first_el, int last_index, Text_array textArray) {
    int n_dot;
    if (ind1 == 0 && ind2 == 0) {                // 0 0
        fputs(".\n", stdout);
    }
    else {
        if (ind1 < 0 && ind2 <= 0) {            // - - ; - 0
            if (ind1 < ind2) {
                n_dot = -ind1 + ind2 + 1;
                for (; n_dot > 0; n_dot--)
                    fputs(".\n", stdout);
            } else {
                n_dot = -ind2 + ind1 + 1;
                for (; n_dot > 0; n_dot--)
                    fputs(".\n", stdout);
            }
        } else {
            if (ind1 == 0 && ind2 < 0){             // 0 +
                n_dot = -ind2 + 1;
                for (; n_dot > 0; n_dot--)
                    fputs(".\n", stdout);
            } else {
                if (ind1 > 0 && ind2 == 0) {          // + 0
                    int temp = ind1;
                    ind1 = ind2;
                    ind2 = temp;
                }
                if (ind1 == 0 && ind2 > 0) {           // 0 +
                    fputs(".\n", stdout);
                    ind1 = 1;
                }
                // + +
                if (ind1 <= n_rows)
                    n_dot = ind2 - n_rows;
                else
                    n_dot = ind2 - ind1 + 1;

                int ind1_1 = ind1;
                int ind2_2 = ind2;

                int step = -1;
                struct Node *first = NULL;
                struct Block_index *curr = NULL;
                struct Node *curr_print = NULL;
                if (ind2 < n_rows) {
                     curr = Find_index(first_el, &ind1_1, ind1, n_rows,last_index);

                    step = ind2 - ind1;
                    ind2_2 = Find_second_index(curr,ind1_1,step);

                     first = Find_first(root, ind1_1, ind2_2);
                } else {
                    if (ind1 <= n_rows) {
                        curr = Find_index(first_el, &ind1_1, ind1, n_rows, last_index);
                        ind2_2 = last_index;
                        step = n_rows - ind1;
                        first = Find_first(root, ind1_1, ind2_2);
                    }
                }
                curr_print = Find_node(&first,ind1_1);
                step++;
                int n_text;
                while (step > 0){

                    if(curr_print->index >= textArray.used || curr_print->n_instr == -1)
                        n_text = 0;
                    else
                        n_text = textArray.num_text[curr_print->index];

                    fputs(curr_print->text->text_arr[n_text], stdout);

                    step--;

                    if(step > 0) {
                        if (curr == NULL) {
                            curr_print = Successor(curr_print);
                        } else {
                            if (ind1_1 < curr->start) {
                                if (ind1_1 != curr->start - 1) {
                                    curr_print = Successor(curr_print);
                                    ind1_1++;
                                } else {
                                    ind1_1 = curr->finish + 1;
                                    curr_print = Find_node(&first, ind1_1);
                                    curr = curr->next;
                                }
                            } else {
                                if (curr->next == NULL) {
                                    curr_print = Successor(curr_print);
                                    curr = curr->next;
                                } else {
                                    curr = curr->next;
                                    if (ind1_1 != curr->start - 1) {
                                        curr_print = Successor(curr_print);
                                        ind1_1++;
                                    } else {
                                        ind1_1 = curr->finish + 1;
                                        curr_print = Find_node(&first, ind1_1);
                                        curr = curr->next;
                                    }
                                }
                            }
                        }
                    }
                }
                for (; n_dot > 0; n_dot--)
                    fputs(".\n", stdout);
            }
        }
    }
}

void Duplicate_index_list(struct Block_index **first_el_index){
    if (*first_el_index != NULL){
        struct Block_index *curr = *first_el_index;
        struct Block_index *prev = NULL;
        while(curr != NULL){
            struct Block_index *new_block = malloc(sizeof(struct Block_index));
            new_block->start = curr->start;
            new_block->finish = curr->finish;
            if(curr->prev == NULL){
                new_block->prev = NULL;
                *first_el_index = new_block;
            }else{
                new_block->prev = prev;
                prev->next = new_block;
            }
            if(curr->next == NULL){
                new_block->next = NULL;
                //*last_el_index = new_block;
            }

            prev = new_block;
            curr = curr->next;
        }
    }
}

void In_order_change_ope(struct Node *first, int ind1, int ind2, int last_op, int first_op) {
    if (first->left != NULL && first->row > ind1)
        In_order_change_ope(first->left, ind1, ind2, last_op, first_op);

    if (first->n_instr > last_op && first->n_instr < first_op) {
        for (int i = 1; i < first->text->used ; ++i) {
            free(first->text->text_arr[i]);
        }
        first->text->used = 1;
        first->n_instr = -1;
    }

    if (first->right != NULL && first->row < ind2)
        In_order_change_ope(first->right, ind1, ind2, last_op, first_op);
}

void Fix_just_insert(struct Node *root, struct Block_index *first_el_index, int last_op, int first_op, int last_index) {
    int ind1 = 0, ind2 = 0;
    struct Node *first;
    if (first_el_index == NULL){
        ind1 = 1;
        ind2 = last_index;
        if (ind2 != 0) {
            first = Find_first(root, ind1, ind2);
            In_order_change_ope(first, ind1, ind2 ,last_op, first_op);
        }
    } else{
        while (first_el_index != NULL){

            if(first_el_index->prev == NULL)
                ind1 = 1;
            else
                ind1 = first_el_index->prev->finish + 1;

            ind2 = first_el_index->start - 1;

            if (ind2 != 0) {
                first = Find_first(root, ind1, ind2);
                In_order_change_ope(first, ind1, ind2 ,last_op, first_op);
            }

            if (first_el_index->next == NULL){
                ind1 = first_el_index->finish + 1;
                ind2 = last_index;

                if (ind1 <= ind2) {
                    first = Find_first(root, ind1, ind2);
                    In_order_change_ope(first, ind1, ind2 ,last_op, first_op);
                }
            }
            first_el_index = first_el_index->next;
        }
    }
}

int main() {
    struct Node *root = NULL;

    struct Block_index *first_el_index = NULL;

    char inst[len_instr] = "0000000000000";                                  // instruction 1,2c 2,2d ecc
    char *text_row = malloc(len_text);                          // text that need to be changed or new text

    int ind1=0,ind2=0;                                                // address of the rows or number of undo/redo
    char op;                                                      // operation c,d,u ecc
    int n_rows = 0;                                               // number of rows
    int last_index = 0;
    int last_root_index = 0;
    int redo_help = 0;
    int after_redo = 0;

    int curr_ind_page = 1;

    int check_u = 0;

    int n_oper = 0;
    int n_oper_undo_available = 0;
    int n_oper_redo_available = 0;

    int num_op = 0;
    int first_op_n = 0;
    int last_op_n = 0;

    Text_array *t_arr = malloc(sizeof(Text_array));
    t_arr->num_text = calloc(2 , sizeof t_arr->num_text);
    t_arr->old_addr = -1;
    t_arr->used = 1;
    t_arr->size = 2;

    Index_page *indexPage = malloc(sizeof(Index_page));
    indexPage->Index_list_first = malloc(5000 * sizeof indexPage->Index_list_first);
    indexPage->t_arr = malloc(5000 * sizeof(Text_array *));
    indexPage->num_rows = malloc(5000 * sizeof(int ));
    indexPage->last_indexes = malloc(5000 * sizeof(int ));
    indexPage->num_ope = malloc(5000 * sizeof(int ));
    indexPage->Index_list_first[0] = NULL;
    indexPage->t_arr[0] = t_arr;
    indexPage->num_rows[0] = 0;
    indexPage->last_indexes[0] = 0;
    indexPage->num_ope[0] = 0;
    indexPage->size = 5000;
    indexPage->used = 1;

    t_arr = NULL;
    t_arr = malloc(sizeof(Text_array));
    t_arr->num_text = calloc(100 , sizeof t_arr->num_text);
    t_arr->old_addr = -1;
    t_arr->used = 1;
    t_arr->size = 100;

    while(inst[0] != 'q'){
        if(fgets(inst,len_instr,stdin) == NULL)
            {strcpy(inst,"00");}    // read the instruction
        if(inst[0] != 'q') {
            // instruction type 1 : c,d,p
            if (strchr(inst, ',') != NULL) {
                // get ind1 ind2 and op
                sscanf(inst, "%d,%d%c", &ind1, &ind2, &op);

                // case 1 : change
                if (op == 'c') {

                    int old_rows, op_rows;

                    if (ind2 < n_rows)
                        old_rows = ind2 - ind1 + 1;  // old rows just need to change the text
                    else
                        old_rows = n_rows - ind1 + 1;
                    op_rows = ind2 - n_rows;

                    num_op++;
                    n_oper++;
                    n_oper_undo_available = n_oper;
                    n_oper_redo_available = 0;

                    int ind1_1 = ind1;
                    struct Block_index *curr = Find_index(first_el_index, &ind1_1, ind1,n_rows,last_index);

                    n_rows = C_operation(&root,text_row,n_rows,ind1,ind2,old_rows,op_rows,ind1_1,&last_index,curr, last_root_index,t_arr,num_op,first_op_n,last_op_n);

                    t_arr = Copy_list_index(&first_el_index,curr_ind_page,indexPage,t_arr,old_rows);
                    indexPage->num_rows[curr_ind_page] = n_rows;
                    indexPage->last_indexes[curr_ind_page] = last_index;
                    indexPage->num_ope[curr_ind_page] = num_op;

                    if (curr_ind_page == indexPage->used)
                        indexPage->used++;
                    curr_ind_page++;

                    if(last_index > last_root_index)
                        last_root_index = last_index;

                } else {
                    // case 2 : delete
                    if (op == 'd') {
                        if (ind1 <= 0){
                            if (ind2 <= 0){

                                num_op++;
                                n_oper++;
                                n_oper_undo_available = n_oper;
                                n_oper_redo_available = 0;

                                t_arr = Copy_list_index(&first_el_index,curr_ind_page,indexPage,t_arr,0);
                                indexPage->num_rows[curr_ind_page] = n_rows;
                                indexPage->last_indexes[curr_ind_page] = last_index;
                                indexPage->num_ope[curr_ind_page] = num_op;

                                if (curr_ind_page == indexPage->used)
                                    indexPage->used++;
                                curr_ind_page++;

                            } else
                            {
                                ind1 = 1;
                            }
                        }
                        if (ind1 > 0 && ind2 > 0) {
                            int ind1_1 = ind1;

                            num_op++;
                            n_oper++;
                            n_oper_undo_available = n_oper;
                            n_oper_redo_available = 0;

                            if (ind1 <= n_rows) {
                                struct Block_index *current = Find_index(first_el_index, &ind1_1, ind1,
                                                                         n_rows, last_index);
                                Fix_list_index(&first_el_index, current, ind1, ind2, n_rows,
                                               ind1_1);
                            }

                            if (ind1 <= n_rows) {
                                if (ind2 <= n_rows)
                                    n_rows = n_rows - (ind2 - ind1 + 1);
                                else
                                    n_rows = n_rows - (n_rows - ind1 + 1);
                            }

                            t_arr = Copy_list_index(&first_el_index,curr_ind_page,indexPage,t_arr,0);
                            indexPage->num_rows[curr_ind_page] = n_rows;
                            indexPage->last_indexes[curr_ind_page] = last_index;
                            indexPage->num_ope[curr_ind_page] = num_op;

                            if (curr_ind_page == indexPage->used)
                                indexPage->used++;
                            curr_ind_page++;
                        }
                    } else {
                        // case 3 : print
                        if (op == 'p') {
                            P_operation(root,ind1,ind2,n_rows, first_el_index, last_index, *t_arr);
                        }
                    }
                }
            }
                // instruction type 2 : u,r
            else {
                // get ind1 and op
                sscanf(inst, "%d%c", &ind1, &op);
                if (ind1 > 0) {
                    // case 1 : undo
                    if (op == 'u') {
                        int n_undo = 0;
                        after_redo = 0;
                        redo_help = 0;

                        int free_help = 0;
                        int last_undo_step = ind1;
                        char last_op = 'u';

                        n_undo = n_undo + ind1;
                        if (n_undo > n_oper_undo_available) { n_undo = n_oper_undo_available; }


                        // until the first change or delete occur
                        while ((op == 'u' || op == 'r' || op == 'p') && inst[0] != 'q') {
                            if (fgets(inst, len_instr, stdin) == NULL) { printf("error"); }
                            if (inst[0] != 'q') {
                                if (strchr(inst, ',') != NULL) {
                                    sscanf(inst, "%d,%d%c", &ind1, &ind2, &op);

                                    if (n_undo != 0)
                                    {
                                        free(t_arr->num_text);
                                        free(t_arr);
                                        if (free_help == 0) {
                                            while (first_el_index != NULL) {
                                                if (first_el_index->next != NULL) {
                                                    first_el_index = first_el_index->next;
                                                    free(first_el_index->prev);
                                                } else {
                                                    free(first_el_index);
                                                    first_el_index = NULL;
                                                }
                                            }
                                        }
                                    }

                                    // if print occur do the undo and then print
                                    if (op == 'p') {
                                        free_help = 1;
                                        curr_ind_page = Fix_index_u_r(*indexPage,&first_el_index,curr_ind_page,n_undo,&n_rows,&last_index, &t_arr);

                                        n_oper_undo_available = n_oper_undo_available - n_undo;
                                        n_oper_redo_available = n_oper - n_oper_undo_available;
                                        n_undo = 0;
                                        P_operation(root, ind1, ind2, n_rows, first_el_index,
                                                    last_index, *t_arr);
                                    }
                                        // there's a change or delete so do the undo and then free the undo stack
                                    else {
                                        curr_ind_page = Fix_index_u_r(*indexPage,&first_el_index,curr_ind_page,n_undo,&n_rows,&last_index, &t_arr);
                                        Duplicate_index_list(&first_el_index);
                                        last_op_n = indexPage->num_ope[curr_ind_page - 1];
                                        first_op_n = num_op + 1;

                                        free_help = 0;

                                        Fix_just_insert(root,first_el_index,last_op_n,first_op_n,last_index);

                                        n_undo = 0;
                                        n_oper = curr_ind_page - 1;
                                        n_oper_undo_available = n_oper;
                                        n_oper_redo_available = 0;

                                        if (op == 'c') {

                                            int old_rows, op_rows;
                                            if (ind2 < n_rows)
                                                old_rows = ind2 - ind1 + 1;  // old rows just need to change the text
                                            else
                                                old_rows = n_rows - ind1 + 1;
                                            op_rows = ind2 - n_rows;

                                            num_op++;
                                            n_oper++;
                                            n_oper_undo_available = n_oper;
                                            n_oper_redo_available = 0;

                                            int ind1_1 = ind1;
                                            struct Block_index *curr = Find_index(first_el_index, &ind1_1, ind1, n_rows,last_index);

                                            n_rows = C_operation(&root, text_row, n_rows, ind1, ind2,old_rows, op_rows, ind1_1,
                                                                 &last_index, curr, last_root_index,t_arr, num_op, first_op_n, last_op_n);

                                            t_arr = Copy_list_index(&first_el_index,curr_ind_page,indexPage,t_arr,old_rows);
                                            indexPage->num_rows[curr_ind_page] = n_rows;
                                            indexPage->last_indexes[curr_ind_page] = last_index;
                                            indexPage->num_ope[curr_ind_page] = num_op;

                                            if (curr_ind_page == indexPage->used)
                                                indexPage->used++;
                                            curr_ind_page++;


                                            if(last_index > last_root_index)
                                                last_root_index = last_index;
                                        } else {
                                            if (ind1 <= 0){
                                                if (ind2 <= 0){

                                                    num_op++;
                                                    n_oper++;
                                                    n_oper_undo_available = n_oper;
                                                    n_oper_redo_available = 0;

                                                    t_arr = Copy_list_index(&first_el_index,curr_ind_page,indexPage, t_arr,0);
                                                    indexPage->num_rows[curr_ind_page] = n_rows;
                                                    indexPage->last_indexes[curr_ind_page] = last_index;
                                                    indexPage->num_ope[curr_ind_page] = num_op;

                                                    if (curr_ind_page == indexPage->used)
                                                        indexPage->used++;
                                                    curr_ind_page++;

                                                } else
                                                {
                                                    ind1 = 1;
                                                }
                                            }
                                            if (ind1 > 0 && ind2 > 0) {
                                                int ind1_1 = ind1;

                                                num_op++;
                                                n_oper++;
                                                n_oper_undo_available = n_oper;
                                                n_oper_redo_available = 0;

                                                if (ind1 <= n_rows) {
                                                    struct Block_index *current_index = Find_index(first_el_index, &ind1_1, ind1,
                                                                                             n_rows, last_index);
                                                    Fix_list_index(&first_el_index, current_index, ind1, ind2, n_rows,
                                                                   ind1_1);
                                                }

                                                if (ind1 <= n_rows) {
                                                    if (ind2 <= n_rows)
                                                        n_rows = n_rows - (ind2 - ind1 + 1);
                                                    else
                                                        n_rows = n_rows - (n_rows - ind1 + 1);
                                                }

                                                t_arr = Copy_list_index(&first_el_index,curr_ind_page,indexPage,t_arr,0);
                                                indexPage->num_rows[curr_ind_page] = n_rows;
                                                indexPage->last_indexes[curr_ind_page] = last_index;
                                                indexPage->num_ope[curr_ind_page] = num_op;

                                                if (curr_ind_page == indexPage->used)
                                                    indexPage->used++;
                                                curr_ind_page++;
                                            }
                                        }
                                    }
                                }
                                    // there's another undo or redo so modify the number of undo to do
                                else {
                                    sscanf(inst, "%d%c", &ind1, &op);
                                    if (ind1 > 0) {
                                        if (op == 'u') {
                                            n_undo = n_undo + ind1;
                                            if (n_undo > n_oper_undo_available) { n_undo = n_oper_undo_available; }
                                            after_redo = 0;
                                            last_undo_step = ind1;
                                            last_op = 'u';
                                        } else {
                                            n_undo = n_undo - ind1;
                                            if (n_undo < -n_oper_redo_available) { n_undo = -n_oper_redo_available; }
                                            after_redo = 1;
                                            last_op = 'r';
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    exit(0);
}