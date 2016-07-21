#ifndef PANEL_LEVELS
#define PANEL_LEVELS 4
#endif

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include "window.hpp"
#include <vector>
#include <panel.h>
#include <string>


class Screen
{
     private:
        std::vector<PANEL*> panelLevel;
        std::vector<Window*> movingWindows;
        Window* bgWindow;
        int level;
        void addToPanelLevel(Window* image);
     
     public:
        void init();
        void update();
        //void scrollBG(Window* bgWindow);
        void scrollBg(int startCol);    
        int move(std::string direction, Window* image, bool bgElement);
        void cleanup();
        Window* loadImages(vector<string> filenames, int x, int y);
        Window* getBG();
};


#endif  // __SCREEN_H__