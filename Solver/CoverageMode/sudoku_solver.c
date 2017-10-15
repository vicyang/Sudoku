/*
    方案：枚举覆盖模式
    523066680 2017-10
    opt 根据每个数字的覆盖集合数排序，较少集合优先
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct possible {
    int n;
    int cols[9];
};

struct game {
    int id;
    char s[100];
    struct game *next;
};

void str_to_mat( char *s, int mat[9][9] );
void print_mat( int mat[9][9] );
void print_sudo_inline( int mat[9][9] );
void load_games( struct game * games, char *filename  );

void get_possible_col(int mat[9][9], int row, int target, int cols[9] )
{
    for ( int c = 0; c < 9; c++ ) cols[c] = 1;

    for ( int c = 0; c < 9; c++ )
    {
        if ( mat[row][c] == target ) 
        {
            for ( int idx = 0; idx < 9; idx++ ) cols[idx] = 0;
            cols[c] = 1;
            break;
        }
        else if ( mat[row][c] != 0 )
        {
            cols[c] = 0;
        }

        for (int r = 0; r < 9; r++)
        {
            if ( mat[r][c] == target )
            {
                cols[c] = 0;
                if ( (int)(r/3) == (int)(row/3) )
                {
                    cols[ (int)(c/3)*3+0 ] = 0;
                    cols[ (int)(c/3)*3+1 ] = 0;
                    cols[ (int)(c/3)*3+2 ] = 0;
                }
            }
        }
    }
}

void enumerate(int mat[9][9], int row, int target, int *count )
{
    int cols[9];
    get_possible_col( mat, row, target, cols );

    if (row >= 9 ) 
    {
        *count += 1; // wrong in *count++
        return;
    }

    int t;
    for (int c = 0; c < 9; c++ )
    {
        if ( cols[c] != 0 )
        {
            t = mat[row][c];
            mat[row][c] = target;
            enumerate( mat, row+1, target, count );
            mat[row][c] = t;
        }
    }
}

void solve(int mat[9][9], int row, int oidx, int order[9] )
{
    int target;
    int cols[9];

    target = order[oidx];
    get_possible_col( mat, row, target, cols );

    if ( oidx >= 9 )
    {
        print_sudo_inline( mat );
        return;
    }

    if (row >= 9 ) 
    {
        solve( mat, 0, oidx+1, order );     //新的数字，从头开始
        return;
    }

    int t;
    for (int c = 0; c < 9; c++ )
    {
        if ( cols[c] != 0 )
        {
            t = mat[row][c];
            mat[row][c] = target;
            solve( mat, row+1, oidx, order );  //同一个数字，下一行
            mat[row][c] = t;
        }
    }
}

int main(int argc, char *argv[] ) 
{
    int mat[9][9];
    int time_a;

    struct game *gamenode = (struct game *)malloc( sizeof(struct game) );
    load_games( gamenode, "../../puzzles/sudoku_nd3.txt" );

    int t;
    int order[9];
    int score[9];
    int idx;
    int count;

    while ( gamenode->next != NULL )
    {
        time_a = clock();
        str_to_mat( gamenode->s, mat );

        idx = 0;
        for (int num = 1; num <= 9; num++ )
        {
            count = 0;
            enumerate(mat, 0, num, &count);  //mat mat2 begin_row target_num count
            order[idx] = num;
            score[idx] = count;

            if (idx > 0)
            {   //排序，较大的数字往后
                for ( int i = 0; i < idx; i++ )
                {
                    if ( score[i] > score[idx] )
                    {
                        t = score[idx];
                        score[idx] = score[i];
                        score[i] = t;
                        t = order[idx];
                        order[idx] = order[i];
                        order[i] = t;
                    }
                }
            }
            idx++;
        }

        solve( mat, 0, 0, order); //mat beginrow order_index order_array

        printf("Game index: %d, Time used: %.3f\n", 
            gamenode->id, 
            (float)(clock()-time_a)/(float)CLOCKS_PER_SEC
        );

        gamenode = gamenode->next;
        //break;
    }

    fprintf(stderr, "Time used: %.3f\n\n", (float)clock()/(float)CLOCKS_PER_SEC );

    return 0;
}

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

void print_mat( int mat[9][9] )
{
    static int r, c;
    for ( r = 0; r < 9; r++ )
    {
        for ( c = 0; c < 9; c++ ) 
            printf("%d ", mat[r][c] );

        printf("\n");
    }
}

void print_sudo_inline( int mat[9][9] )
{
    int r, c;
    for (r = 0; r < 9; r++ )
        for (c = 0; c < 9; c++ )
            printf("%d", mat[r][c] );
    printf("\n");
}