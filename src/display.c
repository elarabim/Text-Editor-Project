#include "../include/display.h"



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



void set_status_message(const char *format, ...){ // format : %s %d etc
    va_list variable_list;
    va_start(variable_list, format); // start reading variables comming after format
    vsnprintf(old_config.status_message, sizeof(old_config.status_message), format, variable_list);
    va_end(variable_list);
    old_config.status_message_time = time(NULL); // returns the time that has past since the Unix epoch
}