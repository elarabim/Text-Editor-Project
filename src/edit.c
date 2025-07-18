#include "../include/edit.h"



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