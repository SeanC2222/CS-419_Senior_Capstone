#include "screen.hpp"
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Screen::init()
{
    initscr();
    if(has_colors() == FALSE)
    { 
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    start_color();
    
    // Should probably put these in a function later: initColors(int level)
    init_pair(1, COLOR_GREEN, COLOR_YELLOW);    
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);
    

    cbreak();                   // Don't wait for user to hit 'enter'
    noecho();                   // Don't display user input on screen
    keypad(stdscr, TRUE);       // For getting arrow keys
    curs_set(0);                // Make the cursor invisible --Doesn't work :(

   //this->bgWindow = Window::getBackground(1,0);   // Get the first level's background and set it to panel level 0
    this->bgWindow = Window::getBackgroundFromFile(0);
    addToPanelLevel(this->bgWindow);
    this->level = 1;
    
    // I'm thinking maybe we put createBackgroundElements(int level)
    // and createOpponents(int level) in this module and then call them here...
}

/* This function will be called at the refresh rate
   so it will wait for a character input, if it doesn't
   happen in REFRESH_RATE time, it just refreshes the screen
*/   
void Screen::update()
{
    // Scroll the background here
    // Check for input and timeout and doUpdate() if none entered
        //halfdelay(x/10 seconds --probably needs to be an init function somewhere else)
    update_panels();
    doupdate();
}

void Screen::addToPanelLevel(Window* image)      // There is also a user pointer for each panel I think...used to store anything if we need it
{
    image->setPanelIndex( panelLevel.size() );
    this->panelLevel.push_back( new_panel(image->getTop()) );
    update_panels();
}

/* Load an animation from an array of filenames, in the order that you want the 
 * animation to proceed.  Actually could be used for all image files...
 */
Window* Screen::loadImages(vector<string> filenames, int xPos, int yPos)
{
    Window* baseWindow = new Window(filenames[0], xPos, yPos);
    baseWindow->setPanelIndex( panelLevel.size() );                  // Noting the Index of the panel location in the Window class
    this->panelLevel.push_back( new_panel(baseWindow->getTop()) );
    
    // For multiple files (only happens for animations)
    for(unsigned int i = 1; i < filenames.size(); i++) 
    {
        WINDOW* nextImage = baseWindow->appendAnimation( filenames[i] );
        this->panelLevel.push_back( new_panel(nextImage) );     // Add all the other screens to panels, but 
        hide_panel(this->panelLevel[ panelLevel.size()-1] );    // Hide the panels so only the original "baseWindow" shows
    }

    this->movingWindows.push_back(baseWindow);
    return baseWindow;
}



/* It now checks for a boolean bgElement flag, which will be set for all but
   the hero.  If it's there, the element can move off the screen.  It also
    now returns the last xPosition of the window (so if > 0 it's still on the screen).
    It's not going to work though, I don't think a panel can be rendered off-screen, but 
    a window can, so one option is to move the window off the panel...
*/
int Screen::move(std::string direction, Window* baseWindow, bool bgElement)
{
    int levelSpeed = this->level;

    int xPos = baseWindow->getX();
    int yPos = baseWindow->getY();
    
    // If it does, make the current image hidden, make the next image visible and change the top Window
    if(baseWindow->isAnimated())
    {
        hide_panel( panelLevel[ baseWindow->getPanelIndex() ] );                // Hide the top window
        baseWindow->rotate();                                                   // Changes the top window index
        show_panel( panelLevel[ baseWindow->getPanelIndex() ] );                //  Shows the NEW top window
    }

    
    if(direction == "left")
        if( (bgElement == false && xPos > 0 ) || bgElement == true )                   // Only move left if not at the left edge
            baseWindow->setX(xPos - 2*levelSpeed);
    if( direction == "right" && (xPos + baseWindow->getWidth()  < bgWindow->getWidth()) )          
        baseWindow->setX(xPos + 2*levelSpeed);
    if(direction == "up" && yPos > 0 )                              
        baseWindow->setY(yPos - levelSpeed);
    if(direction == "down" && (yPos + baseWindow->getHeight() < bgWindow->getHeight()) )          
                baseWindow->setY(yPos + levelSpeed );

    move_panel( this->panelLevel[ baseWindow->getPanelIndex() ] , yPos, xPos );
    
    return (xPos + baseWindow->getWidth());
}

/* Dylan's Scroll Method 
void Screen::scrollBG(Window* bgWindow) // I'm thinking later change params to an array of Window pointers for all objects in the background
{
    int levelSpeed = 2*this->level;

    WINDOW* bgWin = bgWindow->getTop();
    int rows = bgWindow->getHeight();
    int cols = bgWindow->getWidth();
    char* first = new char[cols/2];
    char* remaining = new char[cols];
    std::string strToPrint = "";
    
    wattron(bgWin, COLOR_PAIR(1));
    // For all lines in window
    for(int i = 0; i < rows; i++)
    {
        mvwinnstr(bgWin, i, 0, first, levelSpeed);                          // Get the first section (how many depends on the level)
        mvwinnstr(bgWin, i, levelSpeed, remaining, cols-(levelSpeed+1) );   // Get the next part of the line

        strToPrint = remaining;                                             // Add them together in reverse order
        strToPrint += first;
        mvwprintw( bgWin, i, 0, strToPrint.c_str() );     // Print the remainder of the line at the beginning, first part at end
    }
    
    wattroff(bgWin, COLOR_PAIR(1));
    
    delete[] first;
    delete[] remaining;
    
    wrefresh(bgWin);
}
*/


/*Martha: updates background by replcing it with a scrolled version of itself*/
void Screen::scrollBg(int j)
{
    //this->bgWindow = Window::getBackground(1,j);//Background will start reading from column j
    this->bgWindow = Window::getBackgroundFromFile(j);
    this->bgWindow->setPanelIndex(0);
    replace_panel(this->panelLevel[0],this->bgWindow->getTop());
    update();
}


Window* Screen::getBG()
{
    return this->bgWindow;
}



void Screen::cleanup()
{
    delete bgWindow;
    for( auto *window : movingWindows)
        delete window;
}
