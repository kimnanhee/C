#pragma warning (disable:4996)
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

int arr[20][20] = { 0 };
int score = 0, cnt = 0;

void print_map()
{
    int i, j;
    printf("score : %d\n\n      ", score);
    for (i = 0; i < 20; i++)
        printf("%2d ", i);
    printf("\n");
    printf("    ----------------------------------------------------------------\n");
    for (i = 0; i < 20; i++)
    {
        printf("%3d | ", i);
        for (j = 0; j < 20; j++)
        {
            if (arr[i][j] == -1) printf("[ ]");
            else printf("[-]");
        }
        printf(" |\n");
    }
    printf("    ----------------------------------------------------------------\n");
}
void random_map(int num)
{
    int rand_x, rand_y, n = 0;
    srand(time(NULL));
    while (n < num)
    {
        rand_x = rand() % 20;
        rand_y = rand() % 20;

        if (arr[rand_x][rand_y] == 0)
        {
            arr[rand_x][rand_y] = 1;
            n++;
        }
    }
}
int main()
{
    system("mode con cols=70 lines=32");
    int boom_num;
    while (1)
    {
        printf("enter mine number : ");
        scanf("%d", &boom_num);
        if (boom_num > 200) continue;
        else break;
    }
    random_map(boom_num);
    int in_x, in_y;
    while (1)
    {
        system("cls");
        print_map();
        printf("\n");
        printf("find (x, y) = ");
        scanf("%d %d", &in_x, &in_y);
        if (0 > in_x || in_x > 20 || 0 > in_y || in_y > 20) printf("over range\n");
        else if (arr[in_x][in_y] == 1) // 1
        {
            printf("finish\n");
            printf("score : %d\n", score);
            _sleep(5000);
            return 0;
        }
        else if (arr[in_x][in_y] == -1)
        {
            printf("Already found it!!\n");
        }
        else
        {
            arr[in_x][in_y] = -1;
            cnt++;
            score += cnt / 10 + 1;
            printf("+ %d\n", cnt / 5 + 1);
            printf("success\n");
        }
        _sleep(1000);
    }
    return 0;
}