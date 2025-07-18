#include "../include/syntax.h"

char* HL_C_extensions[] = {".c", ".h", ".cpp", NULL} ;

char *HL_C_KEYWORDS[]= {//C keywords highlighting
  "switch", "if", "while", "for", "break", "continue", "return", "else",
  "struct", "union", "typedef", "static", "enum", "class", "case",
   
  "int|", "long|", "double|", "float|", "char|", "unsigned|", "signed|",
  "void|", NULL
};
Syntax HL_Database[] = {{"c", HL_C_extensions, HL_C_KEYWORDS, "//", "/*", "*/", HIGHLIGHT_NUMBERS | HIGHLIGHT_STRINGS}} ;



int is_separator(int c) {
    /* strchr looks for the first occurrence of a character in a 
    string, and returns a pointer to the matching character in the string */
    return isspace(c) || c == '\0' || strchr(",.()+-/*=~%<>[];", c) != NULL ;
}



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

    if (highlight == HL_DIGITS){ return 96;} // yellow
    else if (highlight == HL_MATCH) {return 91 ;} /* red */
    else if (highlight == HL_STRING){ return 93;} /*bright yellow*/
    else if (highlight == HL_COMMENT || highlight == HL_MLCOMMENT){ return 32;}//Green FOR COMMENTS
    else if (highlight == HL_KEYWORD1){ return 35;}//magnita
    else if (highlight == HL_KEYWORD2){ return 34;}//blue
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