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

typedef char kar;
typedef int Sdk[9][9];
typedef struct { int v, h, b; } Ijk;

struct game {
    int id;
    char s[100];
    struct game *next;
};

void str_to_mat( char *s, Sdk sudo );
void load_games( struct game * games, char *filename  );
void explore (Sdk);
void echo (Sdk);
void play (Sdk);
void init (void);
int best ();

int Verti[9];
int Horiz[9];
int Bloke[9];
int Hover[9][9];
kar Maybe[1024][10];
Ijk Dit[81];
int Head;
int Tail;
int Lost;

int main (int argc, char *argv[])
{
    int time_a;
    Sdk sudo;
    struct game *gamenode = (struct game *)malloc( sizeof(struct game) );
    load_games( gamenode, "../../Puzzles/sudoku_nd0.txt" );

    //备用数据初始化
    init ();

    while ( gamenode->next != NULL )
    {
        // time_a = clock();
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
    for (int i = 0; i < 9; i++) 
    {
        int i3 = 3 * (i / 3);
        for (int j = 0; j < 9; j++)
            Hover[i][j] = j / 3 + i3;
    }

    for (int n = 0; n < ERROR; n += 2) 
    {
        int len = 0;
        for (int i = 1; i <= 9; i++) 
        {
            if (n & 1 << i) continue;
            Maybe[n][1 + len++] = i;
        }
        Maybe[n][0] = len;
    }
}

void play (Sdk sudo)
{
    static int i, j, k;
    for (i = 0; i < 9; i++)
        Horiz[i] = Verti[i] = Bloke[i] = 0;

    Head = 0;
    for (i = 0; i < 9; i++) 
    {
        for (j = 0; j < 9; j++) 
        {
            k = Hover[i][j];
            if (!sudo[i][j]) 
            {
                Dit[Head++] = (Ijk) {j, i, k };
                continue;
            }

            Horiz[i] |= 1 << sudo[i][j];
            Verti[j] |= 1 << sudo[i][j];
            Bloke[k] |= 1 << sudo[i][j];
        }
    }

    Tail = Head;
    Lost = Tail + 1;
    Head = 0;

    explore (sudo);
    puts (" ___________________");
} /* play */

void explore (Sdk sudo) 
{
    if (Head == Lost) return;
    int this = best ();
    if (this == ERROR) return;
    if (this == OK) echo (sudo);

    kar *maybe = &Maybe[this][OK];
    Ijk *w     = &Dit[Head - 1];

    for (int it = 1; it <= maybe[OK]; it++) 
    {
        int n = 1 << maybe[it];
        sudo[w->h][w->v] = maybe[it];
        Horiz[w->h] |= n;
        Verti[w->v] |= n;
        Bloke[w->b] |= n;
        explore (sudo);
        Horiz[w->h] ^= n;
        Verti[w->v] ^= n;
        Bloke[w->b] ^= n;
    }
    sudo[w->h][w->v] = 0;
    Head--;
} /* explore */

int best ()
{
    int min    = 10;
    int best   = OK;
    int posisi = Head;

    for (int head = Head; head < Tail; head++) 
    {
        Ijk *w   = &Dit[head];
        int this = Verti[w->v] | Horiz[w->h] | Bloke[w->b];
        if (this == ERROR) return ERROR;
        int maybe = Maybe[this][OK];

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
    for (int i = 0; i < 9; i++) 
    {
        if (!(i % 3)) puts ("");
        for (int j = 0; j < 9; j++) 
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