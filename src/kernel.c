#include <stdint.h>

volatile uint16_t *video = (uint16_t*)0xB8000;

#define WHITE 0x0F
#define GRAY  0x07
#define BLUE  0x09
#define DARK  0x01

char passcode[32];
char confirm[32];
char command_line[80];

int pass_len = 0;
int confirm_len = 0;
int active_box = 0;
int command_len = 0;

char current_dir[32] = "/";
char file_name[16][32];
char file_data[16][160];
int file_used[16];

void clear_screen(void)
{
    for (int i = 0; i < 80 * 25; i++)
        video[i] = (uint16_t)' ' | ((uint16_t)GRAY << 8);
}

void print_at(const char *text, int row, int column)
{
    int i = 0;
    while (text[i] != '\0' && column + i < 80)
    {
        video[row * 80 + column + i] =
            (uint16_t)text[i] | ((uint16_t)WHITE << 8);
        i++;
    }
}

void print_colored(const char *text, int row, int column, uint8_t attr)
{
    int i = 0;
    while (text[i] != '\0' && column + i < 80)
    {
        video[row * 80 + column + i] =
            (uint16_t)text[i] | ((uint16_t)attr << 8);
        i++;
    }
}

void print_char(char c, int row, int column)
{
    if (row >= 0 && row < 25 && column >= 0 && column < 80)
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

int strings_equal(char *a, const char *b)
{
    int i = 0;
    while (a[i] != '\0' || b[i] != '\0')
    {
        if (a[i] != b[i])
            return 0;
        i++;
    }
    return 1;
}

void copy_string(char *dst, const char *src, int max)
{
    int i = 0;
    while (src[i] != '\0' && i < max - 1)
    {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

int starts_with(char *s, const char *prefix)
{
    int i = 0;
    while (prefix[i] != '\0')
    {
        if (s[i] != prefix[i])
            return 0;
        i++;
    }
    return 1;
}

void terminal_header(void)
{
    clear_screen();
    print_colored("gcOS TERMINAL", 1, 3, BLUE);
    print_at("Type 'help' for commands.", 3, 3);
    print_colored("TEST: gcOS terminal is working!", 5, 3, BLUE);
}

void terminal_prompt(void)
{
    print_colored("gcos:", 22, 2, BLUE);
    print_at(current_dir, 22, 7);

    int p = 7;
    while (current_dir[p - 7] != '\0')
        p++;

    print_at("> ", 22, p);
    int start = p + 2;

    for (int i = 0; i < command_len && start + i < 79; i++)
        print_char(command_line[i], 22, start + i);

    print_char('_', 22, start + command_len);
}

void show_help(void)
{
    print_at("Available commands:", 5, 3);
    print_at("help       - show this list", 6, 5);
    print_at("clear      - clear the terminal", 7, 5);
    print_at("pwd        - show current directory", 8, 5);
    print_at("ls         - list files", 9, 5);
    print_at("cd <dir>   - change directory", 10, 5);
    print_at("touch <f>   - create a file", 11, 5);
    print_at("cat <f>     - read a file", 12, 5);
    print_at("write <f> <text> - write a file", 13, 5);
    print_at("rm <f>      - delete a file", 14, 5);
    print_at("echo <text> - print text", 15, 5);
    print_at("about       - gcOS system info", 16, 5);
    print_at("exit        - leave terminal", 17, 5);
}

int find_file(char *name)
{
    for (int i = 0; i < 16; i++)
    {
        if (file_used[i] && strings_equal(file_name[i], name))
            return i;
    }
    return -1;
}

void list_files(void)
{
    int row = 5;
    print_at("Files in current directory:", row++, 3);

    int found = 0;
    for (int i = 0; i < 16 && row < 21; i++)
    {
        if (file_used[i])
        {
            print_at(file_name[i], row++, 5);
            found = 1;
        }
    }

    if (!found)
        print_at("(empty)", row, 5);
}

void command_error(void)
{
    print_at("Unknown command. Type 'help'.", 19, 3);
}

void execute_command(void)
{
    command_line[command_len] = '\0';

    if (strings_equal(command_line, "help"))
    {
        terminal_header();
        show_help();
    }
    else if (strings_equal(command_line, "clear"))
    {
        terminal_header();
    }
    else if (strings_equal(command_line, "pwd"))
    {
        print_at(current_dir, 5, 3);
    }
    else if (strings_equal(command_line, "ls"))
    {
        list_files();
    }
    else if (starts_with(command_line, "cd "))
    {
        char *name = command_line + 3;
        if (strings_equal(name, "/") || strings_equal(name, ".."))
            copy_string(current_dir, "/", 32);
        else
        {
            copy_string(current_dir, "/", 32);
            print_at("Directory support is currently virtual.", 6, 3);
        }
    }
    else if (starts_with(command_line, "touch "))
    {
        char *name = command_line + 6;
        if (find_file(name) >= 0)
        {
            print_at("File already exists.", 5, 3);
        }
        else
        {
            for (int i = 0; i < 16; i++)
            {
                if (!file_used[i])
                {
                    file_used[i] = 1;
                    copy_string(file_name[i], name, 32);
                    file_data[i][0] = '\0';
                    print_at("File created.", 5, 3);
                    break;
                }
            }
        }
    }
    else if (starts_with(command_line, "cat "))
    {
        int n = find_file(command_line + 4);
        if (n < 0)
            print_at("File not found.", 5, 3);
        else
            print_at(file_data[n], 5, 3);
    }
    else if (starts_with(command_line, "write "))
    {
        char *p = command_line + 6;
        int i = 0;
        while (p[i] != ' ' && p[i] != '\0' && i < 31)
            i++;

        if (p[i] == ' ')
        {
            p[i] = '\0';
            int n = find_file(p);
            if (n >= 0)
            {
                copy_string(file_data[n], p + i + 1, 160);
                print_at("File written.", 5, 3);
            }
            else
                print_at("File not found. Use touch first.", 5, 3);
        }
        else
            print_at("Usage: write <file> <text>", 5, 3);
    }
    else if (starts_with(command_line, "rm "))
    {
        int n = find_file(command_line + 3);
        if (n < 0)
            print_at("File not found.", 5, 3);
        else
        {
            file_used[n] = 0;
            print_at("File deleted.", 5, 3);
        }
    }
    else if (starts_with(command_line, "echo "))
    {
        print_at(command_line + 5, 5, 3);
    }
    else if (strings_equal(command_line, "about"))
    {
        print_at("gcOS - gaming-focused operating system", 5, 3);
        print_at("Current build: kernel prototype", 6, 3);
        print_at("Guest filesystem: RAM prototype", 7, 3);
        print_at("Hardware access: keyboard + VGA", 8, 3);
    }
    else if (strings_equal(command_line, "test"))
    {
        print_colored("Hello from the gcOS terminal!", 5, 3, BLUE);
        print_at("Terminal input and command execution are working.", 6, 3);
    }
    else if (strings_equal(command_line, "exit"))
    {
        return;
    }
    else if (command_len != 0)
    {
        command_error();
    }
}

void terminal_session(void)
{
    command_len = 0;
    command_line[0] = '\0';
    terminal_header();
    terminal_prompt();

    while (1)
    {
        if (!(keyboard_status() & 1))
            continue;

        uint8_t sc = keyboard_read();

        if (sc & 0x80)
            continue;

        if (sc == 0x01)
            return;

        if (sc == 0x1C)
        {
            if (strings_equal(command_line, "exit"))
            {
                return;
            }

            execute_command();
            command_len = 0;
            command_line[0] = '\0';
            terminal_prompt();
            continue;
        }

        if (sc == 0x0E)
        {
            if (command_len > 0)
                command_len--;
            command_line[command_len] = '\0';
            terminal_prompt();
            continue;
        }

        char c = scancode_to_char(sc);
        if (c != 0 && command_len < 74)
        {
            command_line[command_len++] = c;
            command_line[command_len] = '\0';
            terminal_prompt();
        }
    }
}

void guest_home(void)
{
    clear_screen();
    print_colored("gcOS", 3, 36, BLUE);
    print_at("Guest Session", 7, 32);
    print_at("Welcome!", 10, 36);
    print_at("Press T to open Terminal", 13, 28);
    print_at("Press any other key to continue", 15, 24);
}

void guest_session(void)
{
    guest_home();

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
            guest_home();
        }
        else
            break;
    }
}

void draw_passcode_screen(void)
{
    clear_screen();
    print_colored("gcOS", 3, 36, BLUE);
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

void kernel_main(void)
{
    draw_passcode_screen();

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
                draw_passcode_screen();
                continue;
            }

            if (pass_len == 0 && confirm_len == 0)
            {
                clear_screen();
                print_colored("gcOS", 3, 36, BLUE);
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
            print_colored("gcOS", 3, 36, BLUE);
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
                    draw_passcode_screen();
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
            draw_passcode_screen();
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
            draw_passcode_screen();
        }
    }
}
