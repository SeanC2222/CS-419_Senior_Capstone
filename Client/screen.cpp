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
    init_pair(5, COLOR_WHITE, COLOR_MAGENTA);
    init_pair(6, COLOR_MAGENTA, COLOR_BLUE);
    init_pair(7, COLOR_WHITE, COLOR_BLUE);
    init_pair(8, COLOR_RED, COLOR_BLUE);


    cbreak();                   // Don't wait for user to hit 'enter'
    noecho();                   // Don't display user input on screen
    keypad(stdscr, TRUE);       // For getting arrow keys
    curs_set(0);                // Make the cursor invisible --Doesn't work :(

    this->bgWindow = Window::getBackgroundFromFile(0);
    addToPanelLevel(this->bgWindow);
    loadDeathAnimations();
    this->level = 1;
    this->bgIndex= 0;       // Scroll the bg file starting at col 0
    getmaxyx(stdscr, screenHeight, screenWidth);
    waterStart = 641;
    waterEnd = 937;
    deathAnimation = NULL;
    dead = false;
    deathFrame = 1;
    
    // I'm thinking maybe we put createBackgroundElements(int level)
    // and createOpponents(int level) in this module and then call them here...
}

/**
 * Returns the current level dictated by background
 **/
int Screen::getLevel(){
    return this->level;
}

/* This function will be called at the refresh rate
   so it will wait for a character input, if it doesn't
   happen in REFRESH_RATE time, it just refreshes the screen
*/   
int Screen::update()
{
    // Check for collision and play the appropriate animation
    if(!dead)
        checkIfDead();
 
    if( dead )
    {
        this->deathFrame++;
        playDeathScene();
        if( deathAnimation->isLastAnimationFrame() )
        {
            update_panels();
            doupdate();
            usleep(4000000);
            return(1);
        }
    }
        
    // Move any active (on screen) windows
    if( !dead )
    {
        bgWindow->showBgAt(bgIndex++, waterStart, waterEnd);    // Scroll the background here
        if (bgIndex==1000)  //max column size of bg file and should be updated accordingly
            bgIndex=0;  

        if( (hero->getX() + bgIndex >= waterStart -18) && (hero->getX() + bgIndex < waterStart - 14) )    // Rotate is called every 3 bg scrolls, so when in the range of the water, start swimming
            hero->startSwimming();

        for( auto* w : activeWindows)       // Move all the other windows
        {
            if( w->move(level) )    // Returns true if the top window changed
                replace_panel(panelLevel[w->getPanelIndex()], w->getTop());
            
            if(w->getX() < 0)    // If at the left edge, shrink or remove it.
            {
                if(w->getX() + w->getWidth() <= 0)           
                {                                                   
                    removeFromScreen(w);                        
                    return 0;     // Don't bother moving the panel
                }
                else
                    shrinkWindow(w);
            }
            // If at the right edge, show it.                
            move_panel( panelLevel[w->getPanelIndex()], w->getY(), w->getX() );
            int endOfPanel = w->getX() + w->getWidth(); 
            if( (endOfPanel >= screenWidth - 2*level) && (endOfPanel <= screenWidth) ) 
                show_panel( panelLevel[w->getPanelIndex()] );
        }
        if(bgIndex%5 == 0)
            hero->rotate();
        replace_panel( panelLevel[ hero->getPanelIndex() ], hero->getTop() );
        move_panel( this->panelLevel[ hero->getPanelIndex() ], hero->getY(), hero->getX() );
        
    }
        
    update_panels();
    doupdate();
    return 0;
}

/**
 * Initialization method to load the death animation files into the deathAnimations map
 */

void Screen::loadDeathAnimations()
{
    vector <string> mauledFiles {"mauled1.txt","mauled2.txt","mauled3.txt","mauled4.txt","mauled5.txt","mauled6.txt"};
    vector <string> fallingFiles { "fall1.txt" , "fall2.txt" , "fall3.txt" , "fall4.txt" , "fall5.txt", "fall6.txt" };
    vector <string> drowningFiles {"drown1.txt", "drown2.txt", "drown3.txt", "drown4.txt", "drown5.txt", "drown6.txt",  "drown7.txt"};
    
    deathAnimations["mauled"] = loadImages(mauledFiles, WinType::DEATH, COLOR_PAIR(2));
    deathAnimations["fall"] = loadImages(fallingFiles, WinType::DEATH, COLOR_PAIR(5));
    deathAnimations["drown"] = loadImages(drowningFiles, WinType::DEATH, COLOR_PAIR(6));
}


void Screen::playDeathScene()
{
    if(deathFrame%5 == 0)
    {   
        deathAnimation->rotate();                                                   // Changes the top window index
        replace_panel( panelLevel[ deathAnimation->getPanelIndex() ], deathAnimation->getTop() );
        //show_panel( panelLevel[deathAnimation->getPanelIndex()] );
        move_panel( panelLevel[deathAnimation->getPanelIndex()], deathAnimation->getY(), deathAnimation->getX() );

    }
}


void Screen::putOnScreen(Window* image, int X, int Y)
{
    image->setX(X);
    image->setY(Y);
    move_panel( this->panelLevel[ image->getPanelIndex() ] , image->getY(), image->getX() );
    if( X < screenWidth )
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
    int herox2 = herox1 + hero->getWidth();
    int heroy1 = hero->getY();
    int heroy2 = heroy1 + hero->getHeight();
    
    for( Window* screenElement :  activeWindows )
    {
        if( screenElement->getWinType() == WinType::ENEMY )
        {
            int enemyX = screenElement->getX();
            int enemyTopY = screenElement->getY();
            int enemyBottomY = enemyTopY + screenElement->getHeight();
            
            if( (enemyX > herox1 && enemyX < herox2) && 
              ( (enemyTopY > heroy1 && enemyTopY < heroy2) || (enemyBottomY > heroy1 && enemyBottomY < heroy2) ) )
            {
                dead = true;
                hide_panel( panelLevel[hero->getPanelIndex()] );
                deathAnimation = ( (herox1 > waterStart && herox2 < waterEnd) ? deathAnimations["drown"] : deathAnimations["mauled"]);
                deathAnimation->setY(hero->getY());
                deathAnimation->setX(hero->getX());
                show_panel( panelLevel[deathAnimation->getPanelIndex()] );
                move_panel( panelLevel[deathAnimation->getPanelIndex()], deathAnimation->getY(), deathAnimation->getX() );
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
                deathAnimation->setY(screenElement->getY() + 1);
                deathAnimation->setX(screenElement->getX() + 4);
                move_panel( panelLevel[deathAnimation->getPanelIndex()], deathAnimation->getY(), deathAnimation->getX() );
                show_panel( panelLevel[deathAnimation->getPanelIndex()] );
                return;        
            }
        }
        //else if( screenElement->getWinType() == WinType::WALL )
    }
}



void Screen::setHero(int x, int y )
{
    // Load the main walking files
    vector<string> walkFiles = {"gladiatorFacing.txt", "gladiatorStep.txt", "gladiatorBack.txt", "gladiatorStep.txt"};
    Hero* heroWindow = new Hero(walkFiles[0], 200, 200, WinType::HERO, COLOR_PAIR(2));  // For some reason if I add it at 0, 0, it still shows up even with the hide_panel call below
    addToPanelLevel(heroWindow);
    hide_panel( this->panelLevel[ panelLevel.size()-1 ] );
    for(unsigned int i = 1; i < walkFiles.size(); i++) 
        heroWindow->appendAnimation( walkFiles[i], COLOR_PAIR(2) );
    
    // Load the other movements
    vector<string> jumpFiles = {"jump1.txt", "jump2.txt", "jump3.txt", "jump4.txt", "jump5.txt", "jump6.txt", "jump7.txt", "jump8.txt"};
    vector<string> swimFiles = {"swimming1.txt", "swimming2.txt", "swimming3.txt", "swimming4.txt", "swimming5.txt",
        "swimming6.txt", "swimming7.txt", "swimming8.txt"};
    for(auto fname : jumpFiles)
        heroWindow->appendOtherAnimation(fname, "jump", COLOR_PAIR(6));
    for(auto fname : swimFiles)
        heroWindow->appendOtherAnimation(fname, "swim", COLOR_PAIR(6));
    heroWindow->saveScreenLimits(bgWindow->getWidth(), bgWindow->getHeight());
    
    
    heroWindow->setX(x);
    heroWindow->setY(y);
    move_panel(panelLevel[heroWindow->getPanelIndex()], heroWindow->getY(), heroWindow->getX());
    show_panel(panelLevel[heroWindow->getPanelIndex()]);
    this->hero = heroWindow;
    wrefresh(heroWindow->getTop());
}

void Screen::addToPanelLevel(Window* image)     
{
    image->setPanelIndex( panelLevel.size() );
    this->panelLevel.push_back( new_panel(image->getTop()) );
}

/* Load an animation from an array of filenames, in the order that you want the 
 * animation to proceed.  Actually could be used for all image files...
 */
Window* Screen::loadImages(vector<string> filenames, WinType type, int colors)
{
    Window* baseWindow = new Window(filenames[0], 200, 200, type, colors);  // For some reason if I add it at 0, 0, it still shows up even with the hide_panel call below
    addToPanelLevel(baseWindow);
    hide_panel( this->panelLevel[ panelLevel.size()-1 ] );
    
    // For multiple files (only happens for animations)
    for(unsigned int i = 1; i < filenames.size(); i++) 
        baseWindow->appendAnimation( filenames[i], colors );

    //this->movingWindows.push_back(baseWindow);
    return baseWindow;
}



void Screen::moveHero(std::string direction)
{
        hero->move(direction, level);
        replace_panel( panelLevel[ hero->getPanelIndex() ], hero->getTop() );
        move_panel( this->panelLevel[ hero->getPanelIndex() ], hero->getY(), hero->getX() );
}

void Screen::moveWin(std::string direction, Window* W)
{
        if(direction == "up"){
            W->setY(W->getY() - 1);
        } else if (direction == "down"){
            W->setY(W->getY() + 1);
        } else if (direction == "left"){
            W->setX(W->getX() - 1);
        } else if (direction == "right"){
            W->setX(W->getX() + 1);
        } else {
            //Do nothing...
        }
        
        if(W->getWinType() == WinType::ENEMY){
            W->move(0);
            replace_panel( this->panelLevel[ W->getPanelIndex() ], W->getTop() );
            move_panel( this->panelLevel[ W->getPanelIndex() ], W->getY(), W->getX());
        } else {
            move_panel( this->panelLevel[ W->getPanelIndex() ], W->getY(), W->getX());
        }
        
}


void Screen::shrinkWindow(Window* win)
{
    int oldWidth, oldHeight;
    WINDOW* oldWindow = panel_window( panelLevel[win->getPanelIndex()] );
    getmaxyx( oldWindow, oldHeight, oldWidth );
    WINDOW* smallerWindow = win->getSquishedWindow(win->getWidth() + win->getX());  // Because the x should be negative

    replace_panel( panelLevel[win->getPanelIndex()], smallerWindow );
    // if( oldWidth < win->getWidth()-1 )  // Need to delete the old window on top if it's not in the Window's->win vector (memory leak)
    // {
    //     delwin(oldWindow);
    //     cout << "Old width is: " << oldWidth << endl;
    //     cout << "Width is: " << win->getWidth() << endl;
    //     exit(0);
    // }
}

