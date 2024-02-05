# The c4 editor

The c3 editor is a minialistic editor inspired by, and somewhat similar to, an extremely stripped-down version of VI.

There are 3 modes in the c3 editor:
- COMMAND
- INSERT
- REPLACE

## Modes

### These control keys are active in all modes:

| Key      | Action |
| :--      | :-- |
| [tab]    | Right 8 chars |
| [ctrl]+H | Left 1 char |
| [ctrl]+J | Down 1 line |
| [ctrl]+K | Up 1 line |
| [ctrl]+L | Right 1 char |
| [ctrl]+E | Scroll down 1 line |
| [ctrl]+Y | Scroll up 1 line |
| [ctrl]+D | Scroll down 1/2 screen |
| [ctrl]+U | Scroll up 1/2 screen |
| [ctrl]+X | Delete the char under the cursor |

### COMMAND mode

The movement keys are similar to those in VI:

| Key  | Action|
| :--  | :-- |
| H    | Left 1 char |
| [BL] | Right 1 char |
| J    | Down 1 line |
| K    | Up 1 line |
| L    | Right 1 char |
| [CR] | Goto the beginning of the next line |
| $    | Goto the end of the line |
| _    | Goto the beginning of the line |
| a    | Append: move right 1 char and go into INSERT mode |
| A    | Append: goto the end of the line and go into INSERT mode |
| c    | Change: Delete the current char and go into INSERT mode |
| C    | Change: Delete to the end of the line and go into INSERT mode |
| D    | Delete the current line into the YANK buffer |
| g    | Goto the top-left of the screen |
| G    | Goto the bottom-left of the screen |
| i    | Insert: go into INSERT mode |
| I    | Insert: goto the beginning of the line and go into INSERT mode |
| J    | Join the current and next lines together |
| L    | Load: discard all changes and reload the current block |
| o    | Insert an empty line below the current line and go into INSERT mode |
| O    | Insert an empty line above the current line and go into INSERT mode |
| p    | Paste copy the YANK buffer into a new line below the current line |
| P    | Paste copy the YANK buffer into a new line above the current line |
| Q    | Quit the editor |
| r    | Replace the char under the cursor with the next key typed |
| R    | Insert: goto the beginning of the line and go into INSERT mode |
| x    | Delete the char under the cursor |
| X    | Move left one char and delete the char under the cursor |
| W    | Write the current block |
| X    | Move left 1 char and delete the char to the left the cursor |
| Y    | Copy the current line into the YANK buffer |
| +    | Save the current block and read/edit the next block |
| -    | Save the current block and read/edit the previous block |

### INSERT mode

In INSERT mode, all printable characters are inserted into the block.

Carriage-Return inserts a new line.

### REPLACE mode

In REPLACE mode all printable characters are placed into the block.

Carriage-Return moves to the beginning of the next line.

