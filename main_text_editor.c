#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <ctype.h>
/* We'll use it as a macro and not as function 
because the switch case statement (in key_process function) 
does not support a variable returned from another function */
#define CTRL_KEY(k) (k & 0x1f)

struct winsize get_window_size() {
    struct winsize window_size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == -1 || window_size.ws_col == 0){ kill("Couldn't get the size of the window");}
    return window_size ;
}


typedef struct {
    struct termios old_settings ;
    struct winsize window_size; 
} terminal_configurations ;

terminal_configurations old_config ;


void draw_tildes(int ws_row) {   
    for (__uint8_t i = 0; i < ws_row - 1 ; i++) {
        write(STDOUT_FILENO, "~\r\n", 3) ;
    }
    write(STDOUT_FILENO, "~", 3) ;
    write(STDOUT_FILENO, "\x1b[1;1H", 6) ;
}


void clear_screen(int ws_row){
    write(STDOUT_FILENO, "\x1b[2J", 4) ;
    write(STDOUT_FILENO, "\x1b[1;1H", 6) ; // The values by default are 1;1 so we could've simply written \x1b[ whic
                                           // would only take 3 bytes
    draw_tildes(ws_row);

}


////////////////////////////////////////////////////////////////

void kill(char* error_message){
    clear_screen( old_config.window_size.ws_col );
    fprintf(stderr, "%s: %s\n" ,error_message, strerror(errno) );
    exit(EXIT_FAILURE);
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
    settings->c_lflag &= ~( ICANON | ISIG | IEXTEN); 
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
/////////////////////////////////////////////////////////////////////

int main() {
    struct termios new_settings ; 
    old_config.window_size = get_window_size();
    // Displaying the window's size
    /* printf("Screen rows : %d\n", old_config.window_size.ws_row) ;
    printf("Screen columns : %d\n", old_config.window_size.ws_col) ; */
    // config.old_settings are to be preserved until exit
    tcgetattr(STDIN_FILENO, &old_config.old_settings) ;
    // We'll be loading the actual settings in a termios type to be modified using appropriate functions
    if(tcgetattr(STDIN_FILENO, &new_settings) == -1){kill("Old settings caused an error\n");} ;
    enable_raw_mode(&new_settings) ;

/*     close(STDIN_FILENO); // error trigger */
    while (1){
        clear_screen(old_config.window_size.ws_row);
        key_process();}
        
    return EXIT_SUCCESS ;
}