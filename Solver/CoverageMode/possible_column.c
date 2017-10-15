/*
    523066680 2017-10
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

struct possible
{
    int n;
    int cols[9];
};

void str_to_mat( char *s, int mat[9][9] );
void print_mat( int mat[9][9] );

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

void enumerate(int mat[9][9], int target, int row )
{
    int cols[9];
    get_possible_col( mat, row, target, cols );

    if (row >= 9 ) 
    {
        return;
    }

    printf(" %d   ", row);
    for (int c = 0; c < 9; c++ )
        if ( cols[c] != 0 )
            printf("%d ", c);
    
    printf("\n");
    enumerate( mat, target, row+1 );
}

int main(int argc, char *argv[] ) 
{
    char game[] = "030000001200806000000005000000000000000000650043070000600002080090000000000010003";
    int mat[9][9];
    str_to_mat( game, mat );
    print_mat(mat);

    printf("Number: %d\n", 1);
    printf("Row  Possible Column\n");
    enumerate(mat, 1, 0);
    return 0;
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