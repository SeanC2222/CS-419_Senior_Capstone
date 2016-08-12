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
    
    //Level Detail Colors
    init_pair((int)color::ARENA, COLOR_GREEN, COLOR_YELLOW);             //1
    init_pair((int)color::FOREST, COLOR_YELLOW, COLOR_GREEN);            //18
    init_pair((int)color::BEACH, COLOR_MAGENTA, COLOR_BLACK);            //17
    init_pair((int)color::DEEP_WATER, COLOR_BLACK, COLOR_BLUE);          //6
    init_pair((int)color::SHALLOW_WATER, COLOR_WHITE, COLOR_BLUE);       //7
    init_pair((int)color::END, COLOR_MAGENTA, COLOR_YELLOW);             //9
    //HERO COLORS
    init_pair((int)color::HERO_ARENA, COLOR_BLACK, COLOR_YELLOW);
    init_pair((int)color::HERO_FOREST, COLOR_BLACK, COLOR_GREEN);
    init_pair((int)color::HERO_BEACH, COLOR_WHITE, COLOR_BLACK);
    init_pair((int)color::HERO_SHALLOW_WATER, COLOR_BLACK, COLOR_BLUE);
    init_pair((int)color::HERO_DEEP_WATER, COLOR_BLACK, COLOR_BLUE);
    //DETAIL COLORS
    init_pair((int)color::DEATH_MAULED, COLOR_BLACK, COLOR_YELLOW);      //2
    init_pair((int)color::DEATH_FALL, COLOR_WHITE, COLOR_BLACK);         //5
    init_pair((int)color::DEATH_DROWN, COLOR_BLACK, COLOR_BLUE);         //6
    init_pair((int)color::JAVELIN, COLOR_RED, COLOR_YELLOW);             //3
    //SCORE COLORS
    init_pair((int)color::SCORE, COLOR_RED, COLOR_BLACK);                //10
    //ENEMY COLORS
    init_pair((int)color::ARENA_ENEMY_ONE, COLOR_BLACK, COLOR_YELLOW);     //11
    init_pair((int)color::ARENA_ENEMY_TWO, COLOR_RED, COLOR_YELLOW);      //3
    init_pair((int)color::FOREST_ENEMY_ONE, COLOR_BLACK, COLOR_GREEN);     //19
    init_pair((int)color::FOREST_ENEMY_TWO, COLOR_RED, COLOR_GREEN);      //20
    init_pair((int)color::BEACH_ENEMY, COLOR_RED, COLOR_BLACK);           //10
    init_pair((int)color::WATER_ENEMY_ONE, COLOR_GREEN, COLOR_BLUE);     //12
    init_pair((int)color::WATER_ENEMY_TWO, COLOR_CYAN, COLOR_BLUE);      //13
    //PIT COLORS
    init_pair((int)color::PIT_A, COLOR_MAGENTA, COLOR_YELLOW);         //9
    init_pair((int)color::PIT_F, COLOR_WHITE, COLOR_GREEN);           //4
    init_pair((int)color::PIT_B, COLOR_MAGENTA, COLOR_BLACK);           //4
    init_pair((int)color::POOL, COLOR_CYAN, COLOR_BLUE);                 //17
    //MENU PAIRS
    init_pair((int)color::MENU_ONE,COLOR_GREEN, COLOR_BLACK);             //14
    init_pair((int)color::MENU_TWO,COLOR_YELLOW, COLOR_BLACK);           //15
    init_pair((int)color::MENU_THREE,COLOR_RED, COLOR_BLACK);            //16
    //GET WIN FROM FILE PAIRS COLORS
    init_pair((int)color::GW_ONE, COLOR_GREEN, COLOR_YELLOW);            //1
    init_pair((int)color::GW_TWO, COLOR_BLACK, COLOR_YELLOW);            //2
    init_pair((int)color::GW_THREE, COLOR_RED, COLOR_YELLOW);            //3
    init_pair((int)color::GW_FOUR, COLOR_WHITE, COLOR_YELLOW);           //4
    init_pair((int)color::GW_FIVE, COLOR_WHITE, COLOR_BLACK);            //5
    init_pair((int)color::GW_SIX, COLOR_BLACK, COLOR_BLUE);              //6
    init_pair((int)color::GW_SEVEN, COLOR_WHITE, COLOR_BLUE);            //7
    init_pair((int)color::GW_EIGHT, COLOR_RED, COLOR_BLUE);              //8


    cbreak();                   // Don't wait for user to hit 'enter'
    noecho();                   // Don't display user input on screen
    keypad(stdscr, TRUE);       // For getting arrow keys
    curs_set(0);                // Make the cursor invisible --Doesn't work :(

    this->bgWindow = Window::getBackgroundFromFile(0);
    addToPanelLevel(this->bgWindow);
    loadDeathAnimations();
    this->level = 1;
    this->area = color::ARENA;
    this->bgIndex= 0;       // Scroll the bg file starting at col 0
    getmaxyx(stdscr, screenHeight, screenWidth);
    forestStart = 2045;
    sandStart = 3500;
    waterStart = 4000;
    waterMid = 5000;
    waterEnd = 5800;
    deathAnimation = NULL;
    dead = false;
    deathFrame = 1;
    
    // I'm thinking maybe we put createBackgroundElements(int level)
    // and createOpponents(int level) in this module and then call them here...
}

/**
 * Returns the current level dictated by background
 **/
int Screen::getLevel()
{
    return this->level;
}

/**
 * Returns the current level dictated by background
 **/
int Screen::getArea()
{
    if(bgIndex < forestStart){
        this->area = color::ARENA;
    } else if (bgIndex >= forestStart && bgIndex < sandStart){
        this->area = color::FOREST;
    } else if (bgIndex >= sandStart && bgIndex < waterStart){
        this->area = color::BEACH;
    } else if (bgIndex >= waterStart && bgIndex < waterMid){
        this->area = color::SHALLOW_WATER;
    } else {
        this->area = color::DEEP_WATER;
    }
    return this->area;
}

bool Screen::heroIsAlive()
{
    return !this->dead;
}

int Screen::getScreenWidth(){
    return this->screenWidth;
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
 
    this->area = getArea();    
 
    if( dead )
    {
        this->deathFrame++;
        playDeathScene();
        if( deathAnimation->isLastAnimationFrame() )
        {
            update_panels();
            doupdate();
            usleep(3000000);
            return(1);
        }
    }
        
    // Move any active (on screen) windows
    if( !dead )
    {
        
        bgIndex += level;
        
        bgWindow->showBgAt(bgIndex, area, forestStart, sandStart, waterStart, waterEnd);    // Scroll the background here
        if (bgIndex==5800)  //max column size of bg file and should be updated accordingly
        {
            bgIndex=0;  
            level++;
        }

        if( (hero->getX() + hero->getWidth() + bgIndex >= waterStart-3) && (hero->getX() + hero->getWidth() + bgIndex < waterStart+1) )    // Rotate is called every 3 bg scrolls, so when in the range of the water, start swimming
            hero->startSwimming();
            
        if( (hero->getX() + hero->getWidth() + bgIndex >= waterEnd) && (hero->getX() + hero->getWidth() + bgIndex < waterEnd +4) )
            hero->stopSwimming();

        for( auto* w : activeWindows)       // Move all the other windows
        {
            if(w->getWinType()==WinType::JAVELIN){
        		int jXpos=w->getX();
        		jXpos+=3;
        		w->setX(jXpos);
        		move_panel(panelLevel[w->getPanelIndex()], w->getX(), w->getY());	
        		if(w->getX() + w->getWidth() >=screenWidth){
        		   removeFromScreen(w);
        		   return 0;
		        } 
	        }
            if( w->getWinType() != WinType::ENEMY )
            {
                if( w->move(level) )    // Returns true if the top window changed (an animated non-enemy...don't think we have any)
                    replace_panel(panelLevel[w->getPanelIndex()], w->getTop());
            }
            else                        // All enemies are animated
            {
                ((Enemy*)w)->move("left", level);
                replace_panel(panelLevel[w->getPanelIndex()], w->getTop());
            }

            if(w->getX() < 0)    // If at the left edge, shrink or remove it.
            {
                if(w->getX() + w->getWidth() <= 0)           
                {                                                   
                    removeFromScreen(w);                        
                    continue;     // Don't bother moving the panel
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
    
    deathAnimations["mauled"] = loadImages(mauledFiles, WinType::DEATH, COLOR_PAIR(color::DEATH_MAULED));
    deathAnimations["fall"] = loadImages(fallingFiles, WinType::DEATH, COLOR_PAIR(color::DEATH_FALL));
    deathAnimations["drown"] = loadImages(drowningFiles, WinType::DEATH, COLOR_PAIR(color::DEATH_DROWN));
}


void Screen::playDeathScene()
{
    if(deathFrame%5 == 0)
    {   
        deathAnimation->rotate();                                                   // Changes the top window index
        replace_panel( panelLevel[ deathAnimation->getPanelIndex() ], deathAnimation->getTop() );
        move_panel( panelLevel[deathAnimation->getPanelIndex()], deathAnimation->getY(), deathAnimation->getX() );

    }
}


void Screen::putOnScreen(Window* image, int X, int Y)
{
    if(image->getWinType()==WinType::JAVELIN){
    	int heroX=hero->getX()+3;
    	int heroY=hero->getY()+3;	
    	image->setX(heroX);
    	image->setY(heroY);
    } else{
        image->setX(X);
        image->setY(Y);
    }
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
    this->panelLevel.erase( this->panelLevel.begin() + image->getPanelIndex());
    delete image;
    
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
        else if (screenElement -> getWinType() == WinType::JAVELIN){
    	   for ( Window* anotherScreenElement :  activeWindows){
	        if (anotherScreenElement->getWinType() == WinType::ENEMY){
		        if(screenElement -> getX()>= anotherScreenElement ->getX()&&screenElement ->getY()<=anotherScreenElement->getY()+anotherScreenElement->getHeight()&&screenElement->getY()>=anotherScreenElement->getY()){
		            removeFromScreen(anotherScreenElement);
		            removeFromScreen(screenElement);
		            //return;
		        }
	        }
	       } 
	    }
    }
}



Hero* Screen::getHero(int x, int y )
{
    // Load the main walking files
    vector<string> walkFiles = {"gladiatorFacing.txt", "gladiatorStep.txt", "gladiatorBack.txt", "gladiatorStep.txt"};
    Hero* heroWindow = new Hero(walkFiles[0], 200, 200, WinType::HERO, COLOR_PAIR(color::HERO_ARENA));  // For some reason if I add it at 0, 0, it still shows up even with the hide_panel call below
    addToPanelLevel(heroWindow);
    hide_panel( this->panelLevel[ panelLevel.size()-1 ] );
    for(unsigned int i = 1; i < walkFiles.size(); i++) 
        heroWindow->appendAnimation( walkFiles[i], COLOR_PAIR(color::HERO_ARENA) );
    
    // Load the other movements
    vector<string> jumpFiles = {"jump1.txt", "jump2.txt", "jump3.txt", "jump4.txt", "jump5.txt", "jump6.txt", "jump7.txt", "jump8.txt"};
    vector<string> swimFiles = {"swimming1.txt", "swimming2.txt", "swimming3.txt", "swimming4.txt", "swimming5.txt",
        "swimming6.txt", "swimming7.txt", "swimming8.txt"};
    vector<string> upFiles = {"getUp1.txt", "getUp2.txt", "getUp3.txt"};
    for(auto fname : jumpFiles)
        heroWindow->appendOtherAnimation(fname, "jump", COLOR_PAIR(color::GW_SIX));
    for(auto fname : swimFiles)
        heroWindow->appendOtherAnimation(fname, "swim", COLOR_PAIR(color::GW_SIX));
    for(auto fname : upFiles)
        heroWindow->appendOtherAnimation(fname, "climb", COLOR_PAIR(color::HERO_ARENA));
    
    heroWindow->saveScreenLimits(bgWindow->getWidth(), bgWindow->getHeight());
    
    heroWindow->setX(x);
    heroWindow->setY(y);
    move_panel(panelLevel[heroWindow->getPanelIndex()], heroWindow->getY(), heroWindow->getX());
    show_panel(panelLevel[heroWindow->getPanelIndex()]);
    this->hero = heroWindow;
    wrefresh(heroWindow->getTop());
    return heroWindow;
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
        baseWindow->appendAnimation( filenames[i], colors);

    return baseWindow;
}


Enemy* Screen::loadEnemy(vector<string> filenames, int colors)
{
    Enemy* base = new Enemy(filenames[0], 200, 200, WinType::ENEMY, colors);  // For some reason if I add it at 0, 0, it still shows up even with the hide_panel call below
    addToPanelLevel(base);
    hide_panel( this->panelLevel[ panelLevel.size()-1 ] );
    
    for(unsigned int i = 1; i < filenames.size(); i++) 
        base->appendAnimation( filenames[i], colors);
    
    base->saveScreenLimits(bgWindow->getWidth(), bgWindow->getHeight());

    return base;
}


void Screen::shrinkWindow(Window* win)
{
    int oldWidth, oldHeight;
    WINDOW* oldWindow = panel_window( panelLevel[win->getPanelIndex()] );
    getmaxyx( oldWindow, oldHeight, oldWidth );
    WINDOW* smallerWindow = win->getSquishedWindow(win->getWidth() + win->getX());  // Because the x should be negative

    replace_panel( panelLevel[win->getPanelIndex()], smallerWindow );
    if( oldWidth < win->getWidth()-1 ){  // Need to delete the old window on top if it's not in the Window's->win vector (memory leak)
        delwin(oldWindow);
    }
}

