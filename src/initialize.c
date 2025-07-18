#include "../include/initialize.h"

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
