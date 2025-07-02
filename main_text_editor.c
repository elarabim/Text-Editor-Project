#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <ctype.h>
#include <string.h>  // Pour strerror

/* We'll use it as a macro and not as function 
because the switch case statement (in key_process function) 
does not support a variable returned from another function */
#define CTRL_KEY(k) (k & 0x1f)



typedef struct {
    struct termios old_settings ;
    struct winsize window_size; 
} terminal_configurations ;

terminal_configurations old_config ;



////////////////////////////////////////////////////////////////

void draw_tildes(int ws_row) {   
    for (__uint8_t i = 0; i < ws_row - 1 ; i++) {
        write(STDOUT_FILENO, "~\r\n", 3) ;
    }
    write(STDOUT_FILENO, "~", 1) ;
    write(STDOUT_FILENO, "\x1b[1;1H", 6) ;
}


void clear_screen(int ws_row){
    write(STDOUT_FILENO, "\x1b[2J", 4) ;
    write(STDOUT_FILENO, "\x1b[1;1H", 6) ; // The values by default are 1;1 so we could've simply written \x1b[ whic
                                           // would only take 3 bytes
    draw_tildes(ws_row);

}


void kill(char* error_message){
    clear_screen( old_config.window_size.ws_col );
    fprintf(stderr, "%s: %s\n" ,error_message, strerror(errno) );
    exit(EXIT_FAILURE);
}

////////////////////////////////////////////////////////////////


char read_one_key() {
    char c ;
    ssize_t nbr_bytes ; 
    
    // Reading some additional bytes before triggering an error to verify 
    // if the screen is being cleared after an error is detected
    /*
    read(STDIN_FILENO, &c, 1);
    read(STDIN_FILENO, &c, 1);
    read(STDIN_FILENO, &c, 1);
    close(STDIN_FILENO); */
    while ((nbr_bytes = read(STDIN_FILENO, &c, 1)) != 1 ) {
        if (nbr_bytes != 1 && errno != EAGAIN) {kill("reading error in read_one_key\r\n");}
    }
    return c ; 
}

/////////////////////////////////////////////////////////////////////

void key_process() {
    char c = read_one_key() ;
    switch (c) {
        case CTRL_KEY('q') :
            // clear_screen( old_config.window_size.ws_col) ;
            write(STDOUT_FILENO, "\x1b[2J", 4);
            write(STDOUT_FILENO, "\x1b[1;1H", 6);      
            exit(EXIT_SUCCESS) ;
            break ;
    }
}

/////////////////////////////////////////////////////////////////////
struct winsize cursor_position(){
    int rows;
    int columns;
    struct winsize window_size;

    char buf[32];
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) kill("couldn't write down position");
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';
    if (buf[0] != '\x1b' || buf[1] != '[') kill("False format for position");
    if (sscanf(&buf[2], "%d;%d", &rows, &columns) != 2) kill ("couldn't get rows and columns");
    window_size.ws_col = columns;
    window_size.ws_row = rows;
    return window_size;
}    


struct winsize get_window_size() {
    struct winsize window_size;/* 
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == -1 || window_size.ws_col == 0){ */
        ssize_t written = write(STDOUT_FILENO, "\x1b[1234C\x1b[1234B", 14);
        if (written == -1) { 
            kill("An error occured, couldn't get the size of the window");}
        else if (written < 14 ){
            kill("Partial write, writing process was interrupted (check close)");}
        else {
            return cursor_position();
            /* read_one_key();
            kill("testing for now"); *///}
        }
}


////////////////////////////////////////////////////////////////

void disable_raw_mode(){
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_config.old_settings) == -1){kill("Failed to restore old settings");} 
}

///////////////////////////////////////////////////////////////

void enable_raw_mode(struct termios *settings) {
    atexit(disable_raw_mode) ; 
    // Removing Echo
    // Disabling Canonical mode ( A mode where input is only considered when Enter is tapped)
    settings->c_lflag &= ~( ECHO | ICANON | ISIG | IEXTEN); 
    // Disabling signal sending control commands
    /* settings->c_lflag &= ~ISIG ; */ // Ctrl - C + Ctrl - Z
    // Disabling Stop transmition and resume transmition commands
    /* settings->c_iflag &= ~IXON ; // Ctrl - S + Ctrl - Q */
    // Disabling Wating command
    /* settings->c_lflag &= ~IEXTEN;// Ctrl - V */
    settings->c_cflag |= (CS8);
    settings->c_iflag &= ~(IXON | ICRNL | ISTRIP | INPCK | BRKINT);// Ctr l - M
    settings->c_oflag &= ~(OPOST);// /r/n -> /n
    settings->c_cc[VMIN] = 0;
    settings->c_cc[VTIME] = 100;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, settings) == -1) {
        kill("Failed to enable raw mode");
    }
}

///////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////

int main() {
    struct termios new_settings; 

    // Récupérer les anciens réglages avant toute modification
    if (tcgetattr(STDIN_FILENO, &old_config.old_settings) == -1) {
        kill("Could not get terminal attributes");
    }

    new_settings = old_config.old_settings;
    enable_raw_mode(&new_settings);  
    old_config.window_size = get_window_size();  

    while (1) {
        clear_screen(old_config.window_size.ws_row);
        key_process();
    }

    return EXIT_SUCCESS;


}
