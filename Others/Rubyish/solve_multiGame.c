/*
    作者：Rubyish
    日期: 2017-10
    http://bbs.chinaunix.net/forum.php?mod=viewthread&tid=4267389&page=3&authorid=25822983
    
    修改：Rubyish
    添加的函数：
    void fill(Sdk sudo)
    int cmp(...)
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

// 添加的代码 Begin //

typedef int (*Fill)[10];

int cmp (const void *a, const void *b){
    return *(int *)a - *(int *)b;
}

# define CHERRY { A, A, A, A, A, A, A, A, A }
# define A      { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, }

void fill (Sdk sudo);

// 添加的代码 End //

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

void fill (Sdk sudo)
{
    static int h, i, j, n, val, this, go;
    int I[][10] = CHERRY;
    int J[][10] = CHERRY;
    int K[][10] = CHERRY;

    for (h = Head; h < Tail; h++) {
        Ijk *w   = &Dit[h];
        this = Verti[w->v] | Horiz[w->h] | Bloke[w->b];
        kar *may = Maybe[this];
        for (i = 1; i <= may[OK]; i++) {
            val = may[i];
            I[w->h][val] = I[w->h][val] == -1 ? h : -2;
            J[w->v][val] = J[w->v][val] == -1 ? h : -2;
            K[w->b][val] = K[w->b][val] == -1 ? h : -2;
        }
    }

    Fill ijk[] = { I, J, K };
    int posi[Tail - Head];
    go = 0;

    for (i = 0; i < 3; i++) {
        for (j = 0; j < 9; j++) {
            for (n = 1; n <= 9; n++) {
                this = ijk[i][j][n];
                if (this < 0) continue;
                Ijk *x = &Dit[this];
                if (sudo[x->h][x->v]) continue;
                sudo[x->h][x->v] = n;
                Horiz[x->h]     |= 1 << n;
                Verti[x->v]     |= 1 << n;
                Bloke[x->b]     |= 1 << n;
                posi[go++]       = this;
            }
        }
    }

    if (!go) return;

    // SORT INDEX, IMPORTANT!!
    qsort (posi, go, sizeof(int), cmp);
    for (int i = 0; i < go; i++)
        Dit[posi[i]] = Dit[Head++];

    fill (sudo);

} /* fill */

void init (){
    for (int i = 0; i < 9; i++) {
        int i3 = 3 * (i / 3);
        for (int j = 0; j < 9; j++)
            Hover[i][j] = j / 3 + i3;
    }
    for (int n = 0; n < ERROR; n += 2) {
        int len = 0;
        for (int i = 1; i <= 9; i++) {
            if (n & 1 << i) continue;
            Maybe[n][1 + len++] = i;
        }
        Maybe[n][0] = len;
    }
}

void play (Sdk sudo){
    static int i, j, k;
    for (i = 0; i < 9; i++)
        Horiz[i] = Verti[i] = Bloke[i] = 0;

    Head = 0;

    for (i = 0; i < 9; i++) {
        for (j = 0; j < 9; j++) {
            k = Hover[i][j];
            if (!sudo[i][j]) {
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

    fill(sudo);
    explore (sudo);
} /* play */

int explore (Sdk sudo) {
    int res;
    if (Head == Lost) return 0;
    int this = best ();
    if (this == ERROR) return 0;
    if (this == OK) {print_sudo_inline (sudo); return 1;}

    kar *maybe = &Maybe[this][OK];
    Ijk *w     = &Dit[Head - 1];

    int it, n;
    for (it = 1; it <= maybe[OK]; it++) {
        n = 1 << maybe[it];
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
    return 0;
} /* explore */

int best (){
    register int min, best, posisi;
    register int head, this, maybe;
    min    = 10;
    best   = OK;
    posisi = Head;

    for (head = Head; head < Tail; head++) {
        Ijk *w   = &Dit[head];
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
