/*
    Dancing Links in C Language
    Auth: vicyang
    Mail: 523066680@163.com
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node
{
    int val;
    int row;
    int col;
    struct node * left;
    struct node * right;
    struct node * up;
    struct node * down;
    struct node * top;
};

int ele_id = 1;
int dance( struct node * head, int *result, int lv );
void remove_col( struct node * node  );
void resume_col( struct node * node  );
void elements_to_nodes( struct node * C, int current_row, int cols, int *eles  );
void print_nodes( struct node *C, int rows, int cols );

int main(int argc, char *argv[] ) 
{
    int mat[6][7] = {
        {0,0,1,0,1,1,0},
        {1,0,0,1,0,0,1},
        {0,1,1,0,0,1,0},
        {1,0,0,1,0,0,0},
        {0,1,0,0,0,0,1},
        {0,0,0,1,1,0,1}
    };

    int rows = sizeof(mat)/sizeof(mat[0]);
    int cols = sizeof(mat[0])/sizeof(int);

    int left, right;
    struct node * head;
    struct node * C = (struct node *) malloc( sizeof(struct node) * (cols+1) );
    head = &C[0];

    //初始化C列标
    for ( int col = 0; col <= cols; col++ )
    {
        left  = col == 0 ? cols : col-1;
        right = col == cols ? 0 : col+1;
        C[col].col = col;
        C[col].row = 0;
        C[col].left  = &C[left];
        C[col].right = &C[right];
        C[col].up    = &C[col];
        C[col].down  = &C[col];
    }

    for ( int row = 0; row < rows; row++ )
        elements_to_nodes( C, row, cols, mat[row] );

    print_nodes( C, rows, cols );

    int *result = (int *)malloc( sizeof(int) * rows );
    for (int i = 0; i < rows; i++) result[i] = 0;

    dance(head, result, 0);

    printf("\nResult: ");
    for (int i = 0; i < rows; i++)
        if (result[i] != 0)
            printf("%d ", result[i]);

    return 0;
}

int dance( struct node * head, int *result, int lv )
{
    int res;
    if ( head->right == head )
        return 1;

    struct node * c = head->right;
    struct node * r = c->down;
    struct node * ele;

    while ( r != c )
    {
        remove_col( r->top );
        for (ele = r->right; ele != r; ele = ele->right)
            remove_col( ele->top );

        res = dance( head, result, lv+1 );
        if ( res == 1 )
        {
            result[lv] = r->row;
            return 1;
        }

        resume_col( r->top );
        for (ele = r->right; ele != r; ele = ele->right)
            resume_col( ele->top );

        r = r->down;
    }

    return res;
}

void remove_col( struct node * node  )
{
    node->left->right = node->right;
    node->right->left = node->left;

    struct node * vt = node->down;
    struct node * hz;

    for ( ; vt != node; vt = vt->down )
    {
        hz = vt->right;
        for (  ; hz != vt; hz = hz->right )
        {
            hz->up->down  = hz->down;
            hz->down->up  = hz->up;
        }
    }
}

void resume_col( struct node * node  )
{
    node->left->right = node;
    node->right->left = node;

    struct node * vt = node->down;
    struct node * hz;

    for ( ; vt != node; vt = vt->down )
    {
        hz = vt->right;
        for (  ; hz != vt; hz = hz->right )
        {
            hz->up->down  = hz;
            hz->down->up  = hz;
        }
    }
}

void elements_to_nodes( struct node * C, int current_row, int cols, int *eles  )
{
    struct node * ele;
    struct node * first;
    struct node * prev = NULL;
    int col;

    prev = NULL;
    for (int x = 0; x < cols; x++ )
    {
        col = x+1;
        if ( eles[x] == 1 )
        {
            ele = malloc( sizeof(struct node) );
            ele->row = current_row+1;  //begin from 1
            ele->val = ele_id;
            ele->col = col;
            ele->up    = C[col].up;
            ele->down  = &C[col];
            ele->top = &C[col];
            
            if ( C[col].down == &C[col] ) 
                C[col].down = ele;

            C[col].up->down = ele;
            C[col].up = ele;

            if ( prev != NULL )
            {
                prev->right = ele;
                ele->left   = prev;
            }
            else
                first = ele;

            prev = ele;
            ele_id++;
        }
    }

    first->left = ele;
    ele->right  = first;
}

void print_nodes( struct node *C, int rows, int cols )
{
    struct node *(ref[100]);
    int col, row;

    printf("TOP ");
    for (col = 1; col <= cols; col++)
    {
        ref[col] = &C[col];
        printf("C%02d ", ref[col]->col );
    }
    printf("\n");

    for (row = 1; row <= rows; row++ )
    {
        printf("R%02d ", row);
        for (col = 1; col <= cols; col++)
        {
            if ( ref[col]->down->row == row )
            {
                ref[col] = ref[col]->down;
                printf("%3d ", ref[col]->val );
            }
            else
            {
                printf("    ");
            }
        }
        printf("\n");
    }
}