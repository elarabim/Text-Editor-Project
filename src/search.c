#include "search.h"
#include "manage_row.h"

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


