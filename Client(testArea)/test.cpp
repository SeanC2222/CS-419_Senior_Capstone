#include <ncurses.h>
#include <string>
#include <string.h>
#include <vector>
#include "screen.hpp"
#include <unistd.h>
#include <iostream>
#include <fstream>

int menu();

int main()
{
    int frame = 0;
    if (menu()==1) //menu returns 1 when 'q' is pressed
	    return 0;
    Screen main = Screen();
    main.init();

    vector <string> wolfFiles {"wolfy.txt", "wolfy2.txt" };
    vector <string> heroFiles {"gladiatorFacing.txt", "gladiatorStep.txt", "gladiatorBack.txt", "gladiatorStep.txt"};
    vector <string> pit1Files  {"pit1.txt" };
    vector <string> pit2Files  {"pit2.txt" };
    
    // Need to change this.  Maybe just add by default off the screen
    Window* wolf = main.loadImages(wolfFiles, WinType::ENEMY);
    Window* pit1 = main.loadImages(pit1Files, WinType::PIT);
    Window* pit2 = main.loadImages(pit2Files, WinType::PIT);
    main.putOnScreen(wolf, 150, 10);
    main.putOnScreen(pit1, 300, 5);
    main.putOnScreen(pit2, 200, 20);

    Window* hero = main.loadHero(heroFiles, 10, 10);

    main.update();
    timeout(100);
    
    
    wchar_t ch;
    while( (ch =getch() ) != 'q' )
    {
        frame++;
        if(frame == 150)
            main.putOnScreen(wolf, 300, 10);
        

        switch(ch)
        {
            case KEY_LEFT:
                main.move("left", hero);
                break;
            case KEY_RIGHT:
                main.move("right", hero);
                break;
            case KEY_UP:
                main.move("up", hero);
                break;
            case KEY_DOWN:
                main.move("down", hero);
                break;
        }
        
        if( main.update() == 1)
            break;
    }
    main.cleanup();
    endwin();
    return 0;
}

//menu page
int menu() 
{ 
   initscr();
   keypad(stdscr,true); 
   start_color();     
   init_pair(7,COLOR_GREEN,COLOR_BLACK);   
   init_pair(8,COLOR_YELLOW, COLOR_BLACK);
   init_pair(9,COLOR_RED, COLOR_BLACK);
   attrset (COLOR_PAIR(7));                 
   int row, col;
   getmaxyx(stdscr, row, col);
   int x=col/2-35;
   int y=row/2-12;
   ifstream inputfile, inputfile1;
   inputfile.open("./Images/endlessrunner.txt");
   inputfile1.open("./Images/gladiatorFacing.txt");
   if(inputfile.is_open()&&inputfile1.is_open()){
      string line,line1;
      while(getline(inputfile,line)){
	//mvprintw(y,x,"ingetline");
	//y++;
 	mvprintw(y,x,line.c_str());
	y++;
      }
      attrset(COLOR_PAIR(8));	
      x=col/2-4;
      while(getline(inputfile1,line1)){
	mvprintw(y,x,line1.c_str());
	y++;
      }
   }
   else
	mvprintw(y,x,"could not open files");
   y++;
   attrset (COLOR_PAIR(7));
   x=col/2-2;
   mvprintw(y,x,"by:\n");
   y++;
   x=col/2-8;
   mvprintw(y,x,"dylan kosloff\n");
   y++;
   x=col/2-9;
   mvprintw(y, x,"martha gebremariam &\n");
   y++;
   x=col/2-8;
   mvprintw(y,x,"sean mulholland\n");
   y=y+2;
   x=col/2-15;
   mvprintw(y,x,"press space key to start game\n");
   x=col/2-12;
   y++;
   mvprintw(y,x,"press tab key for how-to\n");
   y++;
   x=col/2-8;
   mvprintw(y,x,"press 'q' to quit\n");

   wchar_t ch;
   while( (ch =getch() ) != 'q' )
   {
      if(ch==9) //to be updated later
	    return 0;
	  else if(ch==' ')
	    return 0;
   }    
   endwin(); 
   return 1;
}