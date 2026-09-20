#include <stdint.h>

volatile uint16_t *video = (uint16_t*)0xB8000;

#define WHITE 0x0F
#define GRAY  0x07
#define BLUE  0x01
#define LIGHT_BLUE 0x09

char passcode[32];
char confirm[32];
char terminal_line[80];

int pass_len = 0;
int confirm_len = 0;
int active_box = 0;
int terminal_len = 0;
int terminal_open = 0;

void clear_screen(void)
{
    for (int i = 0; i < 80 * 25; i++)
        video[i] = (uint16_t)' ' | ((uint16_t)GRAY << 8);
}

void print_at(const char *text, int row, int column)
{
    int i = 0;
    while (text[i] != '\0')
    {
        video[row * 80 + column + i] =
            (uint16_t)text[i] | ((uint16_t)WHITE << 8);
        i++;
    }
}

void print_char(char c, int row, int column)
{
    video[row * 80 + column] =
        (uint16_t)c | ((uint16_t)WHITE << 8);
}

void draw_box(int row, int column, int length)
{
    print_at("[", row, column);
    for (int i = 0; i < length; i++)
        print_char(' ', row, column + 1 + i);
    print_at("]", row, column + length + 1);
}

void draw_passcode(char *text, int length, int row)
{
    (void)text;
    draw_box(row, 29, 20);
    for (int i = 0; i < length && i < 20; i++)
        print_char('*', row, 30 + i);
}

void draw_screen(void)
{
    clear_screen();
    print_at("gcOS", 3, 36);
    print_at("Guest Session", 6, 32);
    print_at("Create a passcode", 10, 29);
    draw_passcode(passcode, pass_len, 12);
    print_at("Confirm passcode", 15, 30);
    draw_passcode(confirm, confirm_len, 17);
    print_at("ENTER = Continue", 21, 31);

    if (active_box == 0)
        print_at("< Create passcode", 23, 28);
    else
        print_at("< Confirm passcode", 23, 28);
}

char scancode_to_char(uint8_t sc)
{
    switch (sc)
    {
        case 0x02: return '1'; case 0x03: return '2';
        case 0x04: return '3'; case 0x05: return '4';
        case 0x06: return '5'; case 0x07: return '6';
        case 0x08: return '7'; case 0x09: return '8';
        case 0x0A: return '9'; case 0x0B: return '0';
        case 0x10: return 'q'; case 0x11: return 'w';
        case 0x12: return 'e'; case 0x13: return 'r';
        case 0x14: return 't'; case 0x15: return 'y';
        case 0x16: return 'u'; case 0x17: return 'i';
        case 0x18: return 'o'; case 0x19: return 'p';
        case 0x1E: return 'a'; case 0x1F: return 's';
        case 0x20: return 'd'; case 0x21: return 'f';
        case 0x22: return 'g'; case 0x23: return 'h';
        case 0x24: return 'j'; case 0x25: return 'k';
        case 0x26: return 'l'; case 0x2C: return 'z';
        case 0x2D: return 'x'; case 0x2E: return 'c';
        case 0x2F: return 'v'; case 0x30: return 'b';
        case 0x31: return 'n'; case 0x32: return 'm';
        case 0x39: return ' ';
        case 0x35: return '/';
        case 0x0C: return '-';
        case 0x0D: return '=';
        case 0x33: return ',';
        case 0x34: return '.';
        default: return 0;
    }
}

uint8_t keyboard_read(void)
{
    uint8_t value;
    __asm__ volatile ("inb $0x60, %0" : "=a"(value));
    return value;
}

uint8_t keyboard_status(void)
{
    uint8_t value;
    __asm__ volatile ("inb $0x64, %0" : "=a"(value));
    return value;
}

int strings_equal(char *a, char *b)
{
    int i = 0;
    while (a[i] != '\0' || b[i] != '\0')
    {
        if (a[i] != b[i]) return 0;
        i++;
    }
    return 1;
}

void terminal_print(const char *text, int row)
{
    int i = 0;
    while (text[i] != '\0' && i < 78)
    {
        video[row * 80 + i] =
            (uint16_t)text[i] | ((uint16_t)LIGHT_BLUE << 8);
        i++;
    }
}

void terminal_draw(void)
{
    clear_screen();
    terminal_print("gcOS Terminal", 1);
    print_at("Type commands below. Press ESC to close.", 3, 4);
    print_at("help - show commands", 5, 4);
    print_at("clear - clear terminal", 6, 4);
    print_at("exit - return to Guest Session", 7, 4);
    print_at("echo <text> - print text", 8, 4);

    print_at("gcOS> ", 22, 2);
    for (int i = 0; i < terminal_len && i < 74; i++)
        print_char(terminal_line[i], 8 + 14, 8 + i);
}

void terminal_prompt(void)
{
    print_at("gcOS> ", 22, 2);
    for (int i = 0; i < terminal_len && i < 74; i++)
        print_char(terminal_line[i], 22, 8 + i);
    print_char('_', 22, 8 + terminal_len);
}

void terminal_execute(void)
{
    terminal_line[terminal_len] = '\0';

    if (strings_equal(terminal_line, "help"))
    {
        print_at("Commands: help, clear, exit, echo <text>", 18, 4);
    }
    else if (strings_equal(terminal_line, "clear"))
    {
        terminal_draw();
    }
    else if (strings_equal(terminal_line, "exit"))
    {
        terminal_open = 0;
        clear_screen();
        print_at("gcOS", 3, 36);
        print_at("Guest Session", 8, 32);
        print_at("Press T to open Terminal", 13, 28);
        print_at("Press any other key to continue", 15, 24);
        return;
    }
    else if (terminal_len >= 5 &&
             terminal_line[0]=='e' && terminal_line[1]=='c' &&
             terminal_line[2]=='h' && terminal_line[3]=='o' &&
             terminal_line[4]==' ')
    {
        print_at(terminal_line + 5, 18, 4);
    }
    else if (terminal_len != 0)
    {
        print_at("Unknown command. Type help.", 18, 4);
    }

    terminal_len = 0;
    terminal_line[0] = '\0';
    terminal_prompt();
}

void terminal_session(void)
{
    terminal_open = 1;
    terminal_len = 0;
    terminal_line[0] = '\0';
    terminal_draw();
    terminal_prompt();

    while (terminal_open)
    {
        if (!(keyboard_status() & 1))
            continue;

        uint8_t sc = keyboard_read();

        if (sc & 0x80)
            continue;

        if (sc == 0x01)
        {
            terminal_open = 0;
            break;
        }

        if (sc == 0x1C)
        {
            terminal_execute();
            continue;
        }

        if (sc == 0x0E)
        {
            if (terminal_len > 0)
                terminal_len--;
            terminal_line[terminal_len] = '\0';
            terminal_prompt();
            continue;
        }

        char c = scancode_to_char(sc);
        if (c != 0 && terminal_len < 74)
        {
            terminal_line[terminal_len++] = c;
            terminal_line[terminal_len] = '\0';
            terminal_prompt();
        }
    }
}

void guest_session(void)
{
    clear_screen();
    print_at("gcOS", 3, 36);
    print_at("Guest Session", 7, 32);
    print_at("Welcome!", 10, 36);
    print_at("Press T to open Terminal", 13, 28);
    print_at("Press any other key to continue", 15, 24);

    while (1)
    {
        if (!(keyboard_status() & 1))
            continue;

        uint8_t sc = keyboard_read();
        if (sc & 0x80)
            continue;

        if (sc == 0x14)
        {
            terminal_session();
            clear_screen();
            print_at("gcOS", 3, 36);
            print_at("Guest Session", 7, 32);
            print_at("Welcome!", 10, 36);
            print_at("Press T to open Terminal", 13, 28);
            print_at("Press any other key to continue", 15, 24);
        }
        else
        {
            break;
        }
    }
}

void kernel_main(void)
{
    draw_screen();

    while (1)
    {
        if (!(keyboard_status() & 1))
            continue;

        uint8_t sc = keyboard_read();

        if (sc & 0x80)
            continue;

        if (sc == 0x1C)
        {
            if (active_box == 0)
            {
                active_box = 1;
                draw_screen();
                continue;
            }

            if (pass_len == 0 && confirm_len == 0)
            {
                clear_screen();
                print_at("gcOS", 3, 36);
                print_at("No password set", 10, 32);
                print_at("Press any key to open", 14, 29);

                while (1)
                {
                    if (keyboard_status() & 1)
                    {
                        keyboard_read();
                        guest_session();
                        return;
                    }
                }
            }

            if (strings_equal(passcode, confirm))
            {
                guest_session();
                return;
            }

            clear_screen();
            print_at("gcOS", 3, 36);
            print_at("Passcodes don't match", 11, 29);
            print_at("Press any key to try again", 15, 27);

            while (1)
            {
                if (keyboard_status() & 1)
                {
                    keyboard_read();
                    pass_len = 0;
                    confirm_len = 0;
                    passcode[0] = '\0';
                    confirm[0] = '\0';
                    active_box = 0;
                    draw_screen();
                    break;
                }
            }
            continue;
        }

        if (sc == 0x0E)
        {
            if (active_box == 0 && pass_len > 0)
            {
                pass_len--;
                passcode[pass_len] = '\0';
            }
            else if (active_box == 1 && confirm_len > 0)
            {
                confirm_len--;
                confirm[confirm_len] = '\0';
            }
            draw_screen();
            continue;
        }

        char c = scancode_to_char(sc);

        if (c != 0)
        {
            if (active_box == 0 && pass_len < 20)
            {
                passcode[pass_len++] = c;
                passcode[pass_len] = '\0';
            }
            else if (active_box == 1 && confirm_len < 20)
            {
                confirm[confirm_len++] = c;
                confirm[confirm_len] = '\0';
            }
            draw_screen();
        }
    }
}
