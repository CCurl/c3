# The c3 editor

The c3 editor is an editor inspired by, and similar to, a stripped-down version of VI.

There are 4 modes in the c3 editor:
- NORMAL
- INSERT
- REPLACE
- COMMAND

## Modes

### These control keys are active in all modes:

| Key      | Action |
| :--      | :-- |
| [ctrl]+H | Left 1 char |
| [ctrl]+J | Down 1 line |
| [ctrl]+K | Up 1 line |
| [ctrl]+L | Right 1 char |
| [tab]    | Right 8 chars |
| [ctrl]+I | Right 8 chars |
| [ctrl]+E | Scroll down 1 line |
| [ctrl]+Y | Scroll up 1 line |
| [ctrl]+D | Scroll down 1/2 screen |
| [ctrl]+U | Scroll up 1/2 screen |
| [ctrl]+X | Delete the char to the left of the cursor |
| [escape] | Change to NORMAL mode |

### NORMAL mode

The movement keys are similar to those in VI:

| Key  | Action|
| :--  | :-- |
| H    | Left 1 char |
| J    | Down 1 line |
| K    | Up 1 line |
| L    | Right 1 char |
| $    | Goto the end of the line |
| _    | Goto the beginning of the line |
| [SP] | Right 1 char |
| [CR] | Goto the beginning of the next line |
| a    | Append: move right 1 char and change to INSERT mode |
| A    | Append: goto the end of the line and change to INSERT mode |
| c    | Change: Delete the current char and change to INSERT mode (same as 'xi') |
| C    | Change: Delete to the end of the line and change to INSERT mode (same as 'd$A') |
| dd   | Copy the current line into the YANK buffer and delete the line |
| d$   | Delete to the end of the line |
| D    | Delete to the end of the line (same as 'd$') |
| g    | Goto the top-left of the screen |
| G    | Goto the bottom-left of the screen |
| i    | Insert: change to INSERT mode |
| I    | Insert: goto the beginning of the line and change to INSERT mode |
| J    | Join the current and next lines together |
| L    | Load: discard all changes and reload the current block |
| o    | Insert an empty line BELOW the current line and change to INSERT mode |
| O    | Insert an empty line ABOVE the current line and change to INSERT mode |
| p    | Paste the YANK buffer into a new line BELOW the current line |
| P    | Paste the YANK buffer into a new line ABOVE the current line |
| r    | Replace the char under the cursor with the next key pressed (if printable) |
| R    | Change to REPLACE mode |
| x    | Delete the char under the cursor |
| X    | Delete the char to the left of the cursor |
| Y    | Copy the current line into the YANK buffer |
| :    | Change to COMMAND mode |
| +    | Save the current block and read/edit the next block |
| -    | Save the current block and read/edit the previous block |

### INSERT mode

In INSERT mode, all printable characters are inserted into the block.

Carriage-Return inserts a new line.

### REPLACE mode

In REPLACE mode, all printable characters are placed into the block.

Carriage-Return moves to the beginning of the next line.

### COMMAND mode

COMMAND mode is invoked when pressing ':' in NORMAL mode.

| Command | Action|
| :--     | :-- |
| w       | Write the current block if it has changed |
| w!      | Write the current block, even if it has NOT changed |
| q       | Quit, if the current block has NOT changed |
| q!      | Quit, even if the current block has changed |
| wq      | Write the current block and quit (same as ':w' ':q') |
