#include <stdio.h>
#include <string.h>
#include <time.h>



// defines



#define NUM_CUBES 9
#define BOARD_SIZE (NUM_CUBES * (NUM_CUBES + 1) / 2)



// structs



// node in the skyline linked list, tracks a flat segment of the placed pieces
typedef struct Node Node;
struct Node
{
    Node *r;
    Node *l;
    int x;
    int y;
    int w;
};



// a placed square, position and side length
typedef struct
{
    int x;
    int y;
    int l;
}
Square;



// global



Node *head;
int pieces[NUM_CUBES + 1];
Square squares[BOARD_SIZE];



// functions



void printState();
void printBoard(int num);

int solve_r(int depth);
Node *selectNode();



// main



int main()
{
    // one piece of each size, 1 through num_cubes
    for (int i = 0; i < NUM_CUBES + 1; ++i)
        pieces[i] = i;

    // init board skyline, left wall, floor, right wall as sentinels
    Node rightWall = { NULL, NULL, BOARD_SIZE, BOARD_SIZE + 1, 0 };
    Node floor = { NULL, NULL, 0, 0, BOARD_SIZE };
    Node leftWall = { NULL, NULL, 0, BOARD_SIZE + 1, 0 };

    leftWall.r = &floor;
    floor.r = &rightWall;
    rightWall.l = &floor;
    floor.l = &leftWall;

    head = &leftWall;

    clock_t start = clock();
    solve_r(0);
    clock_t end = clock();

    double time = (end - start) / (double)CLOCKS_PER_SEC;
    printf("Time: %fs\n", time);

    printf("Finished\n");
    return 0;
}



// display functions



void printState()
{
    printf("state: ");

    Node *c = head;
    while (c != NULL)
    {
        printf("(%d %d %d) -> ", c->x, c->y, c->w);
        c = c->r;
    }

    printf("\n");
}



void printBoard(int num)
{
    char image[BOARD_SIZE][BOARD_SIZE];
    memset(image, ' ', BOARD_SIZE * BOARD_SIZE);

    // draw each placed square into the ascii buffer
    for (int i = 0; i < num; ++i)
    {
        int x = squares[i].x;
        int y = squares[i].y;
        int l = squares[i].l - 1;

        image[x + l][y + l] = '*';
        image[x + l][y + 0] = '*';
        image[x + 0][y + l] = '+';
        image[x + 0][y + 0] = '1' + l;

        for (int j = x + 1; j < x + l; ++j) image[j + 0][y + 0] = '-';
        for (int j = x + 1; j < x + l; ++j) image[j + 0][y + l] = '-';
        for (int j = y + 1; j < y + l; ++j) image[x + 0][j + 0] = '|';
        for (int j = y + 1; j < y + l; ++j) image[x + l][j + 0] = '|';
    }

    printf("+-");
    for (int i = 0; i < BOARD_SIZE; ++i) printf("--");
    printf("+ \n");

    for (int y = 0; y < BOARD_SIZE; ++y)
    {
        printf("| ");
        for (int x = 0; x < BOARD_SIZE; ++x)
        {
            char c = image[x][y];

            switch(c)
            {
                case ' ': printf("  "); break;
                case '-': printf("--"); break;
                case '*': printf("+ "); break;
                case '+': printf("+-"); break;
                case '|': printf("| "); break;
                default: printf("%c ", c); break;
            }
        }
        printf("| \n");
    }

    printf("+-");
    for (int i = 0; i < BOARD_SIZE; ++i) printf("--");
    printf("+ \n");
}



// solve functions



int solve_r(int depth)
{
    // all pieces placed
    if (depth == BOARD_SIZE)
    {
        printBoard(depth);
        printf("Solution Found\n");
        return 1;
    }

    Node *node = selectNode();
    Node *right = node->r;
    Node *left = node->l;

    Node **lp;
    Node **rp;
    Node *ol;
    Node *or;

    Node new1;
    Node new2;

    // largest piece that can fit here, capped by section width, board edge, and piece count
    int maxSize = node->w < BOARD_SIZE - node->y ? node->w : BOARD_SIZE - node->y;
    maxSize = maxSize < NUM_CUBES ? maxSize : NUM_CUBES;

    // if the biggest option spans the whole section, handle it separately below
    int fullWidth = maxSize == node->w;
    if (fullWidth) --maxSize;

    for (int size = 1; size <= maxSize; ++size)
    {
        if (pieces[size] == 0) continue;

        int newY = node->y + size;

        // check if left is same y, and merge
        if (newY == left->y)
        {
            new1 = (Node){ &new2, left->l, left->x, newY, left->w + size };
            lp = &left->l->r;
            ol = left->l->r;
        }
        else
        {
            new1 = (Node){ &new2, left, node->x, newY, size };
            lp = &left->r;
            ol = left->r;
        }

        new2 = (Node){ right, &new1, node->x + size, node->y, node->w - size };
        rp = &right->l;
        or = right->l;

        *lp = &new1;
        *rp = &new2;
        --pieces[size];

        squares[depth] = (Square){ node->x, node->y, size };
        int ret = solve_r(depth + 1);

        // backtrack, restore skyline and piece count
        *lp = ol;
        *rp = or;
        ++pieces[size];

        if (ret == 1) return 1;
    }

    // logic is different if the square is the full width of the section
    // this could be in the loop above with if statements, but i felt like putting it down here
    // (its more efficient but a bit of duplicate code)
    if (fullWidth)
    {
        int size = maxSize + 1;

        if (pieces[size] == 0) return 0;

        int newY = node->y + size;

        // merge with left, right, both, or neither, depending on matching heights
        if (newY == left->y && newY == right->y)
        {
            new1 = (Node){ right->r, left->l, left->x, newY, left->w + size + right->w };
            lp = &left->l->r;
            rp = &right->r->l;
            ol = left->l->r;
            or = right->r->l;
        }
        else if (newY == left->y)
        {
            new1 = (Node){ right, left->l, left->x, newY, left->w + size };
            lp = &left->l->r;
            rp = &right->l;
            ol = left->l->r;
            or = right->l;
        }
        else if (newY == right->y)
        {
            new1 = (Node){ right->r, left, node->x, newY, size + right->w };
            lp = &left->r;
            rp = &right->r->l;
            ol = left->r;
            or = right->r->l;
        }
        else
        {
            new1 = (Node){ right, left, node->x, newY, size };
            lp = &left->r;
            rp = &right->l;
            ol = left->r;
            or = right->l;
        }

        *lp = &new1;
        *rp = &new1;
        --pieces[size];

        squares[depth] = (Square){ node->x, node->y, size };
        int ret = solve_r(depth + 1);

        *lp = ol;
        *rp = or;
        ++pieces[size];

        if (ret == 1) return 1;
    }

    return 0;
}



// picks the lowest, narrowest valid corner of the skyline to fill next
Node *selectNode()
{
    Node *current = head->r;
    Node *selected = head->r;

    int min = BOARD_SIZE;

    while (current->r != NULL)
    {
        if (current->y < current->r->y && current->y < current->l->y && current->w < min)
        {
            min = current->w;
            selected = current;
        }

        current = current->r;
    }

    return selected;
}