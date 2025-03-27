#include <stdlib.h>

#define PIXEL_BUFFER_BASE 0xC8000000
#define KEY_BASE 0xFF200050
#define SW_BASE 0xFF200040
#define HEX3_HEX0_BASE 0xFF200020

#define BOARD_SIZE 7
#define CELL_SIZE 30
#define NUM_MINES 10
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240

// Colors (RGB565)
#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF
#define COLOR_GRAY 0xC618
#define COLOR_RED 0xF800
#define COLOR_GREEN 0x07E0
#define COLOR_BLUE 0x001F
#define COLOR_YELLOW 0xFFE0
#define COLOR_CYAN 0x07FF
#define COLOR_MAGENTA 0xF81F

char board[BOARD_SIZE][BOARD_SIZE];
char revealed[BOARD_SIZE][BOARD_SIZE];
char flagged[BOARD_SIZE][BOARD_SIZE];

// Digit font: 5x7 bitmap font for digits 1 to 8
const char digits[8][7] = {
    {0x04, 0x0C, 0x14, 0x04, 0x04, 0x04, 0x1F}, // 1
    {0x0E, 0x11, 0x01, 0x06, 0x08, 0x10, 0x1F}, // 2
    {0x0E, 0x11, 0x01, 0x06, 0x01, 0x11, 0x0E}, // 3
    {0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}, // 4
    {0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}, // 5
    {0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}, // 6
    {0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}, // 7
    {0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}  // 8
};

void delay(volatile int count)
{
    while (count-- > 0)
        asm volatile("nop");
}

void update_hex_display(int value)
{
    volatile int *hex_ptr = (int *)HEX3_HEX0_BASE;

    // Display only on HEX0 (rightmost digit)
    int hex_map[10] = {
        0x3F, // 0
        0x06, // 1
        0x5B, // 2
        0x4F, // 3
        0x66, // 4
        0x6D, // 5
        0x7D, // 6
        0x07, // 7
        0x7F, // 8
        0x6F  // 9
    };

    int digit = (value >= 0 && value <= 9) ? hex_map[value] : 0;
    *hex_ptr = digit; // Write to HEX0 only
}

void draw_pixel(int x, int y, short color)
{
    if (x >= 0 && x < SCREEN_WIDTH && y >= 0 && y < SCREEN_HEIGHT)
    {
        volatile short *pixel_ptr = (short *)(PIXEL_BUFFER_BASE + (y << 10) + (x << 1));
        *pixel_ptr = color;
    }
}

void draw_rect(int x, int y, int w, int h, short color)
{
    for (int i = 0; i < h; i++)
        for (int j = 0; j < w; j++)
            draw_pixel(x + j, y + i, color);
}

void draw_digit(int x, int y, char num, short color)
{
    if (num < '1' || num > '8')
        return;
    const char *glyph = digits[num - '1'];
    for (int row = 0; row < 7; row++)
    {
        char bits = glyph[row];
        for (int col = 0; col < 5; col++)
        {
            if ((bits >> (4 - col)) & 1)
                draw_pixel(x + col, y + row, color);
        }
    }
}

void draw_flag(int x, int y)
{
    int px = x * CELL_SIZE + 10;
    int py = y * CELL_SIZE + 10;
    draw_rect(px, py, 6, 6, COLOR_MAGENTA);  // flag box
    draw_pixel(px + 2, py + 2, COLOR_WHITE); // center dot
}

void draw_board(int cursor_x, int cursor_y)
{
    for (int y = 0; y < BOARD_SIZE; y++)
    {
        for (int x = 0; x < BOARD_SIZE; x++)
        {
            int px = x * CELL_SIZE;
            int py = y * CELL_SIZE;

            short color = COLOR_GRAY;
            if (revealed[y][x])
            {
                color = (board[y][x] == '*') ? COLOR_RED : COLOR_WHITE;
            }
            draw_rect(px, py, CELL_SIZE - 2, CELL_SIZE - 2, color);

            if (revealed[y][x] && board[y][x] >= '1' && board[y][x] <= '8')
            {
                draw_digit(px + 10, py + 10, board[y][x], COLOR_BLUE);
            }
            else if (flagged[y][x] && !revealed[y][x])
            {
                draw_flag(x, y);
            }

            if (x == cursor_x && y == cursor_y)
            {
                draw_rect(px, py, CELL_SIZE - 2, 2, COLOR_YELLOW);
                draw_rect(px, py + CELL_SIZE - 4, CELL_SIZE - 2, 2, COLOR_YELLOW);
                draw_rect(px, py, 2, CELL_SIZE - 2, COLOR_YELLOW);
                draw_rect(px + CELL_SIZE - 4, py, 2, CELL_SIZE - 2, COLOR_YELLOW);
            }
        }
    }
}

int count_adjacent_mines(int x, int y)
{
    int count = 0;
    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && ny >= 0 && nx < BOARD_SIZE && ny < BOARD_SIZE)
            {
                if (board[ny][nx] == '*')
                {
                    count++;
                }
            }
        }
    }
    return count;
}

void generate_board()
{
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            board[y][x] = revealed[y][x] = flagged[y][x] = 0;

    int placed = 0;
    while (placed < NUM_MINES)
    {
        int x = rand() % BOARD_SIZE, y = rand() % BOARD_SIZE;
        if (board[y][x] != '*')
        {
            board[y][x] = '*';
            placed++;
        }
    }

    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            if (board[y][x] != '*')
            {
                int count = count_adjacent_mines(x, y);
                board[y][x] = (count > 0) ? '0' + count : ' ';
            }
}

void reveal(int x, int y)
{
    if (x < 0 || y < 0 || x >= BOARD_SIZE || y >= BOARD_SIZE || revealed[y][x] || flagged[y][x])
        return;
    revealed[y][x] = 1;
    if (board[y][x] == ' ')
        for (int dx = -1; dx <= 1; dx++)
            for (int dy = -1; dy <= 1; dy++)
                if (!(dx == 0 && dy == 0))
                    reveal(x + dx, y + dy);
}

int read_keys()
{
    volatile int *KEY_ptr = (int *)KEY_BASE;
    return ~(*KEY_ptr) & 0xF;
}

int read_switches()
{
    volatile int *SW_ptr = (int *)SW_BASE;
    return *SW_ptr;
}

void wait_for_key_release()
{
    while (read_keys() != 0)
        ;
}

void reveal_all_mines()
{
    for (int y = 0; y < BOARD_SIZE; y++)
        for (int x = 0; x < BOARD_SIZE; x++)
            if (board[y][x] == '*')
                revealed[y][x] = 1;
}

void game_loop()
{
    int cx = 0, cy = 0, game_over = 0;
    int sw_prev = 0;

    while (1)
    {
        draw_board(cx, cy);
        if (game_over)
            continue;

        int keys = read_keys();
        int sw = read_switches();
        draw_board(cx, cy);

        if (keys & 0x1)
        { // Reveal
            if (!flagged[cy][cx])
            {
                if (board[cy][cx] == '*')
                {
                    reveal_all_mines();
                    game_over = 1;
                    draw_board(cx, cy); // Show all mines visually
                    continue;
                }
                else
                {
                    reveal(cx, cy);
                }
            }
            wait_for_key_release();
        }
        if (keys & 0x2 && cx < BOARD_SIZE - 1)
        {
            cx++;
            wait_for_key_release();
        }
        if (keys & 0x4 && cx > 0)
        {
            cx--;
            wait_for_key_release();
        }
        if (keys & 0x8 && cy > 0)
        {
            cy--;
            wait_for_key_release();
        }

        // DOWN: SW[0] edge detect
        if (((sw & 0x1) != 0) && ((sw_prev & 0x1) == 0) && cy < BOARD_SIZE - 1)
        {
            cy++;
        }

        // Toggle flag with SW[1] edge detect
        if (((sw & 0x2) != 0) && ((sw_prev & 0x2) == 0))
        {
            if (!revealed[cy][cx])
            {
                flagged[cy][cx] = !flagged[cy][cx];

                // Count flags and update display
                int count = 0;
                for (int y = 0; y < BOARD_SIZE; y++)
                    for (int x = 0; x < BOARD_SIZE; x++)
                        if (flagged[y][x])
                            count++;

                int flags_remaining = NUM_MINES - count;
                update_hex_display(flags_remaining);
            }
        }

        // Reset game with SW[2]
        // Restart game on rising edge of SW[2]
        if (((sw & 0x4) != 0) && ((sw_prev & 0x4) == 0))
        {
            generate_board();
            for (int y = 0; y < BOARD_SIZE; y++)
                for (int x = 0; x < BOARD_SIZE; x++)
                    revealed[y][x] = flagged[y][x] = 0;

            game_over = 0;
            cx = 0;
            cy = 0;

            draw_board(cx, cy);     // Immediately reflect the reset
            wait_for_key_release(); // optional: makes sure buttons aren't held down
            update_hex_display(NUM_MINES);
        }

        delay(80000);
        sw_prev = sw;
    }
}

int main()
{
    generate_board();
    draw_board(0, 0);
    update_hex_display(NUM_MINES);
    game_loop();
    return 0;
}