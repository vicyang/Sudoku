/*
    Dancing Links算法解数独，C语言实现
    枚举所有终盘类型

    523066680@163.com 2017-09
    https://github.com/vicyang/Sudoku
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct node
{
    int row;
    int col;
    int val;
    int count;
    struct node * left;
    struct node * right;
    struct node * up;
    struct node * down;
    struct node * top;
};

struct game
{
    int id;
    char s[100];
    struct game *next;
};

//81格*9种数字*4个位置
#define MAX_ELE_COUNT 81*9*4

int res_count = 0;
int ele_id;
struct node *(backup[ MAX_ELE_COUNT ]);

void print_sudo_inline( int mat[9][9] );

// 传入 C列标，用来与新元素关联
void elements_to_nodes ( 
    struct node * C, int cols, int *eles,
    int r, int c, int n )
{
    struct node * ele;
    struct node * first;
    struct node * prev = NULL;
    static int x, col;

    prev = NULL;
    for (x = 0; x < cols; x++ )
    {
        col = x+1;
        if ( eles[x] == 1 )
        {
            ele = malloc( sizeof(struct node) );
            backup[ele_id] = ele;

            ele->row = r;   //real row in array
            ele->val = n;
            ele->col = c;   //real column in array
            ele->up   = C[col].up;
            ele->down = &C[col];
            ele->top  = &C[col];
            C[col].count ++;
            
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
            {
                first = ele;
            }

            prev = ele;
            ele_id++;
        }
    }

    first->left = ele;
    ele->right  = first;
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
            hz->top->count --;
        }
        hz->top->count --;
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
            hz->top->count ++;
        }
        hz->top->count ++;
    }
}

int dance( struct node * head, struct node *(result[80]), int mat[9][9], int lv )
{
    if ( head->right == head )
    {
        for (int i = 1; i < 81; i++)
            if ( result[i] != NULL )
                mat[ result[i]->row ][ result[i]->col ] = result[i]->val;

        print_sudo_inline(mat);

        for (int i = 1; i < 81; i++)
            if ( result[i] != NULL )
                mat[ result[i]->row ][ result[i]->col ] = 0;

        res_count++;
        if ( res_count % 1000000 == 0 )
            fprintf(stderr, "%d\n", res_count);

        return 1;
    }

    struct node * c = head->right;
    struct node * r;
    struct node * ele;

    struct node * min = c;
    for ( ; c != head; c = c->right )
        if ( c->count < min->count ) min = c;
    
    c = min;
    if ( c->count <= 0) return 0;

    remove_col( c );

    r = c->down;
    while ( r != c )
    {
        result[lv] = r;
        ele = r->right;

        do
        {
            remove_col( ele->top );
            ele = ele->right;
        }
        while ( ele != r );

        dance( head, result, mat, lv+1 );

        ele = r->left;
        do
        {
            resume_col( ele->top );
            ele = ele->left;
        }
        while ( ele != r );

        r = r->down;
    }

    resume_col( c );
    return 0;
}

void trans( int r, int c, int n, int *elements )
{
    int A,B,C,D;
    int block;

    block = (int)(r/3) * 3 + (int)(c/3);

    A = r * 9 + c;
    B = r * 9 + n + 80;
    C = c * 9 + n + 161;
    D = block * 9 + n + 242;

    elements[A] = 1;
    elements[B] = 1;
    elements[C] = 1;
    elements[D] = 1;
}

void str_to_mat( char *s, int mat[9][9] )
{
    int r, c;
    static int delta = '0' - 0;

    for (int i = 0; i < 81; i++ )  //r,c from 0,0 to 8,8
    {
        r = (int)(i/9);
        c = (int)(i%9);
        mat[r][c] = s[i] - delta;  //char to integer
    }
}

void get_possible_num( int mat[9][9], int row, int col, int *possible )
{
    int blkr = (int)(row/3);
    int blkc = (int)(col/3);
    int rr, cc;

    for ( int idx = 0; idx < 9; idx++ ) possible[idx] = idx+1;

    for ( int idx = 0; idx < 9; idx++  )
    {
        rr = blkr*3 + (int)(idx/3);
        cc = blkc*3 + (int)(idx%3);

        if ( mat[idx][col] != 0 ) possible[ mat[idx][col]-1 ] = 0;
        if ( mat[row][idx] != 0 ) possible[ mat[row][idx]-1 ] = 0;
        if ( mat[rr][cc] != 0 ) possible[ mat[rr][cc]-1 ] = 0;
    }
}

void init_head_node( struct node * C, int cols )
{
    //创建链表头
    int left, right;
    for ( int col = 0; col <= cols; col++ )
    {
        left  = col == 0 ? cols : col-1;
        right = col == cols ? 0 : col+1;

        C[col].col = col;
        C[col].row = 0;
        C[col].count = 0;
        C[col].left  = &C[left];
        C[col].right = &C[right];
        C[col].up    = &C[col];
        C[col].down  = &C[col];
    }
}

void create_matrix_nodes(  int mat[9][9], struct node * C )
{
    int elements[324];
    int possible[9];  //可选数字，get_possible_num 会将元素先初始化再处理

    int r, c, idx;

    //数独游戏转矩阵，同时转链表位元素
    memset( elements, 0, sizeof(elements) );
    //将多个元素数据堆入同一行节点
    for ( r = 0; r < 9; r++ )
        for ( c = 0; c < 9; c++ )
            if ( mat[r][c] != 0 )
                trans( r, c, mat[r][c], elements );

    elements_to_nodes( C, 324, elements, 0, 0, 0 );

    //将空缺单元的可能选项依次转为不同行的节点
    for ( int r = 0; r < 9; r++ )
    {
        for ( int c = 0; c < 9; c++ )
        {
            if ( mat[r][c] == 0 )
            {
                get_possible_num( mat, r, c, possible );
                
                for (int idx = 0; idx < 9; idx++ )
                {
                    if ( possible[idx] != 0 )
                    {
                        memset( elements, 0, sizeof(elements) );
                        trans( r, c, possible[idx], elements );
                        elements_to_nodes( C, 324, elements, r, c, possible[idx] );
                    }
                }
            }
        }
    }
}

void print_mat( int mat[9][9] )
{
    int r, c;
    for (r = 0; r < 9; r++ )
    {
        for (c = 0; c < 9; c++ )
            printf("%d ", mat[r][c] );

        printf("\n");
    }
}

void print_sudo_inline( int mat[9][9] )
{
    int r, c;
    for (r = 0; r < 9; r++ )
    {
        for (c = 0; c < 9; c++ )
            printf("%d", mat[r][c] );
    }

    printf("\n");
}

void clean_matrix_nodes( struct node *C )
{
    for (int i = 0; i < ele_id; i++)
        free(backup[i]);
}

int main(int argc, char *argv[] ) 
{
    int res;
    int cols = 324;
    int mat[9][9];

    struct node * head;
    struct node * C = (struct node *) malloc( sizeof(struct node) * (cols+1) );
    struct node *(result[81]);
    head = &C[0];

    char game[81] = "123456789456789123789123456312645978000000000000000000000000000000000000000000000";
    //char game[81] = "123456789456789123789123456312600000000000000000000000000000000000000000000000000";
    
    ele_id = 0;
    init_head_node( C, cols );
    str_to_mat( game, mat );
    create_matrix_nodes( mat, C );
    
    for (int i = 0; i < 81; i++) result[i] = NULL;
    res = dance(head, result, mat, 0);

    fprintf(stderr, "\n%.3f\n", (float)clock()/(float)CLOCKS_PER_SEC );

    return 0;
}

