#ifndef PANEL_LEVELS
#define PANEL_LEVELS 4
#endif

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "window.hpp"
#include "winTypes.cpp"
#include <vector>
#include <panel.h>
#include <string>
#include <unordered_map>


class Screen
{
     private:
        vector<PANEL*> panelLevel;
        vector<Window*> activeWindows;
        unordered_map <string, Window*> deathAnimations;
        Window* bgWindow;
        Hero* hero;
        Window* deathAnimation;
        int level, bgIndex, deathFrame, screenHeight, screenWidth, waterStart, waterEnd;
        void addToPanelLevel(Window* image);
        bool dead;
     
     public:
        void init();
        int getLevel();
        int update();
        Window* loadImages(vector<string> filenames, WinType type, int colorScheme);
        Enemy* loadEnemy(vector<string> filenames, int colors);
        void playAnimation(Window* win, int x, int y);
        void putOnScreen(Window* image, int X, int Y);
        Hero* getHero(int x, int y);
        void removeFromScreen(Window* image);
        void loadDeathAnimations();
        void checkIfDead();
        void playDeathScene();
        void shrinkWindow(Window* win);
};


#endif  // __SCREEN_H__