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
    //use_default_colors();
    
    // Should probably put these in a function later: initColors(int level)
    init_pair(1, COLOR_GREEN, COLOR_YELLOW);    
    init_pair(2, COLOR_MAGENTA, COLOR_YELLOW);
    init_pair(3, COLOR_RED, COLOR_YELLOW);
    init_pair(4, COLOR_WHITE, COLOR_YELLOW);
    init_pair(5, COLOR_BLUE, COLOR_YELLOW);
    init_pair(6, COLOR_WHITE, COLOR_MAGENTA);


    cbreak();                   // Don't wait for user to hit 'enter'
    noecho();                   // Don't display user input on screen
    keypad(stdscr, TRUE);       // For getting arrow keys
    curs_set(0);                // Make the cursor invisible --Doesn't work :(

   //this->bgWindow = Window::getBackground(1,0);   // Get the first level's background and set it to panel level 0
    this->bgWindow = Window::getBackgroundFromFile(0);
    addToPanelLevel(this->bgWindow);
    loadDeathAnimations();
    this->level = 1;
    this->bgIndex= 0;       // Scroll the bg file starting at col 0
    deathAnimation = NULL;
    dead = false;
    
    // I'm thinking maybe we put createBackgroundElements(int level)
    // and createOpponents(int level) in this module and then call them here...
}

/* This function will be called at the refresh rate
   so it will wait for a character input, if it doesn't
   happen in REFRESH_RATE time, it just refreshes the screen
*/   
void Screen::update(int frame)
{
    // Scroll the background here
    scrollBg(bgIndex++);
    if (bgIndex==2042)  //max column size of bg file and should be updated accordingly
        bgIndex=0;
    
    // Check for collision and play the appropriate animation
    if(!dead)
        checkIfDead();

    // If dead and the whole death animation has played out        
    if( dead )
    {
        if(frame%5 == 0)
        {           // The animation x, y, are ignored because it will play at the current location of deathAnimation
            playAnimation(deathAnimation, 0, 0);
            if( deathAnimation->getPanelIndex() == deathAnimation->getBasePanelIndex() )
            {
                usleep(3000000);
                // Show something on the screen? call another function?
                exit(0);
            }
        }
    }
    
    // Move any active (on screen) windows
    if( !dead )
    {
        for( Window* w : activeWindows)
            move("left", w);
    }
        
    update_panels();
    doupdate();
}

/**
 * Initialization method to load the death animation files into the deathAnimations map
 */

void Screen::loadDeathAnimations()
{
    vector <string> mauledFiles {"mauled1.txt","mauled2.txt","mauled3.txt","mauled4.txt","mauled5.txt","mauled6.txt"};
    vector <string> fallingFiles { "fall1.txt" , "fall2.txt" , "fall3.txt" , "fall4.txt" , "fall5.txt" };
    
    deathAnimations["mauled"] = loadImages(mauledFiles, WinType::DEATH);
    deathAnimations["fall"] = loadImages(fallingFiles, WinType::DEATH);
}



void Screen::putOnScreen(Window* image, int X, int Y)
{
    image->setX(X);
    image->setY(Y);
    move_panel( this->panelLevel[ image->getPanelIndex() ] , Y, X );
    show_panel( this->panelLevel[ image->getPanelIndex() ] );
    activeWindows.push_back(image);
}

void Screen::removeFromScreen(Window* image)
{
    hide_panel( this->panelLevel[image->getPanelIndex()] );
    for(unsigned int i=0; i<activeWindows.size(); i++)
    {
        if(activeWindows[i] == image)
        {
            activeWindows.erase( activeWindows.begin() + i );   // Ridiculous syntax...you must use an iterator?!
            return;
        }
    }
}

void Screen::checkIfDead()
{
    int herox1 = hero->getX();
    int herox2 = herox1+hero->getWidth();
    int heroy1 = hero->getY();
    int heroy2 = heroy1 + hero->getHeight();
    
    for( Window* screenElement :  activeWindows )
    {
        if( screenElement->getWinType() == WinType::ENEMY )
        {
            if(screenElement->getX() > herox1 && screenElement->getX() < herox2 &&
                screenElement->getY() > heroy1 && screenElement->getY() < heroy2)
            {
                dead = true;
                hide_panel( panelLevel[hero->getPanelIndex()] );
                deathAnimation = deathAnimations["mauled"];
                deathAnimation->setX(hero->getX());
                deathAnimation->setY(hero->getY());
                return;
            }
        }
        else if( screenElement->getWinType() == WinType::PIT )
        {
            if(screenElement->getX() > herox1 && screenElement->getX() < herox2 &&
                screenElement->getY() > heroy1 && screenElement->getY() < heroy2)
            {
                dead = true;
                hide_panel( panelLevel[hero->getPanelIndex()] );
                deathAnimation = deathAnimations["fall"];
                deathAnimation->setX(screenElement->getX()+4);
                deathAnimation->setY(screenElement->getY()+1);
                return;        
            }
        }
        //else if( screenElement->getWinType() == WinType::WALL )
    }
}



Window* Screen::loadHero( vector<string> filenames, int x, int y )
{
    Window* heroWindow = loadImages(filenames, WinType::HERO);
    this->hero = heroWindow;
    wrefresh(heroWindow->getTop());
    return heroWindow;
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
Window* Screen::loadImages(vector<string> filenames, WinType type)
{
    Window* baseWindow = new Window(filenames[0], 200, 200, type);
    baseWindow->setPanelIndex( panelLevel.size() );                  // Noting the Index of the panel location in the Window class
    this->panelLevel.push_back( new_panel(baseWindow->getTop()) );
    hide_panel( this->panelLevel[panelLevel.size()-1] );             // Hide the panel, putOnScreen() shows it.
    
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
void Screen::move(std::string direction, Window* baseW)
{
    if(dead)  return;     // Don't move anything if he's dying
    
    int levelSpeed = this->level;

    int xPos = baseW->getX();
    int yPos = baseW->getY();
    
    // 'Play' the next animation frame in the same spot (we will move it below)
    if(baseW->isAnimated())
        playAnimation(baseW, xPos, yPos);

    if(direction == "left") 
    {
        baseW->setX(xPos - 2*levelSpeed);
        if( (baseW->getWinType() == WinType::HERO) && ( baseW->getX() < 0) )
            baseW->setX(xPos + 2*levelSpeed);
        else if(baseW->getX() + baseW->getWidth() <= 0)     // Didn't add this check for the others, not sure if
        {                                                   // we plan to make thing move up or down off the screen
            removeFromScreen(baseW);                        // but it's simple to add later if we decide to.
            return;     // Don't bother moving the panel
        }
    }
    if(direction == "right")
    {
        baseW->setX(xPos + 2*levelSpeed);
        if( (baseW->getWinType() == WinType::HERO) && ( baseW->getX() + baseW->getWidth() > bgWindow->getWidth()) )
            baseW->setX(xPos - 2*levelSpeed);
    }
    if(direction == "up")   
    {
        baseW->setY(yPos - levelSpeed);
        if( (baseW->getWinType() == WinType::HERO) && ( baseW->getY() < 0 ) )
            baseW->setY(yPos + levelSpeed);
    }
    if(direction == "down") 
    {
        baseW->setY(yPos + levelSpeed );
        if( (baseW->getWinType() == WinType::HERO) && (baseW->getY() + baseW->getHeight() > bgWindow->getHeight()) )
            baseW->setY(yPos - levelSpeed );
    }
    move_panel( this->panelLevel[ baseW->getPanelIndex() ] , yPos, xPos );
}


/**
 * Rotates to the next image in the animation and moves the panel to the scpecified position.
 */
void Screen::playAnimation(Window* baseWindow, int x, int y)
{
    hide_panel( panelLevel[ baseWindow->getPanelIndex() ] );                // Hide the top window
    baseWindow->rotate();                                                   // Changes the top window index
    show_panel( panelLevel[ baseWindow->getPanelIndex() ] );                //  Shows the NEW top window
    if (baseWindow == deathAnimation)
        move_panel( this->panelLevel[ baseWindow->getPanelIndex() ], deathAnimation->getY(), deathAnimation->getX() );
    else
        move_panel( this->panelLevel[ baseWindow->getPanelIndex() ], y, x );
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
    delete this->bgWindow;
    this->bgWindow = Window::getBackgroundFromFile(j);
    this->bgWindow->setPanelIndex(0);
    replace_panel(this->panelLevel[0],this->bgWindow->getTop());
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
