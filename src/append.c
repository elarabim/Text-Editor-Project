 #include "../include/append.h"

 void append_buffer(text_buffer* current_text_buff, char* c, int length_c){
    int length_b = current_text_buff->length;
    char* new_text = realloc(current_text_buff->text, length_b + length_c);

    // let's avoid any memory problems
    if (new_text == NULL) {
        printf("The text is NULL");
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