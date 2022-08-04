#include <ncursesw/ncurses.h>
#include <string.h>

char answerBuffer[30];

int dialog(const char * header, const char * line1, const char * line2, const char * opt1, const char * opt2){
    int ret = -1;
    int choice = false;
    int ans;
    
    WINDOW * dialog = newwin(6, 32, 7, 20);
    while(ret == -1){
        wclear(dialog);
        
        wattroff(dialog, A_STANDOUT);
        box(dialog, '|', '-');
        mvwprintw(dialog, 0, 3, header);
        mvwprintw(dialog, 1, 2, line1);
        mvwprintw(dialog, 2, 2, line2);
        
        if (choice == true){
            wattron(dialog, A_STANDOUT);
        }
        else{
            wattroff(dialog, A_STANDOUT);
        }
        mvwprintw(dialog, 3, 2, opt1);
        
        if (choice == false){
            wattron(dialog, A_STANDOUT);
        }
        else{
            wattroff(dialog, A_STANDOUT);
        }
        mvwprintw(dialog, 4, 2, opt2);
        
        wrefresh(dialog);
        
        switch(ans = getch()){
            case KEY_UP:
            case KEY_DOWN:
                choice = !choice;
                break;
            case ' ':
            case KEY_ENTER:
            case '\n':
                ret = choice;
                break;
        }
    }
    
    delwin(dialog);
    return ret;
}

char * ask(const char * header, const char * line1, const char * line2){
    WINDOW * askWin = newwin(6, 32, 7, 20);
    
    
    echo();
    curs_set(1);
    wclear(askWin);
    wattroff(askWin, A_UNDERLINE);
    
    box(askWin, '|', '-');
    mvwprintw(askWin, 0, 3, header);
    mvwprintw(askWin, 1, 2, line1);
    mvwprintw(askWin, 2, 2, line2);
    
    wattron(askWin, A_UNDERLINE);
    mvwhline(askWin, 4, 2, ' ', 28);
    
    wrefresh(askWin);
    mvwgetnstr(askWin, 4, 2, answerBuffer, 28);
        
    
    delwin(askWin);
    noecho();
    curs_set(0);
    return answerBuffer;
}
