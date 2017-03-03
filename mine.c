#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <ncurses.h>

#define IS_VALID(y, x, game) (0 <= (y) && (y) < game->height && 0 <= (x) && (x) < game->width)
#define GET_TILE(y, x, game) (*(game->tiles + (x) + ((y) * game->width)))

void render();

enum tile_types {FLAG, OPEN, CLOSED};
enum game_status {GAME, LOSS, WIN};

typedef struct {
    int bomb, num;
    enum tile_types type;
} tile_t;


typedef struct {
    const int width, height;
    int x, y;
    enum game_status status;
    tile_t *tiles;
} game_t;


void init(game_t *g)
{
    int i, j, x_offset, y_offset, num;
    srand(time(NULL));
    g->y = g->height / 2;
    g->x = g->width / 2;
    g->status = GAME;
    for (i = 0; i < g->width * g->height; ++i)
    {
        (*(g->tiles + i)).type = CLOSED;
        (*(g->tiles + i)).bomb = ((rand() % 5) == 0) ? true : false;
    }
    for (i = 0; i < g->height; ++i)
        for (j = 0; j < g->width; ++j)
        {
            num = 0;
            for (y_offset = -1; y_offset < 2; y_offset++)
                for (x_offset = -1; x_offset < 2; x_offset++)
                {
                    if (x_offset == 0 && y_offset == 0) continue;
                    if (IS_VALID(i + y_offset, j + x_offset, g)
                            && GET_TILE(i + y_offset, j + x_offset, g).bomb == 1)
                        num++;
                }
            GET_TILE(i, j, g).num = num;
        }
}

void open(int y, int x, game_t *g)
{
    int y_offset, x_offset;
    GET_TILE(y, x, g).type = OPEN;
    if (GET_TILE(y, x, g).num != 0)
        return;
    for (y_offset = -1; y_offset < 2; y_offset++)
        for (x_offset = -1; x_offset < 2; x_offset++)
        {
            if (x_offset == 0 && y_offset == 0) continue;
            if (IS_VALID(y + y_offset, x + x_offset, g)
                    && GET_TILE(y + y_offset, x + x_offset, g).type == CLOSED)
            {
                open(y + y_offset, x + x_offset, g);
            }

        }
}

int check_if_won(game_t *g)
{
    int i, j;
    for (i = 0; i < g->height; ++i)
        for (j = 0; j < g->width; ++j)
            if (GET_TILE(i, j, g).type == CLOSED && GET_TILE(i, j, g).bomb == 0)
                return 0;
    return 1;
}


void input(game_t *g)
{
    switch (getchar())
    {
        case 'j':
            if (IS_VALID(g->y + 1, g->x, g)) g->y++;
            break;
        case 'k':
            if (IS_VALID(g->y - 1, g->x, g)) g->y--;
            break;
        case 'l':
            if (IS_VALID(g->y, g->x + 1, g)) g->x++;
            break;
        case 'h':
            if (IS_VALID(g->y, g->x - 1, g)) g->x--;
            break;
        case ' ':
            GET_TILE(g->y, g->x, g).type = FLAG;
            break;
        case 'o':
            if (GET_TILE(g->y, g->x, g).bomb == 1)
                g->status = LOSS;
            open(g->y, g->x, g);
            if (check_if_won(g))
                g->status = WIN;
            break;
        case 'q':
            fflush(stdout);
            exit(1);
            break;

    }
}

void game_loop(game_t *g)
{
    render();
    input(g);
    while (1)
    {
        input(g);
        render(g);
    }
}

void render(game_t *g)
{
    int x, y, x_offset, y_offset;
    char digit[20];
    getmaxyx(stdscr, y_offset, x_offset);
    y_offset = (y_offset - g->height) / 2;
    x_offset = (x_offset - g->width) / 2;
    switch (g->status) {
        case LOSS:
            mvwprintw(stdscr, y_offset - 2, x_offset, "You lost.");
            for (y = 0; y < g->height; ++y)
                for (x = 0; x < g->width; ++x)
                    if (GET_TILE(y, x, g).bomb == 1)
                            mvwprintw(stdscr, y + y_offset, x + x_offset, "#");
            getch();
            clear();
            init(g);
            game_loop(g);
            break;
        case GAME:
            for (y = 0; y < g->height; ++y)
                for (x = 0; x < g->width; ++x)
                {
                    attron(COLOR_PAIR((y == g->y && x == g->x) ? 1 : 2));
                    switch (GET_TILE(y, x, g).type)
                    {
                        case FLAG:
                            attron(COLOR_PAIR((y == g->y && x == g->x) ? 1 : 3));
                            mvwprintw(stdscr, y + y_offset, x + x_offset, " ");
                            attroff(COLOR_PAIR((y == g->y && x == g->x) ? 1 : 3));
                            break;
                        case OPEN:
                            sprintf(digit, "%d", GET_TILE(y, x, g).num);
                            attron(COLOR_PAIR((y == g->y && x == g->x) ? 1 : 4));
                            mvwprintw(stdscr, y + y_offset, x + x_offset, digit);
                            attroff(COLOR_PAIR((y == g->y && x == g->x) ? 1 : 4));
                            break;
                        case CLOSED:
                            mvwprintw(stdscr, y + y_offset, x + x_offset, " ");
                            break;
                    }
                    attroff(COLOR_PAIR((y == g->y && x == g->x) ? 1 : 2));
                }
            break;
        case WIN:
            clear();
            mvwprintw(stdscr, y_offset, x_offset, "You won.");
            getch();
            clear();
            init(g);
            game_loop(g);
            break;
    }
    refresh();
}

void init_curses()
{
    initscr();
    noecho();
    curs_set(FALSE);
    start_color();
    init_pair(1, COLOR_WHITE, COLOR_GREEN);
    init_pair(2, COLOR_BLACK, COLOR_WHITE);
    init_pair(3, COLOR_BLACK, COLOR_RED);
    init_pair(4, COLOR_WHITE, COLOR_BLACK);
}


int main(int argc, char *argv[])
{
    int width = 10, height = 10;
    if (argc == 2)
    {
        switch (strtol(argv[1], (char **)NULL, 10))
        {
            case 0:
                width = 10;
                height = 10;
                break;
            case 1:
                width = 20;
                height = 15;
                break;
            case 2:
                width = 35;
                height = 15;
                break;

        }
    }
    else if (argc == 3)
    {
        width = strtol(argv[1], (char **)NULL, 10);
        height = strtol(argv[2], (char **)NULL, 10);
    }
    game_t game = {width, height};
    game.tiles = malloc(width * height * sizeof(tile_t));
    init(&game);
    init_curses();
    game_loop(&game);
    return 0;
}
