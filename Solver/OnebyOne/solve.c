/*
    523066680 2017-09
    填充可能性唯一的单元，以及隐式唯一的单元
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct unsolve {
    int r, c, n;
    int possible[9];
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

struct spare sparenums[0b1111111111];

void str_to_mat( char *s, int mat[9][9] );
void print_mat( int mat[9][9] );
void make_refs( int mat[9][9], struct ref refs[9][9] );
void load_games( struct game * games, char *filename  );

int fill_one_possible_num( int mat[9][9], struct unsolve *(unsolved[81]), int n )
{
    static int r, c, idx;
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

    for (idx = 0; idx < n; idx++ )
    {
        cells[unsolved[idx]->r][unsolved[idx]->c] = idx;

        if (unsolved[idx]->n == 1)
        {
            mat[unsolved[idx]->r][unsolved[idx]->c] = unsolved[idx]->possible[0];
            //printf("Fill %d,%d  %d\n", unsolved[idx]->r, unsolved[idx]->c, mat[unsolved[idx]->r][unsolved[idx]->c] );
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
                for ( int e = 0; e < unsolved[idx]->n; e++)
                {
                    possible[ unsolved[idx]->possible[e] ]++;
                }
            }
        }

        for (c = 0; c < 9; c++)
        {
            if ( cells[r][c] != -1 ) 
            {
                idx = cells[r][c];
                for ( int e = 0; e < unsolved[idx]->n; e++)
                {
                    if ( possible[ unsolved[idx]->possible[e] ] == 1 )
                    {
                        mat[r][c] = unsolved[idx]->possible[e];
                        //printf("Fill %d,%d  %d\n", r, c, mat[r][c]);
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
                for ( int e = 0; e < unsolved[idx]->n; e++)
                {
                    possible[ unsolved[idx]->possible[e] ]++;
                }
            }
        }

        for (r = 0; r < 9; r++)
        {
            if ( cells[r][c] != -1 ) 
            {
                idx = cells[r][c];
                for ( int e = 0; e < unsolved[idx]->n; e++)
                {
                    if ( possible[ unsolved[idx]->possible[e] ] == 1 )
                    {
                        mat[r][c] = unsolved[idx]->possible[e];
                        //printf("Fill %d,%d  %d\n", r, c, mat[r][c]);
                        fill++;
                    }
                }
            }
        }
    }

    //block

    int blkr, blkc;
    int rr, cc;

    for (int blk = 0; blk < 9; blk++)
    {
        blkr = (int)(blk/3);
        blkc = blk%3;

        memset(possible, 0, sizeof(possible) );

        for (int in = 0; in < 9; in++)
        {
            rr = blkr*3+ (int)(in/3);
            cc = blkc*3+ in%3;
            if ( cells[rr][cc] != -1 ) 
            {
                idx = cells[rr][cc];
                for ( int e = 0; e < unsolved[idx]->n; e++)
                {
                    possible[ unsolved[idx]->possible[e] ]++;
                }
            }
        }

        for (int in = 0; in < 9; in++)
        {
            rr = blkr*3+ (int)(in/3);
            cc = blkc*3+ in%3;
            if ( cells[rr][cc] != -1 ) 
            {
                idx = cells[rr][cc];
                for ( int e = 0; e < unsolved[idx]->n; e++)
                {
                    if ( possible[ unsolved[idx]->possible[e] ] == 1 )
                    {
                        mat[rr][cc] = unsolved[idx]->possible[e];
                        //printf("Fill %d,%d  %d\n", rr, cc, mat[rr][cc]);
                        fill++;
                    }
                }
            }
        }
    }

    if (fill > 0) res = 1;
    return res;
}

void get_possible_num( struct ref refs[9][9], int row, int col, struct unsolve *unsolved )
{
    static int idx;
    int model = 0;

    for ( idx = 0; idx < 9; idx++ )
    {
        model |= 1 << *refs[row][col].cols[idx];
        model |= 1 << *refs[row][col].rows[idx];
        model |= 1 << *refs[row][col].blks[idx];
    }
    model = model >> 1 << 1;

    for ( idx = 0; idx < sparenums[model].n; idx++  )
    {
        unsolved->possible[idx] = sparenums[model].possible[idx];
    }

    // printf("%d\n\n", sparenums[model].n);
    unsolved->n = sparenums[model].n;
}

int solve( struct ref refs[9][9], struct unsolve *(unsolved[81]), int n, int lv )
{
    int r, c;
    struct unsolve possible;
    int res = 0;
    int count = 0;

    if ( lv >= n )
        return 1;

    r = unsolved[lv]->r;
    c = unsolved[lv]->c;
    get_possible_num( refs, r, c, &possible );

    for ( int i = 0; i < possible.n; i++ )
    {
        *refs[r][c].self = possible.possible[i];
        res = solve(refs, unsolved, n, lv+1);
        if (res == 1) return 1;
    }

    *refs[r][c].self = 0;
    //if (possible.n == 0) res = 0;

    return res;
}

int get_unsolved_list( struct ref refs[9][9], struct unsolve *(list[81]) )
{
    static int r, c, i;
    int idx = 0;
    int min = 0;
    struct unsolve *temp;

    for ( r = 0; r < 9; r++ )
        for ( c = 0; c < 9; c++ ) 
            if ( *refs[r][c].self == 0 )
            {
                list[idx] = (struct unsolve *) malloc( sizeof(struct unsolve) );
                list[idx]->r = r;
                list[idx]->c = c;
                get_possible_num( refs, r, c, list[idx] );

                if ( idx > 0 )
                {
                    //排序
                    for (i = 0 ; i < idx ; i++ )
                    {
                        if ( list[i]->n > list[idx]->n  )
                        {
                            temp = list[i];
                            list[i] = list[idx];
                            list[idx] = temp;
                        }
                    }
                }

                idx++;
            }

    return idx;
}

void build_tables( struct spare *sparenums )
{
    static int i, j, t;

    for (i = 0; i <= 0b1111111110; i+=2 )
    {
        t = 0;
        for ( j = 1; j <= 9; j++)
        {
            if ( (1 << j & i) == 0 )
            {
                sparenums[i].possible[t] = j;
                t++;
            }
        }
        sparenums[i].n = t;
    }
}


int main(int argc, char *argv[] ) 
{
    char game[] = "030000001200806000000005000000000000000000650043070000600002080090000000000010003";
    //char game[] = "503092000700000008006007310020600000065000730007043500000706102070000800400009000";
    int mat[9][9];
    int res;
    int n;
    int time_a;

    struct game *gamenode = (struct game *)malloc( sizeof(struct game) );
    load_games( gamenode, "../../Puzzles/sudoku_nd3.txt" );
    struct unsolve *(unsolved[81]);
    struct unsolve test;

    struct ref refs[9][9];
    make_refs(mat, refs);

    build_tables(sparenums);

    while ( gamenode->next != NULL )
    {
        time_a = clock();
        str_to_mat( gamenode->s, mat );
        n = get_unsolved_list( refs, unsolved );

        while ( fill_one_possible_num( mat, unsolved, n ) ) 
        {
            n = get_unsolved_list( refs, unsolved ); 
        }

        res = solve( refs, unsolved, n, 0 );

        print_mat(mat);
        printf("Index: %d, Res: %d, time used: %.3f\n\n", 
            gamenode->id,
            res, 
            (float)(clock()-time_a)/(float)CLOCKS_PER_SEC 
        );

        gamenode = gamenode->next;
        //break;
    }

    printf("Time used: %.3f\n\n", (float)clock()/(float)CLOCKS_PER_SEC );

    return 0;
}

void make_refs( int mat[9][9], struct ref refs[9][9] )
{
    static int blkr, blkc;
    static int r, c, n;

    for (r = 0; r < 9 ; r++)
        for (c = 0; c < 9 ; c++)
        {
            for ( n = 0; n < 9; n++ )
            {
                blkr = (int)(r/3)*3 + (int)(n/3);
                blkc = (int)(c/3)*3 + n%3;
                refs[r][c].cols[n] = &mat[r][n];
                refs[r][c].rows[n] = &mat[n][c];
                refs[r][c].blks[n] = &mat[blkr][blkc];
            }
            refs[r][c].self = &mat[r][c];
        }
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
    static int r, c, i;
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
    for ( r = 0; r < 9; r++ )
    {
        for ( c = 0; c < 9; c++ ) 
            printf("%d ", mat[r][c] );
        printf("\n");
    }
}