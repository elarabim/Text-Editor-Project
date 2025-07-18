#include "../include/manage_file.h"

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

void save_edited(){
    
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
                // Reset old_config.dirty to 0 once the file is saved
                old_config.dirty = 0;
                return;
            }
        }
        close(f) ;
    }
    // if we get to here it surely means the saving process failed
    set_status_message("Saving failed! %s", strerror(errno));
    free(str) ;
   

}
