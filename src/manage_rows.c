#include "../include/manage_row.h"

/* Convert an rx into a cx */

int cusror_x2render_x(plain_row *row,int cursor_x){
    int render_x =0;
    for (int i = 0; i < cursor_x; i++){
        render_x++;//normal character or a tab
        if (row->row_data[i] == '\t'){
            while (render_x % BEE_TAB_STOP != 0){
                render_x++;
            }
        }
    }
    return render_x;
}

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
