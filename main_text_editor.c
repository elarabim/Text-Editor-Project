#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>


struct termios old_settings ;
void disable_raw_mode(){
    tcsetattr(STDERR_FILENO, TCSAFLUSH, &old_settings); 
}

void enable_raw_mode(struct termios *settings) {
    atexit(disable_raw_mode) ; 
    settings->c_lflag &= ~ECHO;
    settings->c_lflag &= ~ICANON; 
    settings->c_lflag &= ~ISIG ;
    settings->c_iflag &= ~IXON ;
    tcsetattr(STDERR_FILENO, TCSAFLUSH, settings); 
}


int main() {

    struct termios new_settings ; 
    // Old_settings are to be preserved until exit
    tcgetattr(STDIN_FILENO, &old_settings) ;
    // We'll be loading the actual settings in a termios type to be modified using appropriate functions
    tcgetattr(STDIN_FILENO, &new_settings) ;
    enable_raw_mode(&new_settings) ;
    char c;
    while (read(STDIN_FILENO, &c, 1) == 1){
        if (c == 'q')
        {
            break ;
        }
        /* Print the printable ascii characters */
        __uint8_t ascii = c ; 
        if (ascii >= 32 && ascii <= 126) {
            printf("%d ('%c')\n", c, c) ;
        }
        else {
            printf("Unprintable character : %d\n", c) ;
        }
        //printf("The pressed keys: %c\n", c) ;
        }

    return EXIT_SUCCESS ;
}: