#include <ncurses.h>
#include <string>
#include <string.h>
#include <vector>
#include "screen.hpp"
#include <unistd.h>

int main()
{
    //unsigned int delay = 50000;       Not sure if we neeed this
    Screen main = Screen();
    main.init();

    vector <string> wolfFiles {"wolfy.txt", "wolfy2.txt" };
    Window* wolf = main.loadImages(wolfFiles, 10, 10);      // Need a more elegant way to add things.  Maybe call these from a single function in Screen.
    
    main.update();

    wchar_t ch;
    while( (ch =getch() ) != 'q' )
    {  
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
            default:
                mvprintw(1,0, "Use the arrow keys to move your puppy!\n ('q' to quit)");
        }
        //usleep(delay);

    }
    main.cleanup();
    endwin();
    return 0;
}