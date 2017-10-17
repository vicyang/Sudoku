/*
    作者：Rubyish
    日期: 2017-10
    http://bbs.chinaunix.net/forum.php?mod=viewthread&tid=4267389&page=3&authorid=25822983
    
    修改：vicyang
    添加的函数：fill_one_possible_num
*/

# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <time.h>

# define ERROR 0b1111111110
# define OK    0
# define BEST  1

typedef int kar;
typedef int Sdk[9][9];
typedef struct { int v, h, b; } Ijk;

struct game {
    int id;
    char s[100];
    struct game *next;
};

struct position {
    int r, c;
};

struct position block_ele[9][9];

void str_to_mat( char *s, Sdk sudo );
void load_games( struct game * games, char *filename  );
int fill_one_possible_number ();
void print_sudo_inline( Sdk sudo );

int explore (Sdk, int);
void echo (Sdk);
void play (Sdk);
void init (void);
int best (int);

int Verti[9];         //列-掩码
int Horiz[9];         //行-掩码
int Bloke[9];         //区块-掩码
int Hover[9][9];
int Maybe[1024][10];  //掩码-可能数 映射表
Ijk Dit[81];          //空单元，unsolved列表
int Tail;
int Lost;

int main (int argc, char *argv[])
{
    int time_a;
    Sdk sudo;
    struct game *gamenode = (struct game *)malloc( sizeof(struct game) );
    load_games( gamenode, "../../Puzzles/sudoku17.txt" );

    //备用数据初始化
    init ();

    while ( gamenode->next != NULL )
    {
        //time_a = clock();
        str_to_mat( gamenode->s, sudo );
        play (sudo);

        // fprintf(stderr, "Game ID: %d, Time used: %.3f\n", gamenode->id, 
        //     (float)(clock()-time_a)/(float)CLOCKS_PER_SEC );
        gamenode = gamenode->next;
        //break;
    }

    fprintf(stderr, "Time used: %.3f\n", (float)clock() / (float)CLOCKS_PER_SEC );
    return 0;
} /* main */

void init ()
{
    static int m, n, i, len;
    for (m = 0; m < 9; m++)
    {
        for (n = 0; n < 9; n++)
        {
            //m:row, n:col
            Hover[m][n] = (int)(m/3)*3 + (int)(n/3);
            //m:block_id, n:block_element_id
            block_ele[m][n].r = (int)(m/3)*3 + (int)(n/3); 
            block_ele[m][n].c = m % 3 * 3 + n % 3;
        }
    }

    for (m = 0; m < 0b1111111110; m += 2) 
    {
        len = 0;
        for (i = 1; i <= 9; i++)
        {
            if ( m & (1 << i) ) continue;
            Maybe[m][1 + len++] = i;
        }
        Maybe[m][0] = len;
    }
}

int fill_one_possible_number (Sdk sudo)
{
    static int idx;
    static int mask;
    int fill = 0;
    Ijk *w;

    for (idx = 0; idx < Tail; idx++)
    {
        w = &Dit[idx];
        //mask
        mask = Horiz[w->h] | Verti[w->v] | Bloke[w->b];
        if ( Maybe[mask][0] == 1 )
        {
            sudo[w->h][w->v] = Maybe[mask][1];
            Horiz[w->h] |= 1 << sudo[w->h][w->v];
            Verti[w->v] |= 1 << sudo[w->h][w->v];
            Bloke[w->b] |= 1 << sudo[w->h][w->v];
            //printf("Fill %d,%d  %d\n", w->h, w->v,  sudo[w->h][w->v]);
            fill++;
        }
    }

    //ROW
    static int m, n, e, r, c;
    static int Rmask, Cmask, Bmask;
    static int Rpossible[10];
    static int Cpossible[10];
    static int Bpossible[10];

    for (m = 0; m < 9; m++)
    {
        memset(Rpossible, 0, sizeof(Rpossible) );
        memset(Cpossible, 0, sizeof(Cpossible) );
        memset(Bpossible, 0, sizeof(Bpossible) );

        for (n = 0; n < 9; n++)
        {
            r = block_ele[m][n].r;
            c = block_ele[m][n].c;

            if ( sudo[m][n] == 0 ) 
            {
                Rmask = Horiz[m] | Verti[n] | Bloke[Hover[m][n]];
                for ( e = 1; e <= Maybe[Rmask][0] ; e++)
                    Rpossible[ Maybe[Rmask][e] ]++;
            }
            if ( sudo[n][m] == 0 ) 
            {
                Cmask = Horiz[n] | Verti[m] | Bloke[Hover[n][m]];
                for ( e = 1; e <= Maybe[Cmask][0] ; e++)
                    Cpossible[ Maybe[Cmask][e] ]++;
            }
            if ( sudo[r][c] == 0 ) 
            {
                Bmask = Horiz[r] | Verti[c] | Bloke[Hover[r][c]];
                for ( e = 1; e <= Maybe[Bmask][0] ; e++)
                {
                    Bpossible[ Maybe[Bmask][e] ]++;
                }
            }
        }

        for (n = 0; n < 9; n++)
        {
            //Rows
            if ( sudo[m][n] == 0 ) 
            {
                Rmask = Horiz[m] | Verti[n] | Bloke[Hover[m][n]];
                for ( e = 1; e <= Maybe[Rmask][0] ; e++)
                {
                    if ( Rpossible[ Maybe[Rmask][e] ] == 1 )
                    {
                        sudo[m][n] = Maybe[Rmask][e];
                        //printf("R Fill %d,%d  %d\n", m, n, sudo[m][n]);
                        Horiz[m] |= 1 << sudo[m][n];
                        Verti[n] |= 1 << sudo[m][n];
                        Bloke[Hover[m][n]] |= 1 << sudo[m][n];
                        fill++;
                    }
                }
            }

            //Cols
            if ( sudo[n][m] == 0 )
            {
                Cmask = Horiz[n] | Verti[m] | Bloke[Hover[n][m]];
                for ( e = 1; e <= Maybe[Cmask][0] ; e++)
                {
                    if ( Cpossible[ Maybe[Cmask][e] ] == 1 )
                    {
                        sudo[n][m] = Maybe[Cmask][e];
                        //printf("C Fill %d,%d  %d\n", m, n, sudo[n][m]);
                        Horiz[n] |= 1 << sudo[n][m];
                        Verti[m] |= 1 << sudo[n][m];
                        Bloke[Hover[n][m]] |= 1 << sudo[n][m];
                        fill++;
                    }
                }
            }

            //Blocks
            r = block_ele[m][n].r;
            c = block_ele[m][n].c;
            if ( sudo[r][c] == 0 ) 
            {
                Bmask = Horiz[r] | Verti[c] | Bloke[Hover[r][c]];
                for ( e = 1; e <= Maybe[Bmask][0] ; e++)
                {
                    if ( Bpossible[ Maybe[Bmask][e] ] == 1 )
                    {
                        sudo[r][c] = Maybe[Bmask][e];
                        //printf("B Fill %d,%d  %d\n", r, c, sudo[r][c]);
                        Horiz[r] |= 1 << sudo[r][c];
                        Verti[c] |= 1 << sudo[r][c];
                        Bloke[Hover[r][c]] |= 1 << sudo[r][c];
                        fill++;
                    }
                }
            }
        }
    }

    return fill;
}

void update_Dit (Sdk sudo)
{
    static int i, j, index;
    index = 0;
    for (i = 0; i < 9; i++) 
        for (j = 0; j < 9; j++) 
            if ( !sudo[i][j] )
                Dit[index++] = (Ijk) { j, i, Hover[i][j] };

    Tail = index;     //空单元数量
    Lost = Tail + 1;  //越界
}

void play (Sdk sudo)
{
    static int i, j, k;
    for (i = 0; i < 9; i++)
        Horiz[i] = Verti[i] = Bloke[i] = 0;

    for (i = 0; i < 9; i++)
    {
        for (j = 0; j < 9; j++)
        {
            k = Hover[i][j];
            if ( !sudo[i][j] ) continue;
            Horiz[i] |= 1 << sudo[i][j];
            Verti[j] |= 1 << sudo[i][j];
            Bloke[k] |= 1 << sudo[i][j];
        }
    }

    update_Dit(sudo);
    while (fill_one_possible_number(sudo) > 0) { update_Dit(sudo); }

    explore (sudo, 0);
} /* play */

int explore (Sdk sudo, int lv)
{
    int res;
    int mask = best (lv);
    if (mask == ERROR) return 0;
    if (mask == OK) { print_sudo_inline(sudo); return 1; }

    int *possible = Maybe[mask];
    Ijk *w     = &Dit[lv];

    res = 0;
    int it, n;
    for (it = 1; it <= possible[OK]; it++)
    {
        n = 1 << possible[it];
        sudo[w->h][w->v] = possible[it];
        Horiz[w->h] |= n;
        Verti[w->v] |= n;
        Bloke[w->b] |= n;
        res = explore (sudo, lv+1);
        if (res == 1) return 1;
        Horiz[w->h] ^= n;
        Verti[w->v] ^= n;
        Bloke[w->b] ^= n;
    }

    sudo[w->h][w->v] = 0;
    return 0;      //遍历所有可能数也没有结果？返回0
} /* explore */

int best (int begin)
{
    register int min, best, select;
    register int head, this, count;
    static Ijk *w;
    min    = 10;
    best   = 0;
    select = begin;

    for (head = begin; head < Tail; head++)
    {
        w    = &Dit[head];
        this = Verti[w->v] | Horiz[w->h] | Bloke[w->b];
        if (this == ERROR) return ERROR;
        count = Maybe[this][0];

        if (min > count) {
            select = head, best = this;
            if (count == BEST) break;
            min = count;
        }
    }

    Ijk dat = Dit[select];
    Dit[select] = Dit[begin];
    Dit[begin] = dat;
    return best;
} /* best */


void echo (Sdk sudo)
{
    static int i, j;
    for (i = 0; i < 9; i++) 
    {
        if (!(i % 3)) puts ("");
        for (j = 0; j < 9; j++) 
        {
            if (!(j % 3)) printf (" ");
            printf ("%d ", sudo[i][j]);
        }
        puts ("");
    }
}

void load_games( struct game * node, char *filename  )
{
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

void str_to_mat( char *s, Sdk sudo )
{
    static int i, r, c;
    static int delta = '0' - 0;
    for (i = 0; i < 81; i++ )  //r,c from 0,0 to 8,8
    {
        r = (int)(i/9);
        c = (int)(i%9);
        sudo[r][c] = s[i] - delta;  //char to integer
    }
}

void print_sudo_inline( Sdk sudo )
{
    static int r, c;
    for (r = 0; r < 9; r++ )
        for (c = 0; c < 9; c++ )
            printf("%d", sudo[r][c] );

    printf("\n");
}