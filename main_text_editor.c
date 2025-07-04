#include "main_text_editor.h"

terminal_configurations old_config ;



enum editorKey {
  ARROW_LEFT = 256, // in GCC and Clang chars are signed and coded in a byte, so 128 is an out of range
                    // which will help us to differentiate from regular chars (previously 'w')
                    // However since windows uses the MSVC compiler that considers chars unsigned we chose 256 just to be safe (1 unsigned byte -> 255)
  ARROW_RIGHT,
  ARROW_UP,
  ARROW_DOWN,
  PAGE_UP,
  PAGE_DOWN,
  HOME_KEY,
  END_KEY,
  DELETE_KEY
};
////////////////////////////////////////////////////////////////
void move_cursor(int direction){

    if (direction == ARROW_UP) {
        if (old_config.cursor_y > 0)
            old_config.cursor_y--;}

    else if (direction == ARROW_DOWN) {
    if (old_config.cursor_y < old_config.window_size.ws_row - 1)
        old_config.cursor_y++;}

    else if (direction == ARROW_RIGHT) {
        if (old_config.cursor_x < old_config.window_size.ws_col - 1)
            old_config.cursor_x++;}

    else if (direction == ARROW_LEFT) {
        if (old_config.cursor_x > 0) 
            old_config.cursor_x--;
        }

    else if (direction == PAGE_UP){
        
        while (old_config.cursor_y > 0){
            old_config.cursor_y--;
        }
        
    }
    else if (direction == PAGE_DOWN){
       
        while (old_config.cursor_y <get_window_size().ws_row -1){
            old_config.cursor_y++;
        }
        
    }
    else if  (direction == END_KEY){//move to the edge right
         while (old_config.cursor_x <get_window_size().ws_col -1){
            old_config.cursor_x++;
        }
    }
    else if (direction == HOME_KEY){//move to the far left
        while (old_config.cursor_x >0){
            old_config.cursor_x--;
        }
    }
}

void draw_tildes(int ws_row, int ws_col, text_buffer* tildes_buff) { 
    char welcome_buff[64];
    for (int i = 0; i < ws_row - 1; i++) {
            if (old_config.nrows <= i){
            append_buffer(tildes_buff, "~", 1);
            if (i == ws_row/2 && old_config.nrows ==0){
            /* write(STDOUT_FILENO, "~\r\n", 3) ; */
            /// Welcome message ///
                int big_welcome = snprintf(welcome_buff, sizeof(welcome_buff), "BEE text editor -- Version %s", BEE_version);
            
                if (big_welcome > ws_col){ 
                    append_buffer(tildes_buff, welcome_buff, ws_col);}
                else{
                    int center = (ws_col - big_welcome)/2 - 1; // The - 1 is to take account for ~
                    while (center != 0){
                        append_buffer(tildes_buff, " ", 1);
                        center--;
                    }
                    append_buffer(tildes_buff, welcome_buff, big_welcome);}
                }
            else{
                int len = old_config.nrows;
                if (len >old_config.window_size.ws_col){
                    len = old_config.window_size.ws_col;
                }
                append_buffer(tildes_buff, old_config.editor_row.row_data, len);
            }
            append_buffer(tildes_buff,"\x1b[K", 3); // K command : erease line; 1: left of the cursor (so we don't delete ~)
            append_buffer(tildes_buff,"\r\n", 2);}
            append_buffer(tildes_buff, "~", 1);
        }

    

   /*  write(STDOUT_FILENO, "~", 1) ;
    write(STDOUT_FILENO, "\x1b[1;1H", 6) ; */
}


void clear_screen(int ws_row, int ws_col){
    text_buffer initializing_screen = {NULL,0};
    // we'll hide the cursor before drawing the tildes
    append_buffer(&initializing_screen,"\x1b[?25l", 6);//?25: hide cursor; l: lowercase "L" -> disable (cursor)

    //we'll replace this clearing command with a line by line one in draw_tildes
    /* append_buffer(&initializing_screen,"\x1b[2J", 4); */// J command: clear the screen ; 2: all of the screen
    append_buffer(&initializing_screen,"\x1b[1;1H", 6); // H command : cursor position
    /* write(STDOUT_FILENO, "\x1b[2J", 4) ;
    write(STDOUT_FILENO, "\x1b[1;1H", 6) ; // The values by default are 1;1 so we could've simply written \x1b[ whic
                                           // would only take 3 bytes */
    draw_tildes(ws_row, ws_col, &initializing_screen);

    // here we'll be positioning the cursor as indicated by the coordinates
    char cursor_at_will[32];
    snprintf(cursor_at_will, sizeof(cursor_at_will), "\x1b[%d;%dH", old_config.cursor_y + 1, old_config.cursor_x + 1);

    //once we're done drawing the tildes we recover the cursor;
    append_buffer(&initializing_screen,"\x1b[?25h", 6);
    append_buffer(&initializing_screen, cursor_at_will, strlen(cursor_at_will));

    write(STDOUT_FILENO, (&initializing_screen)->text, (&initializing_screen)->length); //We could've simply written 
                                                                                        //write(STDOUT_FILENO, initializing_screen.text, initializing_screen.length)
                                                                                        //but we wanted to keep a similar structure with draw as this part was previously
                                                                                        //in draw_tildes
    free_text_buffer(&initializing_screen);

}


void kill(char* error_message){
    clear_screen( old_config.window_size.ws_row ,old_config.window_size.ws_col);
    fprintf(stderr, "%s: %s\n" ,error_message, strerror(errno) );
    exit(EXIT_FAILURE);
}

////////////////////////////////////////////////////////////////


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
    while ((nbr_bytes = read(STDIN_FILENO, &key, 1)) != 1 ) {
        if (nbr_bytes != 1 && errno != EAGAIN) {kill("reading error in read_one_key\r\n");}
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

/////////////////////////////////////////////////////////////////////

void key_process() {
    int key = read_one_key();

    if (key == CTRL_KEY('q')) {
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[1;1H", 6);
        exit(EXIT_SUCCESS);  // Exit the program
    } 
    else if (key == ARROW_UP || ARROW_DOWN ||ARROW_LEFT ||  ARROW_RIGHT 
        ||PAGE_DOWN||PAGE_DOWN ||HOME_KEY||END_KEY ||DELETE_KEY) {
       move_cursor(key);
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
         }
        }
    return window_size;}


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
void OpenEditor(char *filename){
    FILE *fptr = fopen(filename,"r");
    if(fptr ==NULL){kill("fopen doesn'r work");};

    char *opening_line = NULL; 
    size_t cap = 0;
    ssize_t len; 
    len = getline(&opening_line,&cap,fptr);
    if(len != -1){//assert exit succes
        while(len >0 && opening_line[len -1] != '\r' && opening_line[len -1] !='\n' ){
            len--;
        }
        old_config.editor_row.row_size = len;
        old_config.editor_row.row_data  = malloc(len + 1);
        memcpy(old_config.editor_row.row_data,opening_line,len);
        old_config.editor_row.row_data[len] = '\0';
        old_config.nrows = 1;
    }
    free(opening_line);
    free(fptr);
}
int main(int argc,char *argv) {
    struct termios new_settings; 
    // initializing cursor position
    old_config.cursor_x = 0;
    old_config.cursor_y = 0;
    old_config.nrows = 0;
    
    // Récupérer les anciens réglages avant toute modification
    if (tcgetattr(STDIN_FILENO, &old_config.old_settings) == -1) {
        kill("Could not get terminal attributes");
    }

    new_settings = old_config.old_settings;
    enable_raw_mode(&new_settings);  
    old_config.window_size = get_window_size(); 
    if (argc >1){
        OpenEditor(argv[1]);
    }
    while (1) {
        clear_screen(old_config.window_size.ws_row, old_config.window_size.ws_col);
        key_process();
    }

    return EXIT_SUCCESS;
}
