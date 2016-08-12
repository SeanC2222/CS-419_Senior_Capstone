#include "screen.hpp"
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>

Screen::Screen(){
   this->bgWindow = NULL;
   this->hero = NULL;
   this->deathAnimation = NULL;
   this->dead = false;
   this->panelLevel;
   this->activeWindows;
}
   

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
    init_pair((int)color::HERO_A, COLOR_BLACK, COLOR_YELLOW);
    init_pair((int)color::HERO_F, COLOR_BLACK, COLOR_GREEN);
    init_pair((int)color::HERO_B, COLOR_WHITE, COLOR_BLACK);
    init_pair((int)color::HERO_SW, COLOR_WHITE, COLOR_BLUE);
    init_pair((int)color::HERO_DW, COLOR_WHITE, COLOR_BLUE);
    //DETAIL COLORS
    init_pair((int)color::DEATH_MAULED, COLOR_BLACK, COLOR_YELLOW);      //2
    init_pair((int)color::DEATH_FALL, COLOR_WHITE, COLOR_BLACK);         //5
    init_pair((int)color::DEATH_DROWN, COLOR_BLACK, COLOR_BLUE);         //6
    init_pair((int)color::JAVELIN_A, COLOR_RED, COLOR_YELLOW);             //3
    init_pair((int)color::JAVELIN_F, COLOR_RED, COLOR_GREEN);             //3
    init_pair((int)color::JAVELIN_B, COLOR_RED, COLOR_BLACK);             //3
    init_pair((int)color::JAVELIN_W, COLOR_RED, COLOR_BLUE);             //3
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


    cbreak();                   // Don't wait for user to hit 'enter'
    noecho();                   // Don't display user input on screen
    keypad(stdscr, TRUE);       // For getting arrow keys
    curs_set(0);                // Make the cursor invisible --Doesn't work :(
    this->bgIndex= 0;       // Scroll the bg file starting at col 0
    this->bgWindow = Window::getBackgroundFromFile(this->bgIndex);
    addToPanelLevel(this->bgWindow);
    loadDeathAnimations();
    this->level = 1;
    this->hero = NULL;
    this->area = color::ARENA;
    this->oldArea = 0;
    getmaxyx(stdscr, screenHeight, screenWidth);
    forestStart = 2045;
    sandStart = 3500;
    waterStart = 4000;
    waterMid = 5000;
    waterEnd = 6000;
    levelEnd = this->bgWindow->getWidth() - 20;
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
        this->area = ARENA;
    } else if (bgIndex >= forestStart && bgIndex < sandStart){
        this->area = FOREST;
    } else if (bgIndex >= sandStart && bgIndex < waterStart){
        this->area = BEACH;
    } else if (bgIndex >= waterStart && bgIndex < waterMid){
        this->area = SHALLOW_WATER;
    } else if (bgIndex >= waterMid && bgIndex < waterEnd){
        this->area = DEEP_WATER;
    } else if (bgIndex >= waterEnd && bgIndex < levelEnd){
        this->area = END;
    } else {
        this->area = ARENA;
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
    this->area = getArea();    
 
    if(this->area != this->oldArea){

	 this->oldArea = this->area;
	 if(this->area == ARENA){
	    setHeroColors(HERO_A);
	 } else if (this->area == FOREST){
	    setHeroColors(HERO_F);
	 } else if (this->area == BEACH){
	    setHeroColors(HERO_B);
	 } else if (this->area == SHALLOW_WATER){
	    //setHeroColors(HERO_SW);
	 } else if (this->area == DEEP_WATER){
	    //setHeroColors(HERO_DW);
	 } else if (this->area == END){
	    setHeroColors(HERO_A);
	 }
   }

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
            usleep(3000000);
            return(1);
        }
    }
        
    // Move any active (on screen) windows
    if( !dead )
    {
        
        bgIndex += level;
        
        bgWindow->showBgAt(bgIndex, area, forestStart, sandStart, waterStart, waterEnd);    // Scroll the background here
        if (bgIndex==levelEnd)  //max column size of bg file and should be updated accordingly
        {
            bgIndex=0;  
            level++;
        }

        if( (hero->getX() + hero->getWidth() + bgIndex >= waterStart-3) && (hero->getX() + hero->getWidth() + bgIndex < waterStart+1) )    // Rotate is called every 3 bg scrolls, so when in the range of the water, start swimming
            hero->startSwimming();
            
        if( (hero->getX() + hero->getWidth() + bgIndex >= waterEnd) && (hero->getX() + hero->getWidth() + bgIndex < waterEnd +4) )
            hero->stopSwimming();
        for(unsigned int i = 0; i < activeWindows.size(); i++){       // Move all the other windows
        
	    if(activeWindows[i]->getWinType()==WinType::JAVELIN){
		  int jXpos=activeWindows[i]->getX();
		  jXpos+=3;
		  activeWindows[i]->setX(jXpos);
		  move_panel(panelLevel[activeWindows[i]->getPanelIndex()], activeWindows[i]->getY(), activeWindows[i]->getX());	
		  if(activeWindows[i]->getX() + activeWindows[i]->getWidth() >=screenWidth){
		     removeFromScreen(activeWindows[i]);
		     i--;
		     continue;
		  } 
	    } else if( activeWindows[i]->getWinType() != WinType::ENEMY)
            {
                if( activeWindows[i]->move(level) )    // Returns true if the top window changed (an animated non-enemy...don't think we have any)
                    replace_panel(panelLevel[activeWindows[i]->getPanelIndex()], activeWindows[i]->getTop());
            } else if (activeWindows[i]->getWinType() == WinType::BACKGROUND || activeWindows[i]->getWinType() == WinType::HERO){
	       //DON'T AFFECT BACKGROUND
	       continue;
            } else                     // All enemies are animated
            {
                ((Enemy*)activeWindows[i])->move("left", level);
                replace_panel(panelLevel[activeWindows[i]->getPanelIndex()], activeWindows[i]->getTop());
	        show_panel(panelLevel[activeWindows[i]->getPanelIndex()]);
            }
	    
            if(activeWindows[i]->getX() < 0)    // If at the left edge, shrink or remove it.
            {
                if(activeWindows[i]->getX() + activeWindows[i]->getWidth() <= 1)           
                {                                                   
                    removeFromScreen(activeWindows[i]);                        
		    i--;
                    continue;     // Don't bother moving the panel
                }
                //else
                    shrinkWindow(activeWindows[i]);
            }
           // If at the right edge, show it.                
            if(activeWindows[i]->getWinType() != WinType::BACKGROUND){
	       move_panel( panelLevel[activeWindows[i]->getPanelIndex()], activeWindows[i]->getY(), activeWindows[i]->getX() );
	       int endOfPanel = activeWindows[i]->getX() + activeWindows[i]->getWidth(); 
	       if( (endOfPanel >= screenWidth - 2*level) && (endOfPanel <= screenWidth) ) 
		   show_panel( panelLevel[activeWindows[i]->getPanelIndex()] );
	    }
        }
        if(bgIndex%5 == 0)
            hero->rotate();
        if(hero->getTop()){
        }
        if(hero->getPanelIndex()){
	}
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
        int area = this->getArea();
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
    int imageIndex = image->getPanelIndex();
    del_panel(this->panelLevel[imageIndex]);
    this->panelLevel.erase( this->panelLevel.begin() + imageIndex);
    for(unsigned int i=0; i<activeWindows.size(); i++)
    {
        if(activeWindows[i] == image)
        {
            activeWindows.erase( activeWindows.begin() + i );   // Ridiculous syntax...you must use an iterator?!
	    i--;

        } else if (activeWindows[i]->getPanelIndex() >= imageIndex){

	    activeWindows[i]->setPanelIndex(activeWindows[i]->getPanelIndex()-1);

        }
        
    }
    if(this->hero->getPanelIndex() >= imageIndex){
      this->hero->setPanelIndex(this->hero->getPanelIndex()-1);
    }
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
                del_panel( panelLevel[hero->getPanelIndex()] );
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
                del_panel( panelLevel[hero->getPanelIndex()] );
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
		            removeFromScreen(screenElement);
		            removeFromScreen(anotherScreenElement);
		            //return;
		        }
	        }
	       } 
	    }
    }
}



Hero* Screen::getHero()
{
    return this->hero;
}

Hero* Screen::setHeroColors(int colors){

    int curX, curY, winNum;
    bool s, j, c;

    if(this->hero != NULL){
      //Hero exists already
      curX = this->hero->getX();
      curY = this->hero->getY();
      winNum = this->hero->getWinNum();
      s = this->hero->getSwimming();
      j = this->hero->getJumping();
      c = this->hero->getClimbing();
      del_panel(this->panelLevel[this->hero->getPanelIndex()]);
      for(int i = 0 ; i < activeWindows.size(); i++){
	 if(activeWindows[i]->getPanelIndex() >= this->hero->getPanelIndex()){
	    activeWindows[i]->setPanelIndex(activeWindows[i]->getPanelIndex()-1);
	 }
      }
      this->panelLevel.erase(this->panelLevel.begin() + this->hero->getPanelIndex());
      delete this->hero;
      this->hero = NULL;
    } else {
      curX = 10;
      curY = 10;
      s = false;
      j = false;
      c = false;
      winNum = 0;
      if(this->getArea() == SHALLOW_WATER || this->getArea() == DEEP_WATER){
	 s = true;
      }
    }


    vector<string> walkFiles = {"gladiatorFacing.txt", "gladiatorStep.txt", "gladiatorBack.txt", "gladiatorStep.txt"};
    vector<string> jumpFiles = {"jump1.txt", "jump2.txt", "jump3.txt", "jump4.txt", "jump5.txt", "jump6.txt", "jump7.txt", "jump8.txt"};
    vector<string> swimFiles = {"swimming1.txt", "swimming2.txt", "swimming3.txt", "swimming4.txt", "swimming5.txt", "swimming6.txt", "swimming7.txt", "swimming8.txt"};
    vector<string> upFiles = {"getUp1.txt", "getUp2.txt", "getUp3.txt"};
    Hero* heroWindow = new Hero(walkFiles[0], 200, 200, WinType::HERO, COLOR_PAIR(colors));  

    for(unsigned int i = 1; i < walkFiles.size(); i++) 
        heroWindow->appendAnimation( walkFiles[i], COLOR_PAIR(colors) );
    // Load the other movements
    for(auto fname : jumpFiles)
        heroWindow->appendOtherAnimation(fname, "jump", COLOR_PAIR(colors));
    for(auto fname : swimFiles)
        heroWindow->appendOtherAnimation(fname, "swim", COLOR_PAIR(HERO_SW));
    for(auto fname : upFiles)
        heroWindow->appendOtherAnimation(fname, "climb", COLOR_PAIR(colors));
  
    heroWindow->setX(curX);
    heroWindow->setY(curY);
    heroWindow->setTopWindow(winNum);
    heroWindow->setSwimming(s);
    heroWindow->setJumping(j);
    heroWindow->setClimbing(c);
 
    addToPanelLevel(heroWindow);
    hide_panel( this->panelLevel[heroWindow->getPanelIndex()]);

  
    heroWindow->saveScreenLimits(bgWindow->getWidth(), bgWindow->getHeight());
    
    move_panel(panelLevel[heroWindow->getPanelIndex()], heroWindow->getY(), heroWindow->getX());
    show_panel(panelLevel[heroWindow->getPanelIndex()]);
    this->hero = heroWindow;
    return heroWindow;


}
   
bool Screen::moveEnemy(Enemy* e, std::string dir, int speed){

   for( Window* w : activeWindows){
      if(e == (Enemy*)w){
	 return e->move(dir, speed);
      }
   }
   return false;
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
    hide_panel( this->panelLevel[baseWindow->getPanelIndex()]);
    
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

