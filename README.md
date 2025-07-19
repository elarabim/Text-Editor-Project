# Text Editor Project

This project implements a minimalist text editor from scratch in C, with a particular focus on C file syntax highlighting.

## Features

- Syntax highlighting for C language.
- Support for opening, editing, and saving files.
- Cursor navigation and scrolling.
- Real-time file modification tracking.
- Modular codebase: each component has its own ".c" and ".h" files.
- Makefile-based build system.
- Terminal interface using raw mode and ANSI escape sequences.

## Compilation & Execution

1. Open a terminal and navigate to the main project directory.
2. Compile the project using : make

This will:
  - Compile all .c files.
  - Generate .o object files in the obj/ folder.
  - Create the executable main_text_editor in the root directory.

3. Run the editor with : ./main_text_editor <filename
Replace <filename> with the name of the file you want to edit. If the file doesn't exist, it will be created.

## File structure 

- main_text_editor.c – Entry point of the program.
- src/ – Contains all .c source files for different functionalities.
- include/ – Contains all corresponding .h header files.
- obj/ – Object files generated during compilation.
- Makefile – Automates compilation and linking.

## Usage
Once launched, the editor allows you to:
  - Navigate using arrow keys.
  - Type and delete text.
  - Save the file.
  - See real-time syntax highlighting if the file is C code.
