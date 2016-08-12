#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <string>
#include <ncurses.h>
#include <vector>

using namespace std;

enum class WinType { BACKGROUND, ENEMY, WALL, HERO, PIT, DEATH, JAVELIN, SCORE};
enum color { ARENA = 1, FOREST, BEACH, SHALLOW_WATER, DEEP_WATER, END,
                   HERO_ARENA, HERO_FOREST, HERO_BEACH, HERO_SHALLOW_WATER, HERO_DEEP_WATER,
                   DEATH_MAULED, DEATH_FALL, DEATH_DROWN, JAVELIN, SCORE,
                   ARENA_ENEMY_ONE, ARENA_ENEMY_TWO, FOREST_ENEMY_ONE, FOREST_ENEMY_TWO, BEACH_ENEMY, WATER_ENEMY_ONE, WATER_ENEMY_TWO,
                   PIT_A, PIT_F, PIT_B, POOL,
                   MENU_ONE, MENU_TWO, MENU_THREE,
                   GW_ONE, GW_TWO, GW_THREE, GW_FOUR, GW_FIVE, GW_SIX, GW_SEVEN, GW_EIGHT };


class Window
{
    private:
    
    // Probably need to add array of color pairs for the levels here
        int height, width, x, y, pIdx;  // pIdx is the panel to which the BASE window belongs
                                        // The variables x and y are storing the x and y values of the PANEL in which the window is set
        WinType type;
    protected:
        int windowIndex, screenBottomLimit, screenRightLimit; 
        WINDOW* top;                    // Used to track the currently visible window in animations
        vector <WINDOW*> win;

    public:
        virtual ~Window();
        Window(int height, int width, int xStart, int yStart);
        Window(string filename, int xStart, int yStart, WinType type, int colorScheme);
        WINDOW* getWinFromFile(string filename, int xValue, int yValue, unsigned int colorScheme);
        static Window* getBackground(int level, int j);
        static Window* getBackgroundFromFile(int k);
        void setPanelIndex(int PanelNum);
        int getBasePanelIndex();
        int getPanelIndex();
        void saveScreenLimits(int width, int height);
        int getX();
        void setX(int newX);
        int getY();
        void setY(int newY);
        int getWidth();
        int getHeight();
        void appendAnimation(string filename, int colorScheme);
        WINDOW* getTop();
        bool isAnimated();
        virtual void rotate();
        WinType getWinType();
        bool isLastAnimationFrame();
        virtual bool move(int speed);
        WINDOW* getSquishedWindow(int howSmall);
        void showBgAt(int k, int area, int forestStart, int sandStart, int waterStartIdx, int waterEndIdx);
};
#endif