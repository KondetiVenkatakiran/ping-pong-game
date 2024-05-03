#include <stdio.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <pthread.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>

int sX = 11, sY = 40, score = 0, end = 0, size = 100, flag = 0, s = 0, o_score = 0, chances = 3, stop_sig = 0;
char key, XD, YD, me[20], name[20];
int s_fd, a_fd, aX, aY, X, Y;

void score_board(int f);
void data_transfer_s(int a, int b);
void data_transfer_r();
void build_connection();
char kbhit();
void print_border(); // Function prototype declaration
int surf_board_contact_check(int o, int p, int s);
void *ping_pong();
void *paddle();

void main()
{
    build_connection();
    printf("Please share your name with the client (NO SPACES, USE UNDERSCORE):\n");
    scanf("%s", me);
    if (send(a_fd, me, 20, 0) < 0)
    {
        perror("Error in name sending:");
        exit(1);
    }

    if (recv(a_fd, name, 20, 0) < 0)
    {
        perror("Error in name receiving:");
        exit(1);
    }

    initscr();
    clear();
    pthread_t t1, t2, t3;
    pthread_create(&t1, NULL, ping_pong, NULL);
    pthread_create(&t2, NULL, paddle, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    endwin();
}

void print_border()
{
    move(9, 41);
    addstr("----------------------------------------------------");
    for (int i = 9; i <= 38; i += 2)
    {
        move(i, 93);
        addstr(".");
    }
    move(38, 41);
    addstr("----------------------------------------------------");
    refresh();
}

void build_connection()
{

    int b_fd, l_fd, len;
    int port = 8080;
    char *ip = "127.0.0.1";

    struct sockaddr_in s_addr, c_addr;

    bzero(&s_addr, sizeof(s_addr));
    if ((s_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error at socket creation");
        exit(1);
    }
    printf("[+] Socket creation done\n");

    s_addr.sin_family = AF_INET;
    s_addr.sin_port = port;
    s_addr.sin_addr.s_addr = atoi(ip);

    if ((b_fd = bind(s_fd, (struct sockaddr *)&s_addr, sizeof(s_addr))) < 0)
    {
        perror("Error at binding");
        exit(1);
    }
    printf("[+] Binded successfully\n");

    listen(s_fd, 5);
    len = sizeof(c_addr);
    if ((a_fd = accept(s_fd, (struct sockaddr *)&c_addr, &len)) < 0)
    {
        perror("Error at accepting client");
        exit(1);
    }
    printf("[+] Server accepted the client\n");

    printf("Please start the game\n");
}

char kbhit()
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF)
    {
        return ch;
    }
    return '*';
}

int surf_board_contact_check(int o, int p, int s)
{
    if (sY + 1 == p && sX <= o && sX + 6 >= o)
    {
        score = s + 1;
        return 1;
    }
    else if (sY >= p && stop_sig == 0)
    {
        stop_sig = 1;
        chances -= 1;
        move(o, p);
        addstr(" ");
        score_board(0);
        refresh();
        if (chances == 0)
        {
            data_transfer_s(1, 1);
            o_score += 1;
            clear();
            score_board(1);
            refresh();
            sleep(2);
            endwin();
            exit(1);
        }
        else
            data_transfer_s(1, 0);
        return -1;
    }
    return 0;
}

void score_board(int f)
{
    char sc[100];
    if (f == 1)
    {
        move(7, 41);
        if (score > o_score)
            sprintf(sc, "%s is winner!!, final score is : %d", me, score);
        else if (score < o_score)
            sprintf(sc, "%s is winner!!, final score is : %d", name, o_score);
        else if (score == o_score)
            sprintf(sc, "It's a TIE!!, final score is : %d", score);
        addstr(sc);
    }
    else
    {
        move(7, 41);
        sprintf(sc, "My score: %d, %s's score: %d     Chances: %d", score, name, o_score, chances);
        addstr(sc);
    }
}

void *ping_pong()
{
    print_border();
    int LE = 40, RE = 92, TP = 9, BT = 38, f = 1, f1 = 1, speed = (rand() % 1000000) + 100000;
    X = 24;
    Y = 60;
    data_transfer_s(0, 0); // Send initial data to start the game
    while (key != 'q')
    {
        data_transfer_r(); // Receive opponent's data
        if (flag == 1)
        {
            X = (rand() % 29) + 9;
            Y = 69;
            XD = 'L';
            YD = 'D';
            aX = +1;
            aY = -1;
            score += 1;
            flag = 0;
        }
        score_board(0);
        refresh();
        move(X += aX, Y += aY);
        addstr("o");
        refresh();
        usleep(speed);
        move(X, Y);
        addstr(" ");
        if (f == 1 || X == TP + 1)
        {
            if (Y == LE + 1 || (XD == 'R' && YD == 'U') || f1 == 0)
            {
                aX = +1;
                aY = +1;
                XD = 'R';
                YD = 'D';
            }
            else if (Y == RE - 1 || (XD == 'L' && YD == 'U') || f1 == 1)
            {
                aY = -1;
                aX = +1;
                XD = 'L';
                YD = 'D';
            }
            f = 0;
            f1 = -1;
        }

        else if ((f == 2 || X == BT - 1))
        {
            if (Y == LE + 1 || (XD == 'R' && YD == 'D') || f1 == 0)
            {
                aY = +1;
                aX = -1;
                XD = 'R';
                YD = 'U';
            }
            else if (Y == RE - 1 || (XD == 'L' && YD == 'D') || f1 == 1)
            {
                aX = -1;
                aY = -1;
                XD = 'L';
                YD = 'U';
            }
            f = 0;
            f1 = -1;
        }

        else if (Y == RE - 1)
        {
            data_transfer_s(0, 0);
            X = 24; // Reset ball position after a goal
            Y = 60;
            f = 1;
        }

        else if (surf_board_contact_check(X, Y, score) == 1)
        {
            if (X == sX - 1 || (XD == 'L' && YD == 'U') || f1 == 0)
            {
                aY = +1;
                aX = -1;
                XD = 'R';
                YD = 'U';
            }
            else if (X == TP + 1 || (XD == 'L' && YD == 'D') || f1 == 1)
            {
                aY = +1;
                aX = +1;
                XD = 'R';
                YD = 'D';
            }
            f = 0;
            f1 = -1;
        }

        else if (chances == 0)
        {
            o_score += 1;
            clear();
            score_board(1);
            refresh();
            sleep(2);
            endwin();
            exit(1);
            break;
        }
        if (stop_sig == 1)
            break;
    }
    return NULL;
}

void *paddle()
{
    char c, b2[20] = "#", clr[20] = " ";
    while ((key = kbhit()) != 'q')
    {
        if (key == 'w')
        {
            move(sX + 6, sY);
            addstr(clr);
            sX -= 1;
            move(sX, sY);
            addstr(b2);
            refresh();
        }
        if (key == 's')
        {
            move(sX, sY);
            addstr(clr);
            refresh();
            sX += 1;
            move(sX + 6, sY);
            addstr(b2);
            refresh();
        }
    }
    return NULL;
}

void data_transfer_s(int a, int b)
{
    int size = 100, c[5];
    move(X, Y);
    addstr(" ");
    refresh();
    c[0] = X;
    c[1] = -1;
    if (YD == 'D')
        c[1] = +1;
    c[2] = score;
    c[3] = a;
    c[4] = b;
    send(a_fd, c, sizeof(c), 0);

    aX = 0;
    aY = 0;
    flag = 0;
}

void data_transfer_r()
{
    char buff[size];
    int co[5];

    bzero(co, sizeof(co));
    if (recv(a_fd, co, sizeof(co), 0) >= 1)
    {
        if (co[4] == 1)
        {
            clear();
            score += 1;
            score_board(1);
            refresh();
            sleep(2);
            endwin();
            exit(1);
        }
        X = co[0];
        Y = 89;
        XD = 'L';
        YD = 'U';
        aY = -1;
        aX = co[1];
        if (aX == +1)
            YD = 'D';
        o_score = co[2];
        flag = co[3];
        bzero(co, sizeof(co));
        stop_sig = 0;
    }
}
