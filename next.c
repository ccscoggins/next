#include <ncursesw/ncurses.h>
#include <inttypes.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "lcui.h"

WINDOW * byteWin;
WINDOW * textWin;
WINDOW * lineWin;

uint8_t * filebuffer;
long int filelen;
char * filename;
int yoffset;
long int selectedByte;
int selectedNybl;
int writeMode = false;

int loadFile(const char * filename){
    long int result;
    int ret;
    FILE * fp = fopen(filename, "rb");
    
    if (!fp){
        fprintf(stderr, "Unable to open file %s\n", filename);
        return -1;
    }
    
    fseek(fp, 0L, SEEK_END);
    filelen = ftell(fp);
    rewind(fp);
    
    filebuffer = malloc(filelen * sizeof(uint8_t)); //technically sizeof(uint8_t) is unnecessary but it's more legible
    result = fread(filebuffer, sizeof(*filebuffer), filelen, fp);
    if (result != filelen){
        fprintf(stderr, "File %s could not be loaded fully\n", filename);
        ret = -1;
    }
    else
        ret = 0;
    
    fclose(fp);
    return ret;
}

//Fixme - change messages to use ncurses instead of stderr
int saveFile(const char * filename){
    FILE * fp = fopen(filename, "rb");
    long int result;
    if (!(!fp)){
        fprintf(stderr, "File %s already exists.  Moving existing file to %s.bak\n", filename, filename);
        fclose(fp);
        char * newname = malloc(sizeof(*filename) * (strlen(filename) + 5));
        strcpy(newname, filename);
        strcat(newname, ".bak");
        result = rename(filename, newname);
        if (result != 0){
            fprintf(stderr, "ERROR: Unable to backup existing file.  Program will exit\n");
            exit(-1);
        }
    }
    
    fp = fopen(filename, "wb");
    result = fwrite(filebuffer, sizeof(*filebuffer), filelen, fp);
    if(result != filelen){
        fprintf(stderr, "WRITE ERROR: Bytes written not equal to bytes in memory\n");
    }
    else{
        fprintf(stderr, "Write successful\n");
    }
    
    fclose(fp);
}

int saveSequence(){
    char * newfilename;
    FILE * fp;
    int validname = false, getout = false, result;
    
    do{
        newfilename = ask(" Name ", " Please enter a filename:", validname ? "true" : "false");
        
        if (strlen(newfilename) < 1){
            getout = !dialog(" Error ", " Filename cannot be empty.", "", "Ok", "Cancel");
            if(getout) break;
        }
        else if (fp = fopen(newfilename, "rb")){
            fclose(fp);
            validname = dialog(" Notice ", " File already exists.", "Would you like to overwrite it?", "Yes", "No");
        }
        else{
            validname = true;
        }
    } while(!validname);
    
    if (!validname)
        return -1;
    
    fp = fopen(newfilename, "wb");
    if (!fp){
        dialog(" Error ", "  The file could not be opened.", "", "Ok", "Ok");
    }
    result = fwrite(filebuffer, sizeof(*filebuffer), filelen, fp);
    // add append buffer here
    if (result != filelen){
        dialog(" Error ", "The file may have saved incorrectly.", "", "Ok", "Ok");
    }
    else{
        dialog(" Saved ", "  The file saved successfully.", "", "Ok", "Ok");
    }
    
    
}


void freeAll(){
    free(filebuffer);
    filelen = 0;
}

int getHexvalForChar(char input){
    if (input >= '0' && input <= '9')
        return input - '0';
    else
        return input - 'a' + 10;
}

void insertNybl(char input, long int selectedByte, int selectedNybl){
    uint8_t currentValue = filebuffer[selectedByte];
    uint8_t andval = 0x0f << ((1 - selectedNybl) * 4);
    uint8_t orval = getHexvalForChar(input) << (selectedNybl * 4);
    filebuffer[selectedByte] = (currentValue & andval) | orval;
}

void startViewer(){
    initscr();
    noecho();
    cbreak();
    curs_set(0);
    keypad(stdscr, TRUE);
    
    byteWin = newwin(18, 50, 4, 9);
    textWin = newwin(18, 19, 4, 59);
    lineWin = newwin(18, 9, 4, 0);
    
    yoffset = 0;
    selectedByte = 0;
    selectedNybl = 1;
    
    int action;
    int loop = true;
    while(loop){
        clear();
        werase(byteWin);
        werase(textWin);
        werase(lineWin);
        uint8_t contents;
        
        int currbyte;
        for (int i=0; i<256; i++){
            currbyte = yoffset * 16 + i;
            if (currbyte < 0)
                continue;
            if (currbyte >= filelen)
                break;
            if (currbyte % 16 == 0)
                mvwprintw(lineWin, i/16 + 1, 0, "%08X", currbyte);
            contents = filebuffer[currbyte];
            mvwprintw(byteWin, i/16 + 1, (i%16 * 3) + 1, "%02X", contents);
            if (currbyte == selectedByte){
                wattron(textWin, A_STANDOUT);
                mvwchgat(byteWin, i/16 + 1, (i%16 * 3) + 2 - selectedNybl, 1, A_STANDOUT, 0, NULL);
            }
            else{
                wattroff(textWin, A_STANDOUT);
            }
            mvwprintw(textWin, i/16 + 1, (i%16) + 1, "%c", (contents < 0x20 | contents > 0x7e) ? '.' : contents);
        }
        
        wattroff(byteWin, A_STANDOUT);
        wattroff(textWin, A_STANDOUT);
        
        mvwvline(lineWin, 1, 8, '|', 16);
        box(byteWin, '|', '-');
        box(textWin, '|', '-');
        
        mvprintw(0, 22, "NEXT Hex Editor - 2022 ccscoggins");
        mvprintw(1, 4, "Filename: %s", filename);
        mvprintw(2, 4, "File Length: %8X, Selected byte: %8X, yoff: %d", filelen, selectedByte, yoffset);
        if (writeMode){
            mvprintw(3, 40, "WRITE MODE");
        }
        
        refresh();
        wrefresh(textWin);
        wrefresh(lineWin);
        wrefresh(byteWin);
        
        int input;
        switch(input = getch()){
            case KEY_UP:
                selectedByte -= 16;
                if (selectedByte < 0)
                    selectedByte = 0;
                if (selectedByte < yoffset * 16)
                    yoffset -= 1;
                break;
            case KEY_DOWN:
                selectedByte += 16;
                if (selectedByte >= filelen)
                    selectedByte = filelen - 1;
                if (selectedByte > (yoffset + 15) * 16)
                    yoffset += 1;
                break;
            case KEY_LEFT:
                selectedNybl += 1;
                if (selectedNybl > 1){
                    selectedNybl = 0;
                    selectedByte -= 1;
                    if (selectedByte < 0)
                        selectedByte = 0;
                    if (selectedByte < yoffset * 16)
                        yoffset -= 1;
                }
                break;
                
            case '0': case '1': case '2': case '3':
            case '4': case '5': case '6': case '7':
            case '8': case '9': case 'a': case 'b':
            case 'c': case 'd': case 'e': case 'f':
                if (writeMode)
                    insertNybl((char)input, selectedByte, selectedNybl);
                if (!writeMode)
                    break;
            case KEY_RIGHT:
                selectedNybl -= 1;
                if (selectedNybl < 0){
                    selectedNybl = 1;
                    selectedByte += 1;
                    if (selectedByte >= filelen)
                        selectedByte = filelen - 1;
                    if (selectedByte > (yoffset + 15) * 16)
                        yoffset += 1;
                }
                break;
            case KEY_PPAGE:
                selectedByte -= 256;
                if (selectedByte < 0)
                    selectedByte = 0;
                for (int i=0; i<16; i++){
                    if (selectedByte > (yoffset+14) * 16)
                        break;
                    yoffset -= 1;
                }
                break;
                
            case KEY_NPAGE:
                selectedByte += 256;
                if (selectedByte >= filelen)
                    selectedByte = filelen - 1;
                for (int i=0; i<16; i++){
                    if (selectedByte < (yoffset+1) * 16){
                        break;
                    }
                    yoffset += 1;
                }
                break;
                
            case ' ':
                writeMode = !writeMode;
                break;
            case 'q':
                loop = !dialog(" Quit ",
                    "  Would you like to quit?",
                    "Your work will not be saved.",
                    "Yes",
                    "No");
                break;
            case 's':
                if (dialog(" Save ", "  Would you like to save?", "", "Yes", "No")) saveSequence();
        }
    }
    
    delwin(byteWin);
    delwin(textWin);
    
    endwin();
}

void displayHelp(){
    fprintf(stdout, "NEXT - Hex Viewer/Editor\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "A filename must be provided to run NEXT\n");
    
}

int main(int argc, char * argv[]){
    if (argc < 2){
        displayHelp();
        exit(-1);
    }
    
    filename = argv[1];
    
    int start = loadFile(filename);
    if (start != 0){
        exit(-1);
    }
    
    startViewer();
    
    freeAll();
}
