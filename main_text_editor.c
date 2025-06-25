#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>

void enable_raw_mode() {
    /* To turn off the ECHO feature */
    struct termios settings;
    tcgetattr(STDIN_FILENO, &settings);
    settings.c_lflag &= ~ECHO;
    /* settings.c_lflag &= ~ICANON; */
    tcsetattr(STDERR_FILENO, TCSAFLUSH, &settings) ; 
}

int main() {
    enable_raw_mode();
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1){
        if (c == 'q'){break;}
        printf("The pressed keys: %c\n", c) ;}
   /*  returned_value = read(STDIN_FILENO, &c, 1);
    printf("The pressed keys: %c\n", c) ;
    while (returned_value == 1){
        returned_value = read(STDIN_FILENO, &c, 1);
        printf("The pressed keys: %c\n", c) ;} */
    

    return EXIT_SUCCESS ;
}