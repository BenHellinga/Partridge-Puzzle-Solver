#include <stdio.h>
#include <string.h>
#include <time.h>



// defines



#define NUM_CUBES 10
#define BOARD_SIZE (NUM_CUBES * (NUM_CUBES + 1) / 2)



// structs



typedef struct Node Node;
struct Node
{
    Node *r;
    Node *l;
    int x;
    int y;
    int w;
};



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
int invalidPit(int width, int height);



// main



int main()
{
    // init pieces
    for (int i = 0; i < NUM_CUBES + 1; ++i)
        pieces[i] = i;

    // init board
    Node rightWall = { NULL, NULL, BOARD_SIZE, BOARD_SIZE + 1, 0 };
    Node floor = { NULL, NULL, 0, 0, BOARD_SIZE };
    Node leftWall = { NULL, NULL, 0, BOARD_SIZE + 1, 0 };

    leftWall.r = &floor;
    floor.r = &rightWall;
    rightWall.l = &floor;
    floor.l = &leftWall;

    head = &leftWall;

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    solve_r(0);

    clock_gettime(CLOCK_MONOTONIC, &end);

    double time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
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

    int maxSize = node->w < BOARD_SIZE - node->y ? node->w : BOARD_SIZE - node->y;
    maxSize = maxSize < NUM_CUBES ? maxSize : NUM_CUBES;

    int fullWidth = maxSize == node->w;
    if (fullWidth) --maxSize;

    for (int size = 1; size <= maxSize; ++size)
    {
        if (pieces[size] == 0) continue;

        int newY = node->y + size;

        // check left pit
        if (left->y < newY && left->y < left->l->y)
        {
            int height = newY < left->l->y ? newY - left->y : left->l->y - left->y;
            if (invalidPit(left->w, height))
                return 0;
        }

        // check right pit (right is the new node we're going to make)
        if (node->y < right->y)
        {
            int height = newY < right->y ? newY - node->y : right->y - node->y;
            if (invalidPit(node->w - size, height))
                continue;
        }

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

        // check left pit
        if (left->y < newY && left->y < left->l->y)
        {
            int height = newY < left->l->y ? newY - left->y : left->l->y - left->y;
            if (invalidPit(left->w, height))
                return 0;
        }

        // check right pit
        if (right->y < newY && right->y < right->r->y)
        {
            int height = newY < right->r->y ? newY - right->y : right->r->y - right->y;
            if (invalidPit(right->w, height))
                return 0;
        }

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



Node *selectNode()
{
    Node *current = head->r;
    Node *selected = head->r;

    int min = BOARD_SIZE;

    while (current->r != NULL)
    {    
        // // find lowest y
        // if (current->y < selected->y)
        //     selected = current;

        if (current->y < current->r->y && current->y < current->l->y && current->w < min)
        {
            min = current->w;
            selected = current;
        }

        // if (current->y < current->r->y) return current;

        current = current->r;
    }

    return selected;
}



int invalidPit(int width, int height)
{
    return 0;

    int n1;
    int n2;
    int n3;
    int n4;
    int n5;

    switch(width)
    {
        case 1:
            n1 = pieces[1];
            if (height > n1) return 1;
            break;

        case 2:
            n2 = pieces[2];
            if (height > 2 * n2) return 1;
            break;

        case 3:
            n1 = pieces[1];
            n2 = pieces[2];
            n3 = pieces[3];

            switch (height)
            {
                case 1:
                    if (n3 == 0 && (n1 == 0 || n2 == 0)) return 1;
                    break;

                default:
                    if (height > 3 * n3) return 1;
                    break;
            }
            break;

        case 4:
            n1 = pieces[1];
            n2 = pieces[2];
            n3 = pieces[3];
            n4 = pieces[4];

            switch (height)
            {
                case 1:
                    if (n4 == 0 && n2 < 2 && (n1 == 0 || n3 == 0)) return 1;
                    break;

                case 2:
                    if (n4 == 0 && n2 < 2) return 1;
                    break; 

                default:
                    if (height > 4 * n4) return 1;
                    break;
            }
            break;

        case 5:
            n1 = pieces[1];
            n2 = pieces[2];
            n3 = pieces[3];
            n4 = pieces[4];
            n5 = pieces[5];

            switch (height)
            {
                case 1:
                    if (n5 == 0 && (n4 == 0 || n1 == 0) && (n3 == 0 || n2 == 0) && (n2 < 2 || n1 == 0)) return 1;
                    break;

                case 2:
                    if (n5 == 0 && (n3 == 0 || n2 == 0)) return 1;
                    break;

                case 3:
                    if (n5 == 0 && (n3 == 0 || n2 < 2)) return 1;
                    break;

                case 4:
                    if (n5 == 0 && (n3 < 2 || n2 < 2)) return 1;
                    break;

                default:
                    if (height > 5 * n4) return 1;
                    break;
            }
            break;
    }

    return 0;
}