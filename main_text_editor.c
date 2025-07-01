#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <errno.h>

/* We'll use it as a macro and not as function 
because the switch case statement (in editorProcessKeypress function) 
does not support a variable returned from another function */
#define CTRL_KEY(k) (k & 0x1f)

struct termios old_settings ;


////////////////////////////////////////////////////////////////

void kill(char* error_message){
    fprintf(stderr, error_message, errno );
    exit(EXIT_FAILURE);
}

////////////////////////////////////////////////////////////////

void disable_raw_mode(){
    if(tcsetattr(STDERR_FILENO, TCSAFLUSH, &old_settings) == -1){kill("User's attribustions caused an error");} 
}

///////////////////////////////////////////////////////////////

void enable_raw_mode(struct termios *settings) {
    atexit(disable_raw_mode) ; 
    // Removing Echo
    // Disabling Canonical mode ( A mode where input is only considered when Enter is tapped)
    settings->c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN); 
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
    tcsetattr(STDERR_FILENO, TCSAFLUSH, settings) ; 
}

///////////////////////////////////////////////////////

char editorReadKey() {
    char c ;
    ssize_t nbr_bytes ; 
    while ((nbr_bytes = read(STDIN_FILENO, &c, 1)) != 1 ) {
        if (nbr_bytes != 1 && errno != EAGAIN) {kill("reading error in editorReadKey\r\n");}
    }
    return c ; 
}

/////////////////////////////////////////////////////////////////////

void editorProcessKeypress() {
    char c = editorReadKey() ;
    switch (c) {
        case CTRL_KEY('q') :
            exit(EXIT_SUCCESS) ;
            break ;
    }
}

void Clearscrenn(){
    write(STDOUT_FILENO, "\x1b[2J", 4) ;
    write(STDOUT_FILENO, "\x1b[1;1H", 6) ;
}
/////////////////////////////////////////////////////////////////////

int main() {

    struct termios new_settings ; 
    // Old_settings are to be preserved until exit
    tcgetattr(STDIN_FILENO, &old_settings) ;
    // We'll be loading the actual settings in a termios type to be modified using appropriate functions
    if(tcgetattr(STDIN_FILENO, &new_settings) == -1){kill("Old settings caused an error\n");} ;
    enable_raw_mode(&new_settings) ;
   /*  close(STDIN_FILENO); // error trigger */ 
    while (1){
        Clearscrenn() ;    
        editorProcessKeypress();}
    return EXIT_SUCCESS ;
}