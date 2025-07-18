#include "../include/terminal_config.h"

int read_one_key() {
    char key ;
    ssize_t nbr_bytes ; 
    
    // Reading some additional bytes before triggering an error to verify 
    // if the screen is being cleared after an error is detected
    /*
    read(STDIN_FILENO, &c, 1);
    read(STDIN_FILENO, &c, 1);
    read(STDIN_FILENO, &c, 1);
    close(STDIN_FILENO); */
    while ((nbr_bytes = read(STDIN_FILENO, &key, 1)) != 1) {
        if (nbr_bytes == -1 && errno != EAGAIN) {
            kill("reading error in read_one_key\r\n");
            }
    }
    if (key == '\x1b'){
        char direction[3]; // 3 bytes because we may handle more escapes afterwards
        if (read(STDIN_FILENO, &direction[0], 1) != 1){ return '\x1b';} // there is no second byte just a space
        if (read(STDIN_FILENO, &direction[1], 1) != 1){ return '\x1b';} // first byte may exist but doesn't mean 
                                                                        // anything without the second one
        if (direction[0] == '['){
            if (direction[1] == 'A'){ return ARROW_UP;} // up
            else if (direction[1] == 'B'){ return ARROW_DOWN;} // down
            else if (direction[1] == 'C'){ return ARROW_RIGHT;} // right
            else if (direction[1] == 'D'){ return ARROW_LEFT;} // left
            else if (direction[1] == 'H'){ return HOME_KEY;}
            else if (direction[1] == 'F'){ return END_KEY;}
            else{
                //empty for the moment..cases wih only 2
                if (direction[1] == 'O'){
                     if (read(STDIN_FILENO, &direction[2], 1) != 1){ return '\x1b';} //there must be 
                                                                                    //something after O
                    if(direction[2] == 'H'){
                        return HOME_KEY;
                    }
                    if (direction[2] == 'F'){
                        return END_KEY;
                    }
                }
                if (read(STDIN_FILENO, &direction[2], 1) != 1){return '\x1b';}//third byte 
                                                                            //doesn't exist
                if (direction[2] == '~'){
                    if (direction[1] == '5'){return PAGE_UP;};
                    if (direction[1] == '6'){return PAGE_DOWN;};
                    if (direction[1] == '1'){return HOME_KEY;};
                    if (direction[1] == '7'){return HOME_KEY;};
                    if (direction[1] == '4'){return END_KEY;};
                    if (direction[1] == '8'){return END_KEY;};
                    if (direction[1] == '3'){return DELETE_KEY;};
                }
            }
        }

        return '\x1b'; // the sequence starts with a space but the rest isn't recognized
    }
    return key ; 
}


struct winsize get_window_size() {
    struct winsize window_size;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &window_size) == -1 || window_size.ws_col == 0){ 
        // if ioctl won't work on this system we'll request the window size manually
        ssize_t written = write(STDOUT_FILENO, "\x1b[1234C\x1b[1234B", 14);
        if (written == -1) { 
            kill("An error occured, couldn't get the size of the window");}
        else if (written < 14 ){
            kill("Partial write, writing process was interrupted (check close)");}
        else {
            window_size = cursor_position();
            /* read_one_key();
            kill("testing for now"); */
         }
        }
    return window_size;}



void kill(char* error_message){
    clear_screen( old_config.window_size.ws_row ,old_config.window_size.ws_col);
    fprintf(stderr, "%s: %s\n" ,error_message, strerror(errno) );
    exit(EXIT_FAILURE);
}


void disable_raw_mode(){
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_config.old_settings) == -1){kill("Failed to restore old settings");} 
}

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



struct winsize cursor_position(){
    int rows;
    int columns;
    struct winsize window_size;

    char position[32]; // even though the response isn't that big ( ^[[number1,number2R ) we'll allocate 32 bytes just to be safe
    __uint8_t i = 0;
    ssize_t written = write(STDOUT_FILENO, "\x1b[6n", 4);
    if (written == -1) {
        kill("An error occured, couldn't get the cursor's position");}

    else if(written < 4){kill("Partial write, writing process was interrupted (check close)");}

    while (i < sizeof(position) - 1) {
        if (read(STDIN_FILENO, &position[i], 1) != 1) {kill("reading problem in cursor_position");}
        if (position[i] == 'R') {break;} // R marks the end of this response
        i++;
    }
    position[i] = '\0'; // as any string position should be ended with a '\0'
    if (position[0] != '\x1b' || position[1] != '['){
         kill("False format for position");}
    // Now we'll parse position to get the coordinates of the cursor
    if (sscanf(&position[2], "%d;%d", &rows, &columns) != 2) {
        kill ("couldn't get rows and columns");}
    window_size.ws_col = columns;
    window_size.ws_row = rows;
    return window_size;
}    
