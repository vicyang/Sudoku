/*
    作者：Rubyish
    日期: 2017-10
    http://bbs.chinaunix.net/forum.php?mod=viewthread&tid=4267389&page=3&authorid=25822983
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
int Head;
int Tail;
int Lost;

int main (int argc, char *argv[])
{
    int time_a;
    Sdk sudo;
    struct game *gamenode = (struct game *)malloc( sizeof(struct game) );
    load_games( gamenode, "./puzzles.txt" );

    //备用数据初始化
    init ();

    while ( gamenode->next != NULL )
    {
        time_a = clock();
        str_to_mat( gamenode->s, sudo );
        play (sudo);

        printf("Game ID: %d, Time used: %.3f\n", gamenode->id, 
            (float)(clock()-time_a)/(float)CLOCKS_PER_SEC );
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
        for (n = 0; n < 9; n++)
            //m:row, n:col <=> block_id
            Hover[m][n] = (int)(m/3)*3 + (int)(n/3);

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
    explore (sudo, 0);
} /* play */

int explore (Sdk sudo, int lv)
{
    int res, mask;
    mask = best( lv );
    if (mask == 0b1111111110) return 0;
    //没有找到空单元，即为终盘
    if (mask == 0) { print_sudo_inline(sudo); return 1; }

    int *possible = Maybe[ mask ];
    Ijk *w     = &Dit[lv];

    res = 0;
    int idx, n;
    for (idx = 1; idx <= possible[0]; idx++)
    {
        n = 1 << possible[idx];
        sudo[w->h][w->v] = possible[idx];
        Horiz[w->h] |= n;
        Verti[w->v] |= n;
        Bloke[w->b] |= n;
        res = explore (sudo, lv+1);
        // if (res == 1) return 1;
        Horiz[w->h] ^= n;
        Verti[w->v] ^= n;
        Bloke[w->b] ^= n;
        sudo[w->h][w->v] = 0;
    }
    return 0;
}

int best ( int begin )
{
    register int min, best, select;
    register int idx, mask, count;
    Ijk *w;
    min    = 10;  //设一个最小量
    select = -1;

    //空单元，索引迭代，从起点到末点
    for (idx = begin; idx < Tail; idx++)
    {
        w    = &Dit[idx];        //w = 对应单元坐标信息
        mask = Verti[w->v] | Horiz[w->h] | Bloke[w->b];
        if (mask == ERROR) return ERROR;
        count = Maybe[mask][0];  //count = 可选数的个数

        if ( count < min ) 
        {
            select = idx, best = mask;
            if (count == 1) break;
            min = count;
        }
    }

    //最优项与范围"池"第一项交换
    if (select != -1)
    {
        Ijk dat = Dit[select];
        Dit[select] = Dit[begin];
        Dit[begin]  = dat;
        return best;
    }
    else
        return 0;
}

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

    if ( fp == NULL )
    {
        printf("File not found!\n");
        exit(0);
    }

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