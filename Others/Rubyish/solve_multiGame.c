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

int explore (Sdk);
void echo (Sdk);
void play (Sdk);
void init (void);
int best ();

int Verti[9];         //列-掩码
int Horiz[9];         //行-掩码
int Bloke[9];         //区块-掩码
int Hover[9][9];
int Maybe[1024][10];  //掩码-可能数 映射表
Ijk Dit[81];          //空单元，unsolved列表
int Head;
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
    static int r, c;
    static int idx;
    int mask;
    int fill = 0;
    Ijk *w;

    int possible[10];
    for (idx = Head; idx < Tail; idx++)
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
    for (r = 0; r < 9; r++)
    {
        memset(possible, 0, sizeof(possible) );
        for (c = 0; c < 9; c++)
        {
            if ( sudo[r][c] == 0 ) 
            {
                mask = Horiz[r] | Verti[c] | Bloke[Hover[r][c]];
                for ( int e = 1; e <= Maybe[mask][0] ; e++)
                {
                    possible[ Maybe[mask][e] ]++;
                }
            }
        }

        for (c = 0; c < 9; c++)
        {
            if ( sudo[r][c] == 0 ) 
            {
                mask = Horiz[r] | Verti[c] | Bloke[Hover[r][c]];
                for ( int e = 1; e <= Maybe[mask][0] ; e++)
                {
                    if ( possible[ Maybe[mask][e] ] == 1 )
                    {
                        sudo[r][c] = Maybe[mask][e];
                        //printf("R Fill %d,%d  %d\n", r, c, sudo[r][c]);
                        Horiz[r] |= 1 << sudo[r][c];
                        Verti[c] |= 1 << sudo[r][c];
                        Bloke[Hover[r][c]] |= 1 << sudo[r][c];
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
            if ( sudo[r][c] == 0 ) 
            {
                mask = Horiz[r] | Verti[c] | Bloke[Hover[r][c]];
                for ( int e = 1; e <= Maybe[mask][0] ; e++)
                {
                    possible[ Maybe[mask][e] ]++;
                }
            }
        }

        for (r = 0; r < 9; r++)
        {
            if ( sudo[r][c] == 0 ) 
            {
                mask = Horiz[r] | Verti[c] | Bloke[Hover[r][c]];
                for ( int e = 1; e <= Maybe[mask][0] ; e++)
                {
                    if ( possible[ Maybe[mask][e] ] == 1 )
                    {
                        sudo[r][c] = Maybe[mask][e];
                        //printf("C Fill %d,%d  %d\n", r, c, sudo[r][c]);
                        Horiz[r] |= 1 << sudo[r][c];
                        Verti[c] |= 1 << sudo[r][c];
                        Bloke[Hover[r][c]] |= 1 << sudo[r][c];
                        fill++;
                    }
                }
            }
        }
    }

    //block
    static int blk, in;

    for (blk = 0; blk < 9; blk++)
    {
        memset(possible, 0, sizeof(possible) );
        for (in = 0; in < 9; in++)
        {
            r = block_ele[blk][in].r;
            c = block_ele[blk][in].c;
            if ( sudo[r][c] == 0 ) 
            {
                mask = Horiz[r] | Verti[c] | Bloke[Hover[r][c]];
                for ( int e = 1; e <= Maybe[mask][0] ; e++)
                {
                    possible[ Maybe[mask][e] ]++;
                }
            }
        }

        for (in = 0; in < 9; in++)
        {
            r = block_ele[blk][in].r;
            c = block_ele[blk][in].c;
            if ( sudo[r][c] == 0 ) 
            {
                mask = Horiz[r] | Verti[c] | Bloke[Hover[r][c]];
                for ( int e = 1; e <= Maybe[mask][0] ; e++)
                {
                    if ( possible[ Maybe[mask][e] ] == 1 )
                    {
                        sudo[r][c] = Maybe[mask][e];
                        //fprintf(stderr, "BLK Fill %d,%d  %d\n", r, c, sudo[r][c]);
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
    static int i, j;
    Head = 0;
    for (i = 0; i < 9; i++) 
        for (j = 0; j < 9; j++) 
            if ( !sudo[i][j] )
                Dit[Head++] = (Ijk) {j, i, Hover[i][j] };

    Tail = Head;      //空单元数量
    Lost = Tail + 1;  //越界
    Head = 0;         //起点
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

    explore (sudo);
} /* play */

int explore (Sdk sudo)
{
    int res;
    if (Head == Lost) return 0;
    int this = best ();
    if (this == ERROR) return 0;
    if (this == OK) { print_sudo_inline(sudo); return 1; }

    int *maybe = &Maybe[this][OK];
    Ijk *w     = &Dit[Head - 1];

    res = 0;
    for (int it = 1; it <= maybe[OK]; it++)
    {
        int n = 1 << maybe[it];
        sudo[w->h][w->v] = maybe[it];
        Horiz[w->h] |= n;
        Verti[w->v] |= n;
        Bloke[w->b] |= n;
        res = explore (sudo);
        if (res == 1) return 1;
        Horiz[w->h] ^= n;
        Verti[w->v] ^= n;
        Bloke[w->b] ^= n;
    }

    sudo[w->h][w->v] = 0;
    Head--;
    return 0;      //遍历所有可能数也没有结果？返回0
} /* explore */

int best ()
{
    register int min, best, posisi;
    register int head, this, maybe;
    static Ijk *w;
    min    = 10;
    best   = OK;
    posisi = Head;

    for (head = Head; head < Tail; head++)
    {
        w    = &Dit[head];
        this = Verti[w->v] | Horiz[w->h] | Bloke[w->b];
        if (this == ERROR) return ERROR;
        maybe = Maybe[this][OK];

        if (min > maybe) {
            posisi = head, best = this;
            if (maybe == BEST) break;
            min = maybe;
        }
    }

    Ijk dat = Dit[posisi];
    Dit[posisi] = Dit[Head];
    Dit[Head++] = dat;
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