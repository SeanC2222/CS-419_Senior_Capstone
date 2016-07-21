#include <ncurses.h>
#include <string>
#include <string.h>
#include <vector>
#include "screen.hpp"
#include <unistd.h>
#include <iostream>

int main()
{
    Screen main = Screen();
    main.init();

    vector <string> wolfFiles {"wolfy.txt", "wolfy2.txt" };
    vector <string> heroFiles {"gladiatorFacing.txt", "gladiatorStep.txt", "gladiatorBack.txt", "gladiatorStep.txt"};
    Window* hero = main.loadImages(heroFiles, 10,10);

    Window* wolf = main.loadImages(wolfFiles, 150, 10);      // Need a more elegant way to add things.  Maybe call these from a single function in Screen.

    main.update();

    /*added by martha*/
    int j=0;
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    timeout(100);
    /*added by martha*/

    wchar_t ch;
    while( (ch =getch() ) != 'q' )
    {
        /*added by martha*/
        main.scrollBg(j);
        j++;
        if (j==175)  //max column size of bg file and should be updated accordingly
	        j=0;
        /*added by martha*/
        
        main.move("left", wolf, true);        // Apparently we need to move all the bg objects first, otherwise they show up on top.

        switch(ch)
        {
            case KEY_LEFT:
                main.move("left", hero, false);
                break;
            case KEY_RIGHT:
                main.move("right", hero, false);
                break;
            case KEY_UP:
                main.move("up", hero, false);
                break;
            case KEY_DOWN:
                main.move("down", hero, false);
                break;
        }
        
        main.update();
    }
    main.cleanup();
    endwin();
    return 0;
}
