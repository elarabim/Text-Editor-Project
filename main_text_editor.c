#include "main_text_editor.h"

terminal_configurations old_config ;



///////////////////////////////////////////////////////

void append_buffer(text_buffer* current_text_buff, char* c, int length_c){
    int length_b = current_text_buff->length;
    char* new_text = realloc(current_text_buff->text, length_b + length_c);

    // let's avoid any memory problems
    if (new_text == NULL) {
        printf("The text you wish to manipulate is NULL");
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
int cusror_x2render_x(plain_row *row,int cursor_x){
    int render_x =0;
    for (int i=0;i<cursor_x;i++){
        render_x++;//normal character or a tab
        if (row->row_data[i] == '\t'){
            while (render_x%BEE_TAB_STOP != 0){
                render_x++;
            }
        }
    }
    return render_x;
}
void scroll(){
    /* Vertical scrolling */
    plain_row *row = (old_config.cursor_y >= old_config.nrows) ? NULL : &old_config.editor_row[old_config.cursor_y];
    if (row) {
        old_config.render_x = cusror_x2render_x(row, old_config.cursor_x);
    } else {
        old_config.render_x = 0;
    }
    if (old_config.cursor_y < old_config.row_offset){
        old_config.render_x = cusror_x2render_x(&old_config.editor_row[old_config.cursor_y],old_config.cursor_x);
    }
    if (old_config.cursor_y < old_config.row_offset) {
        // this checks if the cursor is above visibale window
        old_config.row_offset = old_config.cursor_y;}
    else if (old_config.cursor_y >= (old_config.window_size).ws_row + old_config.row_offset) { 
        // this checks if the cursor is bellow visibale window
        old_config.row_offset = old_config.cursor_y - (old_config.window_size).ws_row + 1;}

    /* Horizontal scrolling */
    if (old_config.render_x < old_config.column_offset) {
        // this checks if the cursor is at the left visibale window
        old_config.column_offset = old_config.render_x;}
    else if (old_config.render_x >= (old_config.window_size).ws_col + old_config.column_offset) { 
        // this checks if the cursor is at the right visibale window
        old_config.column_offset = old_config.render_x - (old_config.window_size).ws_col + 1;}
}

void valid_row(plain_row **row, terminal_configurations old_config){
    if (old_config.cursor_y <= old_config.nrows - 1){ //  since cursor_y starts out from 0 we substract 1 from nrows
        // In this case our cursor is at worst at the bottom of the file, so we can give the user the permission 
        // to go one line down to write somthing
        int cursor_y = old_config.cursor_y;
        *row = &old_config.editor_row[cursor_y];
    }

}
////////////////////////////////////////////////////////////////
void move_cursor(int direction){

    plain_row *row = NULL;

    valid_row(&row, old_config);

    if (direction == ARROW_UP) {
        if (old_config.cursor_y > 0){
            old_config.cursor_y--;}
            }

    else if (direction == ARROW_DOWN) {
    if (old_config.cursor_y < old_config.nrows){ // we'll be comparing our y to nrows instaed of windowsize
        old_config.cursor_y++;
      }}

    else if (direction == ARROW_RIGHT) {
        // 
        /* if (old_config.cursor_x < old_config.window_size.ws_col - 1) */
        if (row && old_config.cursor_x <= row->row_size - 1){
            old_config.cursor_x++;}
        
        else if(old_config.cursor_x == old_config.editor_row[old_config.cursor_y].row_size){
            if (old_config.cursor_y < old_config.nrows){
                old_config.cursor_x = 0;
                old_config.cursor_y++;
            }
        
        }
    }
    else if (direction == ARROW_LEFT) {
        if (old_config.cursor_x > 0){
            old_config.cursor_x--;}
        
        else if (old_config.cursor_x == 0)
        {
            if (old_config.cursor_y >0){
                /* old_config.cursor_x = old_config.editor_row->row_size; */
                //editor row contain aall visible rows ?
                old_config.cursor_y--;
                old_config.cursor_x = old_config.editor_row[old_config.cursor_y].row_size; 
            }
        }
    }

    else if (direction == PAGE_UP){
        
        old_config.cursor_y = old_config.row_offset;
        for(int i= old_config.nrows;i>0;i--){
            move_cursor(ARROW_UP);
        }
    }
    else if (direction == PAGE_DOWN){
       
        old_config.cursor_y  =old_config.cursor_y<old_config.nrows?get_window_size().ws_row -1 + old_config.row_offset:old_config.nrows;
            
        for(int i= old_config.nrows;i>0;i--){
            move_cursor(ARROW_DOWN);
        }
        
    }
    else if  (direction == END_KEY){//move to the edge right
        old_config.cursor_x =  old_config.cursor_y<old_config.nrows?old_config.editor_row[old_config.cursor_y].row_size:old_config.cursor_x;
            
    }
    else if (direction == HOME_KEY){//move to the far left
        while (old_config.cursor_x >0){
            old_config.cursor_x--;
        }
    }

    valid_row(&row,old_config);
    int row_length;
    if (row){
        row_length = row->row_size;
    }
    else{
        row_length = 0;
    }
    if (old_config.cursor_x > row_length) {
        old_config.cursor_x = row_length;
}}

////////////////////////////////////////////////////////////////////////////////////////////

void draw_rows(int ws_row, int ws_col, text_buffer* tildes_buff) { 
    char welcome_buff[64];
   
    for (int i = 0; i < ws_row  ; i++) {
        int file_row = i + old_config.row_offset ;
        if (old_config.nrows <= file_row) { //past the file rows
            if (old_config.nrows == 0 && i == old_config.window_size.ws_row / 3) {
                /* write(STDOUT_FILENO, "~\r\n", 3) ; */
                /// Welcome message ///
                
                int big_welcome = snprintf(welcome_buff, sizeof(welcome_buff),
                                           "BEE text editor -- Version %s", BEE_version);
                
                append_buffer(tildes_buff, "~", 1);

                if (big_welcome > ws_col) {
                    append_buffer(tildes_buff, welcome_buff, ws_col);
                } 
                else {
                    int center = (ws_col - big_welcome) / 2 - 1; // The - 1 is to take account for ~
                    while (center != 0) {
                        append_buffer(tildes_buff, " ", 1);
                        center--;
                    }
                    append_buffer(tildes_buff, welcome_buff, big_welcome);
                }
            } 
            else {
                append_buffer(tildes_buff, "~", 1);
            }
        } 
        else {
            int length_row = old_config.editor_row[file_row].ren_size - old_config.column_offset;// This will store the size of what is on the left of the offset
            if (length_row < 0){ length_row = 0;} // length might be negative as the offset could be bigger than the row_size 
                                                    // (offest = 5 for an empty row for example)
            if (length_row > old_config.window_size.ws_col) {
                length_row = old_config.window_size.ws_col;
            }

            int column_offset = old_config.column_offset;

            char *beyond_offset = &old_config.editor_row[file_row].render[column_offset]; //it needs to be a pointer (&) as it'll be an argument for isdigit
            unsigned char *highlights_array = &old_config.editor_row[file_row].highlight[old_config.column_offset]; // the nth highlight corresponds to the nth character in render
            int current_color = -1; //keeping track of color so we don't have to write down an esc each time

            for(int j = 0; j < length_row; j++){
                 if (iscntrl(beyond_offset[j])){
                    append_buffer(tildes_buff, "\x1b[7m", 4);
                    char ordre = 0;
                    if (beyond_offset[j] <= 26  /* in alphabetical range */ ){ // remider that our key are ints now so each letter (A for example) is now presented by its order (26 for A)
                        ordre = '@' + beyond_offset[j];
                        if (beyond_offset[j] != 0){ // 0 is ASCII for '\0'
                            append_buffer(tildes_buff, &ordre, 1);}}  

                    else {
                        append_buffer(tildes_buff, "?", 1); // "?" is a pointer to '?' 
                    }

                    append_buffer(tildes_buff, "\x1b[m", 3);

                    if (current_color != -1){// restoring colors after managing ctrl keys
                        char buffer[16];
                        int color_lenght = snprintf(buffer, sizeof(buffer),"\x1b[%dm", current_color);
                        append_buffer(tildes_buff,buffer,color_lenght);}
                    }
                
                else if (highlights_array[j] == HL_DIGITS || highlights_array[j] == HL_MATCH || highlights_array[j] == HL_COMMENT || 
                    highlights_array[j] == HL_STRING || highlights_array[j] == HL_KEYWORD1 || highlights_array[j] == HL_KEYWORD2 || highlights_array[j] == HL_MLCOMMENT){
                    int color = color_syntax(highlights_array[j]);
                    if (color != current_color){
                    current_color = color;
                    char buffer[16];
                    int color_lenght = snprintf(buffer, sizeof(buffer),"\x1b[%dm", color);
                    append_buffer(tildes_buff,buffer,color_lenght);
                    
                    }
                append_buffer(tildes_buff,&beyond_offset[j],1); //&beyond_offset[j] being one digit we have to call its pointer since append_buffer takes char* and cahr for the 2e arg
                }
                else if(highlights_array[j] == HL_NORMAL){
                    if(current_color != -1){
                        append_buffer(tildes_buff,"\x1b[39m",5);
                        current_color = -1;} //we'll only have to write an esc if the previous char was a digit
                    
                    append_buffer(tildes_buff,&beyond_offset[j],1);}
            
            }}
        append_buffer(tildes_buff, "\x1b[39m", 5);
        append_buffer(tildes_buff, "\x1b[K", 3);  // Clears the row from cursor to end
        append_buffer(tildes_buff, "\r\n", 2);
    }
   /*  append_buffer(tildes_buff, "~", 1); */
   /*  write(STDOUT_FILENO, "~", 1) ;
    write(STDOUT_FILENO, "\x1b[1;1H", 6) ; */
}

void draw_status_bar(text_buffer *status_buff){
    // first let's invert colors
    int n_columns = old_config.window_size.ws_col;
    append_buffer(status_buff, "\x1b[7m", 4);

    char status_text[32];
    char current_line[32];

    int status_text_length = 0;
    int current_line_text_length = 0;

    current_line_text_length = snprintf(current_line, sizeof(current_line), "%d/%d", old_config.cursor_y + 1, old_config.nrows);

    if (old_config.file_name){
        status_text_length = snprintf(status_text, sizeof(status_text), "%.20s - %d lines",old_config.file_name ,old_config.nrows);
    }
    else{ status_text_length = snprintf(status_text, sizeof(status_text), "%.20s - %d lines","[No Name]" , old_config.nrows); }
    
    if (status_text_length > n_columns) {status_text_length = n_columns;}


    // now let's draw the status bar
    append_buffer(status_buff, status_text, status_text_length);

    int first_info_len = 0;

    if (old_config.dirty != 0){
        append_buffer(status_buff, " (modified)", 12);
        first_info_len = status_text_length + 12;
        }
    else{
        first_info_len = status_text_length;
    }


    char file_type[16] ;
    if (old_config.syntax != NULL) {
        strncpy(file_type, old_config.syntax->file_type, sizeof(file_type)) ;
        file_type[sizeof(file_type) - 1] = '\0' ; 
    } else {
        strncpy(file_type, "no ft", sizeof(file_type)) ;
        file_type[sizeof(file_type) - 1] = '\0' ;
    }

    current_line_text_length = snprintf(current_line, sizeof(current_line), "%s | %d/%d", file_type,
                                old_config.cursor_y + 1, old_config.nrows) ;

    for (int i = first_info_len; i < n_columns; i++){
        if (n_columns - i == current_line_text_length ){
            append_buffer(status_buff, current_line, current_line_text_length);
            break;
        }
        else {
            append_buffer(status_buff, " ", 1);}
    }
    //finally we recover default parameters for colors
    append_buffer(status_buff,"\x1b[m", 3);
    append_buffer(status_buff, "\r\n", 2); //making place for our status message
}

void draw_message_bar(text_buffer *message_buff){
    int n_columns = old_config.window_size.ws_col;
    append_buffer(message_buff, "\x1b[K", 3);
    int message_length = strlen(old_config.status_message);
    if ( message_length > n_columns ){
        message_length = n_columns;}
    if( message_length && time(NULL) - old_config.status_message_time < 5 ){ /* our timeout will be 5s*/
        append_buffer(message_buff, old_config.status_message,message_length);}
    }


void clear_screen(int ws_row, int ws_col){
    text_buffer initializing_screen = {NULL,0};

    scroll(); // we must position the cursor well before reseting the screen
    // we'll hide the cursor before drawing the tildes
    append_buffer(&initializing_screen,"\x1b[?25l", 6);//?25: hide cursor; l: lowercase "L" -> disable (cursor)

    //we'll replace this clearing command with a line by line one in draw_rows
    /* append_buffer(&initializing_screen,"\x1b[2J", 4); */ // J command: clear the screen ; 2: all of the screen
    append_buffer(&initializing_screen,"\x1b[1;1H", 6); // H command : cursor position
    /* write(STDOUT_FILENO, "\x1b[2J", 4) ;
    write(STDOUT_FILENO, "\x1b[1;1H", 6) ; // The values by default are 1;1 so we could've simply written \x1b[ whic
                                           // would only take 3 bytes */

    /////////////////////////////// Drawing /////////////////////////////////////////
    draw_rows(ws_row, ws_col, &initializing_screen);

    // now that we are exactly at the bottom of the file we'll draw our status bar

    draw_status_bar(&initializing_screen);

    // now we draw the message bar

    draw_message_bar(&initializing_screen);

    /////////////////////////////////////////////////////////////////////////////////

    // here we'll be positioning the cursor as indicated by the coordinates
    char cursor_at_will[32];
    // now that our y coordinate refers to the cursor's vertical position in the file ( and not the screen) to reposition it 
    // to the top of the screen we'll have to subtract olf_config.row_offset from it, same goes with the x coordinate
    snprintf(cursor_at_will, sizeof(cursor_at_will), "\x1b[%d;%dH",(old_config.cursor_y - old_config.row_offset) + 1,(old_config.cursor_x - old_config.column_offset)+ 1);  

    //once we're done drawing the tildes we recover the cursor;
    append_buffer(&initializing_screen,"\x1b[?25h", 6);
    append_buffer(&initializing_screen, cursor_at_will, strlen(cursor_at_will));

    write(STDOUT_FILENO, (&initializing_screen)->text, (&initializing_screen)->length); //We could've simply written 
                                                                                        //write(STDOUT_FILENO, initializing_screen.text, initializing_screen.length)
                                                                                        //but we wanted to keep a similar structure with draw as this part was previously
                                                                                        //in draw_rows
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


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* Insert the char c at the position idx of the given row 
(I used int here as key in read_one_key() has type int)*/

void insert_char_in_a_row(plain_row* row, int idx, int c) {
    /* idx not valid => char inserted at the end of the row */
    if (idx < 0 || idx > row->row_size) {
        idx = row->row_size ; 
    }
    /* Reallocate the memory for "row_data" to support the insertion of the new char */
    row->row_data = realloc(row->row_data, row->row_size + 2) ; /* 1 for c and 1 for \0 */
    /* Copy all the characters until idx of the old row in the reallocated "row_data" */
    memmove(&row->row_data[idx + 1], &row->row_data[idx], row->row_size - idx + 1) ;
    /* Increment the size of the row and inserting c properly at the posiiton idx */
    row->row_size += 1 ; 
    row->row_data[idx] = c ;
    /* Updating the row following the previous changements */
    update_row(row) ;

}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* Insert the char in the text editor */

void insert_char_in_the_editor(int c) {
    /* Cursor coordinates */
    int cx = old_config.cursor_x ;
    int cy = old_config.cursor_y ;
    /* Add a new (empty) row if cursor at the end of file */
    if (cy == old_config.nrows) {
        insert_row(old_config.nrows,"", 0) ;
    }
    /* Insert the char at the cursor position */
    insert_char_in_a_row(&old_config.editor_row[cy], cx, c) ;
    /* Increment the cursor position horizontally after the insertion of c */
    old_config.cursor_x += 1 ; 
}
void new_line_insert(){
    if (old_config.cursor_x == 0){insert_row(old_config.cursor_y,"",0);}
    else{
        plain_row *row =  &old_config.editor_row[old_config.cursor_y];
        insert_row(old_config.cursor_y+1,&row->row_data[old_config.cursor_x],row->row_size-old_config.cursor_x);
        row = &old_config.editor_row[old_config.cursor_y];
        row->row_size = old_config.cursor_x;
        row->row_data[row->row_size] = '\0';
        update_row(row);
    }
    old_config.cursor_y++;
    old_config.cursor_x = 0;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* Convert all the rows in the editor to a single string
 => useful later to save the txt file edited 
 The length of the string has to be memorized for ulterior use, thus the pointer */

char* rows_to_str(int* str_len) {
    /* Get the total length of the final string */
    int len = 0 ;
    for (int i = 0 ; i < old_config.nrows ; i++) {
        len += old_config.editor_row[i].row_size + 1 ; /* +1 here for \n */
    }
    *str_len = len ;

    /* Copy each row in the string, separating each one by the \n character */
    char* str = malloc(*str_len) ;
    char* ptr = str ;       /* this pointer will be used to move through the rows, so
                            str doesn't loose its starting adress in the memory and so its data */

    for (int i = 0 ; i < old_config.nrows ; i++) {
        /* Copy all the bytes of the current row to the memory block at which points ptr */
        memcpy(ptr, old_config.editor_row[i].row_data, old_config.editor_row[i].row_size) ;
        /* Advance ptr to point at the end of the copied (ie converted) row */
        ptr += old_config.editor_row[i].row_size ;    
        /* Add \n to separate the copied row from the next one */
        *ptr = '\n' ;
        ptr ++ ;
    }
    return str ;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* Save the file if modified after editing with BEE */
// save as
char* Prompt(char* pr, void (*callback)(char* ,int)){
    size_t size_cap = 128;
    char *buffer = malloc(size_cap);

    size_t len = 0;
    buffer[0] = '\0';
    while(1){//repeatedly update the status message and refresh the screen
        set_status_message(pr,buffer);
        clear_screen(old_config.window_size.ws_row, old_config.window_size.ws_col);

        int c = read_one_key();
        if (c==DELETE_KEY ||c== CTRL_KEY('h') || c == BACKSPACE){
            if (len !=0){buffer[--len] = '\0';}
        }
        else if (c == '\x1b'){//Escape character
            set_status_message("");
            if (callback) {callback(buffer, c);}
            free(buffer);
            return NULL;//we don't want to save now
        }
        else if (c == '\r'){//Press Enter to save
            if (len != 0){
                set_status_message("");
                if (callback) {callback(buffer, c);}
                return buffer;
            }
        }
        else if(!iscntrl(c) && c<128){
            if (len == size_cap -1){
                size_cap *=2;
                buffer = realloc(buffer,size_cap);
            }//allocating more when we go above the capacity
            buffer[len++] = c;
            buffer[len] = '\0';
      
        }
        if (callback) {callback(buffer, c);}
    }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void save_edited() {
    
    if (old_config.file_name == NULL) {
        /* It has to be treated later on */
        old_config.file_name = Prompt("Save as: %s (Press Esc to cancel save request)", NULL);
        if (old_config.file_name == NULL){//empty save message
            set_status_message("Could not save");
            return;
        }
        syntax_highlight(); // must be called each time we save a file
    }

    /* Convert all rows into one string */
    int len ; 
    char* str = rows_to_str(&len) ;

    /* Open the file_name file for reading and writing or creating it 
    if it doesn't exist
    0644 (standard permission) : owner can read/wrtie | others can only read */
    int f = open(old_config.file_name, O_RDWR | O_CREAT, 0644) ;
    if (f != -1) {
        if (ftruncate(f, len) != -1) {    /* sets the file’s size to the specified length */  
            if (write(f, str, len) == len) {
                close(f) ;
                free(str) ;
                // if we get to here it surely means the saving process succeeded
                set_status_message("Saving successed, %d bytes were written to disk", len);
                return;
            }
        }
        close(f) ;
    }
    // if we get to here it surely means the saving process failed
                set_status_message("Saving failed! %s", strerror(errno));
    free(str) ;
   
    // Reset old_config.dirty to 0 once the file is saved
    old_config.dirty = 0;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* Convert an rx into a cx */

int rx_to_cx(plain_row* row, int rx) {
    int current_rx = 0 ;
    int cx ; 
    for (cx = 0 ; cx < row->row_size ; cx++) {
        if (row->row_data[cx] == '\t') {
            current_rx += (BEE_TAB_STOP - 1) - (current_rx % BEE_TAB_STOP) ;
        }
            current_rx += 1 ;
            if (current_rx > rx) {
                return cx ;
            }
    }
    return cx ;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* Search feature */ 
void search_query_callback(char *query, int key){

    static int last_row = -1; // -1 for no previous match
    static int last_col = -1;
    static int direction = 1; // 1 for searching forward, -1 for searching backward

    /* Restore hl to its previous value after each search */
    static int saved_hl_line ;
    static char *saved_hl = NULL ;
    if (saved_hl) { 
        memcpy(old_config.editor_row[saved_hl_line].highlight, saved_hl, old_config.editor_row[saved_hl_line].ren_size) ;
        free(saved_hl) ;
        saved_hl = NULL ;
    }
    if (key == '\r' || key == '\x1b' ){
        last_row = -1; // -1 for no previous match
        last_col = - 1;
        direction = 1; // 1 for searching forward, -1 for searching backward
        return;}

    else if (key == ARROW_DOWN || key == ARROW_RIGHT){
        direction = 1;
    }

    else if (key == ARROW_UP || key == ARROW_LEFT){
        direction = -1;
    }

    else{
        last_row = -1;
        last_col = -1;
        direction = 1;
    }

    if (last_row == -1 && last_col == -1){ 
        direction = 1;} //we can't go backwards if there is nothing in there
    
    int current_row = last_row;
    int current_col = last_col;


    for (int i = 0 ; i < old_config.nrows ; i++) {

        if (current_row < 0){current_row = old_config.nrows - 1;}// going backwards while at the beggining of file = going to EOF

        else if (current_row >= old_config.nrows){current_row = 0;} // going forward while at the EOF = going to the beggining of file

        /* Get the current_row row */
        plain_row *row = &old_config.editor_row[current_row] ; // 
        /* Check if query is a substring of the current_row row */
        char *match_start = row->render;

        if (current_row == last_row && current_col != -1){ match_start += current_col + 1;}

        char *match = strstr(match_start, query);

        if (match) {

            last_row = current_row;
            last_col = match - row->render;

            old_config.cursor_y = current_row;
            /* Go to the match itself within the line */
            old_config.cursor_x = rx_to_cx (row, match - row->render) ;
            old_config.row_offset = old_config.nrows ; 

            saved_hl_line = current_row ;
            saved_hl = malloc(row->ren_size) ;
            memcpy(saved_hl, row->highlight, row->ren_size) ;
            memset(&row->highlight[match - row->render], HL_MATCH, strlen(query)) ; 
            return ;
    }
        //no more matches in this row
        current_row += direction;
        current_col = -1;
  }
}  

 
 
void search_query() {
  int pre_cursor_x = old_config.cursor_x;
  int pre_cursor_y = old_config.cursor_y;
  int pre_row_offset = old_config.row_offset;
  int pre_column_offset = old_config.column_offset;

  char *query = Prompt("Search: %s (ESC to cancel)",search_query_callback) ;
  if (query) {
        free(query) ; 
  }

  else { // if query is NULL, then the saving process was cancelled
    old_config.cursor_x = pre_cursor_x;
    old_config.cursor_y = pre_cursor_y;
    old_config.row_offset = pre_row_offset;
    old_config.column_offset = pre_column_offset;
  }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


void key_process() {
    int key = read_one_key();

    static int quit_times = QUIT_CONFIRMATION; //quit_times has to be static to retain its value after each call

    if (key == CTRL_KEY('q')) {
        if (old_config.dirty != 0 && quit_times > 0){
            set_status_message("Warning!!! %d changes were applied. To quit press Ctrl-Q %d more times.", old_config.dirty, quit_times);
            quit_times --;
            return;
        }
        write(STDOUT_FILENO, "\x1b[2J", 4);
        write(STDOUT_FILENO, "\x1b[1;1H", 6);
        exit(EXIT_SUCCESS);  // Exit the program
    } 
    else if (key == '\r'){
        new_line_insert();
    }



    else if (key == ARROW_UP || key == ARROW_DOWN || 
            key == ARROW_LEFT || key == ARROW_RIGHT ||
            key == PAGE_UP   || key == PAGE_DOWN   ||
            key == HOME_KEY  || key == END_KEY  ) {
        move_cursor(key);
    }

    else if (key == CTRL_KEY('s')) {
        save_edited() ;
    }

    else if (key == '\r') {
        /* TODO */
    }

    else if (key == BACKSPACE || key == CTRL_KEY('h') 
            || key == DELETE_KEY) {
        if (key == DELETE_KEY){
            move_cursor(ARROW_RIGHT);}
        delete_char();
    } 

    else if (key == CTRL_KEY('l') || key == '\x1b') {
        /* TODO */
    }

    else if (key == CTRL_KEY('f')) {
        search_query() ;
    }

    /* If no special key is used then it's a character to insert in the editor */
    else {
        insert_char_in_the_editor(key) ;
    }

    // Reset quit_times
    quit_times = QUIT_CONFIRMATION;
}
/* ++++++++++++++++++++++ Deleting operators ++++++++++++++++++++++++ */
void delete_char_from_row(plain_row* row, int position){
    if (position < 0 || position >= row->ren_size){ return ;} 
    else{
        memmove(&row->row_data[position], &row->row_data[position + 1], row->ren_size - position);
        row->ren_size -= 1;
        update_row(row);
    }
}
void free_row(plain_row *erow){
    free(erow->render);
    free(erow->row_data);
    free(erow->highlight);
}

void delete_row(int row_index){//appending the content of a line to the previous if we press DEL_KEY
    if (row_index<0 || row_index> old_config.nrows){return;}
    free_row(&old_config.editor_row[row_index]);
    memmove(&old_config.editor_row[row_index],&old_config.editor_row[row_index +1],sizeof(plain_row)*(old_config.nrows - 1 - row_index));

    //writing there the rest of the row
    old_config.nrows--;
    old_config.dirty++;

    for (int i = row_index; i < old_config.nrows; i++){
        old_config.editor_row[i].idx -= 1; 
    }
}

void row_append_str(plain_row* erow,char *str,size_t len){
    erow->row_data = realloc(erow->row_data,erow->row_size + len + 1);
    //réallocation pour l'ajout de string et '\0'
    memcpy(&erow->row_data[erow->row_size],str,len);
    erow->row_size += len;
    erow->row_data[erow->row_size] = '\0';
    update_row(erow);
    old_config.dirty++;

}
void delete_char(){
    int cursor_y = old_config.cursor_y;
    int cursor_x = old_config.cursor_x;

    if (cursor_y == old_config.nrows ){ // the cursor is past the EOF, there is nothing to delete
        return;
    }
    if (cursor_x ==0 && cursor_y ==0){return;}

    plain_row* row = &old_config.editor_row[cursor_y];
    if (cursor_x > 0){ // if cursor_x is 0 we're at the beggining of the row, there is nothing to delete
        delete_char_from_row(row, old_config.cursor_x - 1);
        old_config.cursor_x -= 1;
    }
    else{//sûr que y !=0
        old_config.cursor_x = old_config.editor_row[old_config.cursor_y -1].row_size;
        row_append_str(&old_config.editor_row[old_config.cursor_y-1],row->row_data,row->row_size);
        delete_row(old_config.cursor_y);
        old_config.cursor_y--;
    }
}
/* +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */


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


/////////////////////////////////////////////////////////////////////
void update_row(plain_row* row){
    int num_tabs = 0;
    for(int j=0; j<row->ren_size; j++){
        if( row->row_data[j] == '\t'){
            num_tabs++;
        }
    }
    free(row->render);
    row->render = malloc(row->row_size + (BEE_TAB_STOP-1)*num_tabs+1);//maximum d'espace ajouté par tab est 7 par défaut
    //right now we'll only copy the content
    int idx_render = 0;
    //we use two indices because later we will use that for differentiating render and original indices.                                                                                                                                                                                   
    for(int i=0;i<row->row_size;i++){
        if (row->row_data[i] == '\t'){
            row->render[idx_render++] = ' ';
            while(idx_render %BEE_TAB_STOP != 0){row->render[idx_render++]= ' ';}
        }
        else {row->render[idx_render++] = row->row_data[i];}
    }
    
    row->render[idx_render] = '\0';
    row->ren_size = idx_render;
    // once we update a row we mark that some changes were done
    old_config.dirty += 1;

    update_syntax(row);
}


void insert_row(int idx, char* opening_line, ssize_t len) {
    if (idx<0 || idx> old_config.nrows){return;}

    old_config.editor_row = realloc(old_config.editor_row,sizeof(plain_row) * (old_config.nrows + 1));
    /* To support multiple lines storage into the buffer */
    memcpy(&old_config.editor_row[idx + 1],&old_config.editor_row[idx],sizeof(plain_row) * (old_config.nrows - idx)); /* + 1 for the new line */
    for (int i = idx + 1;  i < old_config.nrows; i++){
        old_config.editor_row[i].idx += 1;
    }
    /* Index of the new line to store */
    old_config.editor_row[idx].idx = idx;
    old_config.editor_row[idx].hl_open_comment = 0 ; 

    old_config.editor_row[idx].row_size = len;
    old_config.editor_row[idx].row_data  = malloc(len + 1);
    memcpy(old_config.editor_row[idx].row_data, opening_line, len);
    old_config.editor_row[idx].row_data[len] = '\0';

    /* Increment the number of rows as a new line has been stored */
   /*  old_config.nrows += 1; */

    old_config.editor_row[idx].ren_size =0;
    old_config.editor_row[idx].render = NULL;
    old_config.editor_row[idx].highlight = NULL;
    update_row(&old_config.editor_row[idx]);
    old_config.nrows += 1;

}

//////////////////////////////////////////////////////////////////////


void open_editor(char* filename){
    FILE* fptr = fopen(filename,"r");
    if(fptr ==NULL){kill("fopen doesn't work\n");};

   
    
    // save a copy of the file's name in our configurations structure after getting rid of previous memory 
    free(old_config.file_name);
    /* old_config.file_name = filename; */ // turns out you can't just copy it this way as it an argv[1], thus it
                                            // only has a temporary pointer assigned to it which will become invalid
                                            //once the program is executed

    // we'll allocate some memory to do so
    char *filename_copy = malloc(strlen(filename) + 1); // the + 1 is for '\0'  
    strcpy(filename_copy, filename);
    old_config.file_name = filename_copy;

    syntax_highlight(); //gather infos about the file
    
    
    char *opening_line = NULL; 
    size_t cap = 0;
    ssize_t len; 
    /* While loop to read / store all the lines of the file */
    while ((len = getline(&opening_line,&cap,fptr)) != -1) {
        while (len > 0 && (opening_line[len - 1] == '\n' || opening_line[len - 1] == '\r')) {
            len--;
        }
        insert_row(old_config.nrows,opening_line, len) ;
    }
    free(opening_line);
    fclose(fptr) ;      /* Close the file rather than free the pointer (which is not correct) */

    // Reset old_config.dirty to 0 since insert_row calls update_raw which changes its value to 1 the moment the file opens
    old_config.dirty = 0;
}

void set_status_message(const char *format, ...){ // format : %s %d etc
    va_list variable_list;
    va_start(variable_list, format); // start reading variables comming after format
    vsnprintf(old_config.status_message, sizeof(old_config.status_message), format, variable_list);
    va_end(variable_list);
    old_config.status_message_time = time(NULL); // returns the time that has past since the Unix epoch
}

void intialize_editor(){
    // initializing row and column offset 
    old_config.row_offset = 0;
    old_config.column_offset = 0;
    // initializing cursor position
    old_config.cursor_x = 0;
    old_config.cursor_y = 0;
    old_config.render_x = 0;
    // initializing the file's number of rows
    old_config.nrows = 0;
    // intializing our editor's editor_row with a NULL type for later modifications
    old_config.editor_row = NULL ;
    // Avoiding to draw an additional tild (~)
    old_config.window_size.ws_row -= 2;// making place for our status section (with the status message)
    // initializing file name
    old_config.file_name = NULL;
    // Initializing status message and timeout
    old_config.status_message[0] = '\0';
    old_config.status_message_time= 0;
    // initializing the dirty boolean
    old_config.dirty = 0;
    old_config.syntax = NULL ; 
    
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int is_separator(int c) {
    /* strchr looks for the first occurrence of a character in a 
    string, and returns a pointer to the matching character in the string */
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL ;
}


/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void update_syntax(plain_row *row){
    row->highlight = realloc(row->highlight, row->ren_size);// we reallocate memory since the row may have gotten longer or it is an entirely new row
    memset(row->highlight, HL_NORMAL, row->ren_size); // memset will make sure we only pass unsigned chars

    if (old_config.syntax == NULL) {
        return ;
    }
    char **keywords = old_config.syntax->keywords;
    /* the beginning of the line is considired as a separator. */
    char *sl_comm_start = old_config.syntax->comment_start_single_line;
    char *ml_comm_start = old_config.syntax->comment_start_multi_lines;
    char *ml_comm_end = old_config.syntax->comment_end_multi_lines;
    
    int sl_comm_len = 0;
    int mls_comm_len = 0;
    int mle_comm_len = 0;

    if (sl_comm_start){
        sl_comm_len = strlen(sl_comm_start);
    }
    
    if (ml_comm_start){
        mls_comm_len = strlen(ml_comm_start) ;
    }

    if (ml_comm_end){
        mle_comm_len = strlen(ml_comm_end);
    }

    int previous_separator = 1; 
    int quote_string = 0;
    int i = 0 ; 
    int in_comment = 0;
    if (row->idx > 0) {
        in_comment = old_config.editor_row[row->idx - 1].hl_open_comment ;
    }
    

    while (i < row->row_size) {
        unsigned char previous_hl ;
        if (i > 0) {
            previous_hl = row->highlight[i - 1] ;
        }
        else {
            previous_hl = HL_NORMAL ;
        }
        //comments
        if(sl_comm_len && !quote_string && !in_comment){//if there is a comment and is not a string
            if (!strncmp(&row->render[i], sl_comm_start, sl_comm_len)){
                memset(&row->highlight[i], HL_COMMENT, row->ren_size - i);
                break;
            }
        }
        if (mls_comm_len && mle_comm_len && !quote_string){
            int cmp = 0;
            if (in_comment) {
                /* highlight the current character */
                row->highlight[i] = HL_MLCOMMENT ;
                /* check if we're at the end of a multi-line comment */
                cmp = strncmp(&row->render[i], ml_comm_end, mle_comm_len) ;
                if (cmp == 0) {
                    /* we're at the end of the ml-comment  */ 
                    memset(&row->highlight[i], HL_MLCOMMENT, mle_comm_len) ;
                    i += mle_comm_len ;
                    in_comment = 0;
                    previous_separator = 1;
                    continue;
                }
                else {
                    /* we're not at the end of comment => consume the current character */

                    i += 1 ;
                    continue;
                }
            }
            else {
                cmp = strncmp(&row->render[i], ml_comm_start, mls_comm_len) ;
                if (cmp == 0) {
                    memset(&row->highlight[i], HL_MLCOMMENT, mls_comm_len);
                    in_comment = 1;
                    i += mls_comm_len;
                    continue;
                }
            }
        }
        
        if (old_config.syntax->flags & HIGHLIGHT_STRINGS){
            if(quote_string){
                row->highlight[i] = HL_STRING;
                if (row->render[i] == '\\' && i+1 < row->ren_size){
                    row->highlight[i] = HL_STRING;
                    i++;
                    i++;
                    continue;;
                }
                if (row->render[i] == quote_string){
                    quote_string = 0;
                }
                i++;
                previous_separator = 1;
                continue;
           

            }
            else{
                if (row->render[i] == '"' || row->render[i] == '\''){
                    quote_string = row->render[i];
                    row->highlight[i] = HL_STRING;
                    i++;
                    continue;
                }
            }
        }
       
        if (old_config.syntax->flags & HIGHLIGHT_NUMBERS) { // HIGHLIGHT_NUMBES = 1 so if it is activated, old_config.syntax->flags = 1, and 1 & 1 = 1 (True)
            if (isdigit(row->render[i]) && (previous_separator || previous_hl == HL_DIGITS
                || ((row->render[i] == '.') && (previous_hl == HL_DIGITS)))){
                row->highlight[i] = HL_DIGITS;
                i += 1 ; 
                previous_separator = 0 ; 
                continue ; 
            }
        }

        //keywords
        if (previous_separator){//a separator came before the keyword
            int j;
            for (j=0;keywords[j];j++){
                int key_len = strlen(keywords[j]);
                int key_word2 = (keywords[j][key_len -1] == '|');
                if (key_word2) key_len--;

                if (!strncmp(&row->render[i],keywords[j],key_len) &&
            is_separator(row->render[i + key_len])){
                if( key_word2){
                    memset(&row->highlight[i],HL_KEYWORD2,key_len);
                }
                else{
                    memset(&row->highlight[i],HL_KEYWORD1,key_len);
                }
                i += key_len;
                break;
            }
        }
            if(keywords[j] != NULL){
                previous_separator = 0;
                continue;
            
            }
        }
        previous_separator = is_separator(row->render[i]) ;
        i += 1 ; 
    }
    if (row->hl_open_comment != in_comment) {
        row->hl_open_comment = in_comment;
        /* hl_open_comment did change => update the next line if there is one left */
        if (row->idx + 1 < old_config.nrows) {
            update_syntax(&old_config.editor_row[row->idx + 1]) ;
        }
    }
}

int color_syntax(int highlight){

    if (highlight == HL_DIGITS){ return 31;}
    else if (highlight == HL_MATCH) {return 34 ;} /* blue */
    else if (highlight == HL_STRING){ return 35;}
    else if (highlight == HL_COMMENT || highlight == HL_MLCOMMENT){ return 36;}//CYAN FOR COMMENTS
    else if (highlight == HL_KEYWORD1){ return 33;}//yellow
    else if (highlight == HL_KEYWORD2){ return 32;}//green
    else { return 37;}
}

void syntax_highlight(){
    old_config.syntax = NULL; //reseting syntax highlighting
    if (old_config.file_name == NULL){ return;}

    char* filename = old_config.file_name;
    char* extension = strrchr(filename, '.');
    for (unsigned int i = 0; i < HL_DATABASE_ENTRIES; i ++){ // we might have many dtat bases
        Syntax *syntax = &HL_Database[i];

        /* int extensions_len = sizeof(syntax->file_match)/sizeof(syntax->file_match[0]);
        for (int j = 0; j < extensions_len; j++){ // here syntax is a pointer so it shouldn't matter to how many elts it points
                                                  // since a pointer is always the same size, unlike Syntax HL_Database[] which was declared as an array
 */ 
        for (int j =  0; syntax->file_match[j] /* Breaking cdt is syntax->file_match == NULL */; j++){
            if (extension == NULL){ // file has no extension, but is a full filename (like Makefile)
                if( strcmp(filename, syntax->file_match[j]) == 0 ){ // I'm using strcmp and not strstr here because Makefile_test for example isn't recognized as Makefile
                    old_config.syntax = syntax;
                    // Once we save a previously none existing file we must update all highlights (not just the extension)
                    for (int file_row = 0; file_row < old_config.nrows; file_row ++){
                        update_syntax(&old_config.editor_row[file_row]);
                    }}
            }
            else{
                if (strcmp(extension , syntax->file_match[j]) == 0){ 
                    old_config.syntax = syntax;
                     // Once we save a previously none existing file we must update all highlights (not just the extension)
                    for (int file_row = 0; file_row < old_config.nrows; file_row ++){
                        update_syntax(&old_config.editor_row[file_row]);
                    }}
            }
        }   
    }
}
/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int main(int argc, char** argv) {

    struct termios new_settings; 
    // i'll extract the window size before calling initialize_editor as it modifies it
    old_config.window_size = get_window_size(); 
    intialize_editor();
    
    // Récupérer les anciens réglages avant toute modification
    if (tcgetattr(STDIN_FILENO, &old_config.old_settings) == -1) {
        kill("Could not get terminal attributes\n");
    }

    new_settings = old_config.old_settings;
    enable_raw_mode(&new_settings);  

    if (argc >1){
        open_editor(argv[1]);
    }

    set_status_message("HELP: Ctrl-S = save | Ctrl-Q = quit | Ctrl-F = find") ;
    
    /* old_config.nrows--; */
    while (1) {
        clear_screen(old_config.window_size.ws_row, old_config.window_size.ws_col);
        key_process();
    }

    return EXIT_SUCCESS;
}

