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
    timeout(10);
    /*added by martha*/

    wchar_t ch;
    while( (ch =getch() ) != 'q' )
    {
        /*added by martha*/
        main.scrollBg(j);
        j++;
        if (j==2042)  //max column size of bg file and should be updated accordingly
	        j=0;
	    int wolfX, wolfWidth, wolfY, wolfHeight, heroX, heroWidth, heroY, heroHeight;
	    getmaxyx( wolf->getTop(), wolfHeight, wolfWidth);
	    getmaxyx( hero->getTop(), heroHeight, heroWidth);

	    wolfX=wolf->getX();
	    wolfY=wolf->getY();
	    heroX=hero->getX();
	    heroY=hero->getY();

        //if endX of hero>startX of wolf and if startX of hero<endX of wolf and if 
        //endY of hero>startY of wolf and if startY of hero < endY of wolf, then it
        //means there's a collision. 
	    if(((heroX+heroWidth)>wolfX) && (heroX<(wolfX+wolfWidth))){
	        if (((heroY+heroHeight) > wolfY) && (heroY<(wolfY+wolfHeight))){
	            mvwprintw(main.getBG()->getTop(),1,0,"***!!!!COLLISIONNN!!!!***");
		        wrefresh(main.getBG()->getTop());
		        usleep(1000000);
		        break;	       
	        }
	    }
 	    /*Added by Martha*/
        
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
