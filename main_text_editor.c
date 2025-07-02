#include "main_text_editor.h"
terminal_configurations old_config ;

////////////////////////////////////////////////////////////////

void draw_tildes(int ws_row,text_buffer* tildes_buff) { 
    for (int i = 0; i < ws_row - 1; i++) {
         /* write(STDOUT_FILENO, "~\r\n", 3) ; */
        append_buffer(tildes_buff, "~", 1);
        append_buffer(tildes_buff,"\x1b[K", 3); // K command : erease line; 1: left of the cursor (so we don't delete ~)
        append_buffer(tildes_buff,"\r\n", 2);}
    append_buffer(tildes_buff, "~", 1);
    append_buffer(tildes_buff, "\x1b[1;1H", 6);
    //once we're done drawing the tildes we recover the cursor;
    append_buffer(tildes_buff,"\x1b[?25h", 6);
    write(STDOUT_FILENO, tildes_buff->text, tildes_buff->length);
       
    
   /*  write(STDOUT_FILENO, "~", 1) ;
    write(STDOUT_FILENO, "\x1b[1;1H", 6) ; */
}


void clear_screen(int ws_row){
    text_buffer initializing_screen = {NULL,0};
    // we'll hide the cursor before drawing the tildes
    append_buffer(&initializing_screen,"\x1b[?25l", 6);//?25: hide cursor; l: lowercase "L" -> disable

    //we'll replace this clearing command with a line by line one in draw_tildes
    /* append_buffer(&initializing_screen,"\x1b[2J", 4); */// J command: clear the screen ; 2: all of the screen
    append_buffer(&initializing_screen,"\x1b[1;1H", 6); // H command : cursor position
    /* write(STDOUT_FILENO, "\x1b[2J", 4) ;
    write(STDOUT_FILENO, "\x1b[1;1H", 6) ; // The values by default are 1;1 so we could've simply written \x1b[ whic
                                           // would only take 3 bytes */
    draw_tildes(ws_row, &initializing_screen);

    

    free_text_buffer(&initializing_screen);

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
         return window_size;}
        }}


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

void append_buffer(text_buffer* current_text_buff, char* c, int length_c){
    int length_b = current_text_buff->length;
    char* new_text = realloc(current_text_buff->text, length_b + length_c);

    // let's avoid any memory problems
    if (new_text == NULL) {
        printf("The text you wish to mqnipulate is NULL");
        return;
    }

    else{
        // now we add c to our current_text
        for (__uint8_t i = 0; i < length_c; i++) {
            new_text[length_b + i] = c[i];
        }
        current_text_buff->length += length_c;
        current_text_buff->text = new_text;
    }

}

void free_text_buffer(text_buffer* current_text_buffer){
    free(current_text_buffer->text); // length is just an int, only text has to be freed
}
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
