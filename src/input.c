#include "../include/input.h"


void valid_row(plain_row **row, terminal_configurations old_config){
    if (old_config.cursor_y <= old_config.nrows - 1){ //  since cursor_y starts out from 0 we substract 1 from nrows
        // In this case our cursor is at worst at the bottom of the file, so we can give the user the permission 
        // to go one line down to write somthing
        int cursor_y = old_config.cursor_y;
        *row = &old_config.editor_row[cursor_y];
    }

}

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

/* Save the file if modified after editing with BEE */
// save as




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
