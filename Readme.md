# NEXT - terminal based hex editor for linux

Didn't feel like bothering the sysadmin to install a hex editor so I made my own.

Currently reads and writes but does not append and can't create new files, so it's a little limited at the moment.

## COMPILING
Depends on lncursesw.  Once you have that, just run make and you're good.

## USAGE
Provide a filename as an argument on the command line.  You'll see a three-paned TUI.  On the left is line numbering, the middle is a hexadecimal view of the file, the right is an ASCII view.  Printable characters are printed, all others are shown as '.'

Use the arrow buttons to navigate, PGUP/PGDOWN to move one screen (256 bytes) at a time.

**Space** enters Write Mode, upon which you can edit the selected nybble (half-byte) by pressing a valid value (0-9/A-F).

Press S to save the opened file, and Q to quit.  Note that it does not ask you to save upon quitting, so make sure to save manually first.
