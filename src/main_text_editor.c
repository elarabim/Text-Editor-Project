#include "../include/display.h"
#include "../include/initialize.h"
#include "../include/input.h"

terminal_configurations old_config ;

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

