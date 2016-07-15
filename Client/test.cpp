#include <ncurses.h>
#include <string>
#include <string.h>
#include <vector>
#include "screen.hpp"
#include <unistd.h>
#include <iostream>

int main()
{
    //unsigned int delay = 50000;       Not sure if we neeed this
    Screen main = Screen();
    main.init();

    vector <string> wolfFiles {"wolfy.txt", "wolfy2.txt" };
    Window* wolf = main.loadImages(wolfFiles, 10, 10);      // Need a more elegant way to add things.  Maybe call these from a single function in Screen.
    
    main.update();

    /*added by martha*/
    int j=0;
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    timeout(0);
/*added by martha*/

    wchar_t ch;
    while( (ch =getch() ) != 'q' )
    {
        /*added by martha*/
        main.scrollBg(j);
        j++;
        /*added by martha*/

        switch(ch)
        {
            case KEY_LEFT:
                main.move("left", wolf);
                break;
            case KEY_RIGHT:
                main.move("right", wolf);
                break;
            case KEY_UP:
                main.move("up", wolf);
                break;
            case KEY_DOWN:
                main.move("down", wolf);
                break;
            /*default case  commented out by martha*/
            //default:
               // mvprintw(1,0, "Use the arrow keys to move your puppy!\n ('q' to quit)");
        }
    }
    main.cleanup();
    endwin();
    return 0;
}
