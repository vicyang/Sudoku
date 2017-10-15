/*
    DLX 算法解数独题，C语言版
    523066680 2017-10
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct node {
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

struct spare {
    int n;
    int possible[9];
};

struct game {
    int id;
    char s[100];
    struct game *next;
};

struct ref {
    int *(rows[9]);
    int *(cols[9]);
    int *(blks[9]);
    int *self;
};

struct unsolve {
    int r, c;
    struct spare *spare;
};

struct position { 
    int r, c;
};

int COLS[9];
int ROWS[9];
int BLKS[9];
int block_id[9][9];
struct position block_ele[9][9];

//81格*9种数字*4个位置
#define MAX_ELE_COUNT 81*9*4

int ele_id;
//静态分配节点空间到数组，重复利用。
struct node backup[ MAX_ELE_COUNT ];
//建表，用于查询可选数
struct spare sparenums[0b1111111111];

void str_to_mat( char *s, int mat[9][9] );
void load_games( struct game * games, char *filename  );
void print_mat( int mat[9][9] );
void print_sudo_inline( int mat[9][9] );

void build_tables( struct spare *sparenums );
void init_bit_model( int mat[9][9] );
void set_block_index( int block_id[9][9] );

void get_possible_num( int row, int col, struct spare **possible );
int fill_one_possible_num( int mat[9][9], struct unsolve unsolved[81], int n );
int get_unsolved_list(int mat[9][9], struct unsolve unsolved[81] );

void init_head_node( struct node * C, int cols );
void trans( int r, int c, int n, int *elements );
void elements_to_nodes ( struct node * C, int cols, int *eles, int r, int c, int n );
void create_matrix_nodes(  int mat[9][9],  struct unsolve unsolved[81], int n, struct node * C );
int dance( struct node * head, struct node *(result[80]), int lv );
void remove_col( struct node * node  );
void resume_col( struct node * node  );

int main(int argc, char *argv[] ) 
{
    int cols = 324;
    int i, n, res;
    int mat[9][9];

    struct node * head;
    struct node * C = (struct node *) malloc( sizeof(struct node) * (cols+1) );
    struct node *(result[81]);
    head = &C[0];

    struct game *gamenode = (struct game *)malloc( sizeof(struct game) );
    load_games( gamenode, "../../Puzzles/sudoku_nd3.txt" );

    build_tables(sparenums);
    set_block_index(block_id);

    struct unsolve unsolved[81];

    while ( gamenode->next != NULL )
    {
        ele_id = 0;
        init_head_node( C, cols );
        str_to_mat( gamenode->s, mat );

        init_bit_model(mat);

        //找出唯一可选数字并填充
        n = get_unsolved_list( mat, unsolved );
        while ( fill_one_possible_num( mat, unsolved, n ) ) {
            n = get_unsolved_list( mat, unsolved ); 
        }

        create_matrix_nodes( mat, unsolved, n, C );
        
        for (i = 0; i < 81; i++) result[i] = NULL;
        res = dance(head, result, 0);

        if (res == 1)
            for (i = 1; i < 81; i++)   //0是固有单元的数据，所以从1开始
                if ( result[i] != NULL )
                    mat[ result[i]->row ][ result[i]->col ] = result[i]->val;

        print_sudo_inline(mat);
        //printf("Game index: %d %d\n", gamenode->id, res );

        gamenode = gamenode->next;
        //break;
    }

    fprintf(stderr, "%.3f\n", (float)clock()/(float)CLOCKS_PER_SEC );

    return 0;
}


//=================================================================
//                             Possible Number
//=================================================================

int get_unsolved_list(int mat[9][9], struct unsolve unsolved[81] )
{
    static int r, c;
    int idx = 0;

    for ( r = 0; r < 9; r++ )
        for ( c = 0; c < 9; c++ ) 
            if ( mat[r][c] == 0 )
            {
                unsolved[idx].r = r;
                unsolved[idx].c = c;
                get_possible_num( r, c, &unsolved[idx].spare );
                idx++;
            }

    return idx;
}

void get_possible_num( int row, int col, struct spare **possible )
{
    static int model;
    model = COLS[col] | ROWS[row] | BLKS[ block_id[row][col] ];
    //model = model >> 1 << 1;  //replace last bit to zero
    *possible = &sparenums[model];
}

int fill_one_possible_num( int mat[9][9], struct unsolve unsolved[81], int n )
{
    static int r, c;
    int res = 0;
    int fill = 0;
    int possible[10];

    int cells[9][9] = {
        {-1,-1,-1,-1,-1,-1,-1,-1,-1}, 
        {-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1},
        {-1,-1,-1,-1,-1,-1,-1,-1,-1}
    };

    static int idx;
    for (idx = 0; idx < n; idx++ )
    {
        //定义坐标-索引映射表
        cells[unsolved[idx].r][unsolved[idx].c] = idx;
        //填充只有1个可选元素的单元，更新COLS, ROWS, BLKS
        if ( unsolved[idx].spare->n == 1 )
        {
            r = unsolved[idx].r;
            c = unsolved[idx].c;            
            mat[r][c] = unsolved[idx].spare->possible[0];
            //printf("Fill %d,%d  %d\n", r, c, mat[r][c]);
            COLS[c] |= 1 << mat[r][c];
            ROWS[r] |= 1 << mat[r][c];
            BLKS[ block_id[r][c] ] |= 1 << mat[r][c];

            fill++;
        }
    }

    //ROW
    for (r = 0; r < 9; r++)
    {
        memset(possible, 0, sizeof(possible) );
        for (c = 0; c < 9; c++)
        {
            if ( cells[r][c] != -1 ) 
            {
                idx = cells[r][c];
                for ( int e = 0; e < unsolved[idx].spare->n; e++)
                {
                    possible[ unsolved[idx].spare->possible[e] ]++;
                }
            }
        }

        for (c = 0; c < 9; c++)
        {
            if ( cells[r][c] != -1 ) 
            {
                idx = cells[r][c];
                for ( int e = 0; e < unsolved[idx].spare->n; e++)
                {
                    if ( possible[ unsolved[idx].spare->possible[e] ] == 1 )
                    {
                        mat[r][c] = unsolved[idx].spare->possible[e];
                        //printf("Fill %d,%d  %d\n", r, c, mat[r][c]);

                        COLS[c] |= 1 << mat[r][c];
                        ROWS[r] |= 1 << mat[r][c];
                        BLKS[ block_id[r][c] ] |= 1 << mat[r][c];

                        fill++;
                    }
                }
            }
        }
    }

    //Cols
    for (c = 0; c < 9; c++)
    {
        memset(possible, 0, sizeof(possible) );
        for (r = 0; r < 9; r++)
        {
            if ( cells[r][c] != -1 ) 
            {
                idx = cells[r][c];
                for ( int e = 0; e < unsolved[idx].spare->n; e++)
                {
                    possible[ unsolved[idx].spare->possible[e] ]++;
                }
            }
        }

        for (r = 0; r < 9; r++)
        {
            if ( cells[r][c] != -1 ) 
            {
                idx = cells[r][c];
                for ( int e = 0; e < unsolved[idx].spare->n; e++)
                {
                    if ( possible[ unsolved[idx].spare->possible[e] ] == 1 )
                    {
                        mat[r][c] = unsolved[idx].spare->possible[e];
                        //printf("Fill %d,%d  %d\n", r, c, mat[r][c]);

                        COLS[c] |= 1 << mat[r][c];
                        ROWS[r] |= 1 << mat[r][c];
                        BLKS[ block_id[r][c] ] |= 1 << mat[r][c];

                        fill++;
                    }
                }
            }
        }
    }

    //block
    static int rr, cc;
    static int blk, in;

    for (blk = 0; blk < 9; blk++)
    {
        memset(possible, 0, sizeof(possible) );
        for (in = 0; in < 9; in++)
        {
            rr = block_ele[blk][in].r;
            cc = block_ele[blk][in].c;
            if ( cells[rr][cc] != -1 ) 
            {
                idx = cells[rr][cc];
                for ( int e = 0; e < unsolved[idx].spare->n; e++)
                {
                    possible[ unsolved[idx].spare->possible[e] ]++;
                }
            }
        }

        for (in = 0; in < 9; in++)
        {
            rr = block_ele[blk][in].r;
            cc = block_ele[blk][in].c;
            if ( cells[rr][cc] != -1 ) 
            {
                idx = cells[rr][cc];
                for ( int e = 0; e < unsolved[idx].spare->n; e++)
                {
                    if ( possible[ unsolved[idx].spare->possible[e] ] == 1 )
                    {
                        mat[rr][cc] = unsolved[idx].spare->possible[e];
                        //printf("Fill %d,%d  %d\n", rr, cc, mat[rr][cc]);

                        // int blk = (int)(r/3)*3 + (int)(c/3);
                        COLS[cc] |= 1 << mat[rr][cc];
                        ROWS[rr] |= 1 << mat[rr][cc];
                        BLKS[blk] |= 1 << mat[rr][cc];

                        fill++;
                    }
                }
            }
        }
    }

    if (fill > 0) res = 1;
    return res;
}

void init_bit_model( int mat[9][9] )
{
    static int m, n;
    static int rr, cc;

    for (m = 0; m < 9; m++)
    {
        COLS[m] = 0, ROWS[m] = 0, BLKS[m] = 0;
        for (n = 0; n < 9; n++)
        {
            rr = block_ele[m][n].r;
            cc = block_ele[m][n].c;
            COLS[m] |= 1 << mat[n][m];
            ROWS[m] |= 1 << mat[m][n];
            BLKS[m] |= 1 << mat[rr][cc];
        }
        COLS[m] = COLS[m] >> 1 << 1;
        ROWS[m] = ROWS[m] >> 1 << 1;
        BLKS[m] = BLKS[m] >> 1 << 1;
    }
}

void build_tables( struct spare *sparenums )
{
    int t;
    for (int i = 0; i <= 0b1111111110; i+=2 )
    {
        t = 0;
        for ( int j = 1; j <= 9; j++)
            if ( (1 << j & i) == 0 )
            {
                sparenums[i].possible[t] = j;
                t++;
            }

        sparenums[i].n = t;
    }
}

void set_block_index( int block_id[9][9] )
{
    static int m, n;
    for (m = 0; m < 9; m++)
        for (n = 0; n < 9; n++)
        {
            //m:row, n:col
            block_id[m][n] = (int)(m/3)*3 + (int)(n/3);

            //m:block_id, n:block_element_id
            block_ele[m][n].r = (int)(m/3)*3 + (int)(n/3); 
            block_ele[m][n].c = m % 3 * 3 + n % 3;
        }
}

//=================================================================
//                         Dancing
//=================================================================

int dance( struct node * head, struct node *(result[80]), int lv )
{
    if ( head->right == head )
        return 1;

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
        ele = r->right;

        do
        {
            remove_col( ele->top );
            ele = ele->right;
        }
        while ( ele != r );

        if ( dance( head, result, lv+1 ) == 1 )
        {
            result[lv] = r;
            return 1;
        }

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

//=================================================================
//                        Build Nodes Matrix
//=================================================================

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

void create_matrix_nodes(  int mat[9][9],  struct unsolve unsolved[81], int n, struct node * C )
{
    int elements[324];
    static int r, c, idx, i;

    //数独提示数转为特征序列
    memset( elements, 0, sizeof(elements) );
    //将多个元素数据堆入同一行节点
    for ( r = 0; r < 9; r++ )
        for ( c = 0; c < 9; c++ )
            if ( mat[r][c] != 0 )
                trans( r, c, mat[r][c], elements );

    elements_to_nodes( C, 324, elements, 0, 0, 0 );

    for (idx = 0; idx < n; idx++ )
    {
        for (i = 0; i < unsolved[idx].spare->n; i++ )
        {
            memset( elements, 0, sizeof(elements) );
            trans( unsolved[idx].r, unsolved[idx].c, unsolved[idx].spare->possible[i], elements );
            elements_to_nodes( C, 324, elements, unsolved[idx].r, unsolved[idx].c, unsolved[idx].spare->possible[i] );
        }
    }
}

// 传入 C列标，用来与新元素关联
void elements_to_nodes ( 
    struct node * C, int cols, int *eles,
    int r, int c, int n )
{
    struct node * ele;
    struct node * first;
    struct node * prev;
    static int col;
    static int x;

    prev = NULL;
    for ( x = 0; x < cols; x++ )
    {
        col = x+1;
        if ( eles[x] == 1 )
        {
            ele = &backup[ele_id];

            ele->row = r;   //real row index in mat
            ele->val = n;
            ele->col = c;   //real col index in mat
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

void trans( int r, int c, int n, int *elements )
{
    static int A,B,C,D;
    static int block;

    block = block_id[r][c];

    A = r * 9 + c;
    B = r * 9 + n + 80;
    C = c * 9 + n + 161;
    D = block * 9 + n + 242;

    elements[A] = 1;
    elements[B] = 1;
    elements[C] = 1;
    elements[D] = 1;
}

//=================================================================
//                      Format and Print
//=================================================================

void str_to_mat( char *s, int mat[9][9] )
{
    static int i, r, c;
    static int delta = '0' - 0;

    for (i = 0; i < 81; i++ )  //r,c from 0,0 to 8,8
    {
        r = (int)(i/9);
        c = (int)(i%9);
        mat[r][c] = s[i] - delta;  //char to integer
    }
}

void print_mat( int mat[9][9] )
{
    static int r, c;
    for (r = 0; r < 9; r++ )
    {
        for (c = 0; c < 9; c++ )
            printf("%d ", mat[r][c] );

        printf("\n");
    }
}

void print_sudo_inline( int mat[9][9] )
{
    static int r, c;
    for (r = 0; r < 9; r++ )
        for (c = 0; c < 9; c++ )
            printf("%d", mat[r][c] );

    printf("\n");
}

//=================================================================
//                           Load Game
//=================================================================

void load_games( struct game * games, char *filename  )
{
    struct game *node;
    node = games;
    node->id = 1;

    FILE *fp;
    fp = fopen( filename, "r" );

    int id = 1;
    while ( ! feof( fp ) )
    {
        fgets( node->s, 100, fp );
        //如果读取到空行或者残缺行，提前break，下一节点为 NULL
        if ( (int)strlen(node->s) < 81 ) break;

        node->id = id++;
        node->next = (struct game *) malloc( sizeof(struct game) );
        node = node->next;
    }

    node->next = NULL;
}