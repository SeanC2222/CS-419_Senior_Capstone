#ifndef PANEL_LEVELS
#define PANEL_LEVELS 4
#endif

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "window.hpp"
#include <vector>
#include <panel.h>
#include <string>
#include <unordered_map>


class Screen
{
     private:
        vector<PANEL*> panelLevel;
        vector<Window*> activeWindows;
        //std::vector<Window*> movingEnemies;       Maybe for later
        vector<Window*> movingWindows;
        unordered_map <string, Window*> deathAnimations;
        Window* bgWindow;
        Window* hero;
        Window* deathAnimation;
        int level, bgIndex;
        void addToPanelLevel(Window* image);
        bool dead;
     
     public:
        void init();
        void update(int frame);
        //void scrollBG(Window* bgWindow);
        void scrollBg(int startCol);    
        void move(std::string direction, Window* baseW);
        void cleanup();
        Window* loadImages(vector<string> filenames, WinType type);
        Window* getBG();
        void playAnimation(Window* win, int x, int y);
        void putOnScreen(Window* image, int X, int Y);
        Window* loadHero( vector<string> filenames, int x, int y );
        void removeFromScreen(Window* image);
        void loadDeathAnimations();
        void checkIfDead();

};


#endif  // __SCREEN_H__