#ifndef __WINDOW_H__
#define __WINDOW_H__

#include <string>
#include <ncurses.h>
#include <vector>

using namespace std;

class Window
{
    private:
    
    // Probably need to add array of color pairs for the levels here
        vector <WINDOW*> win;
        int height, width, x, y, pIdx;  // pIdx is the panel to which the BASE window belongs
                                        // The variables x and y are storing the x and y values of the PANEL in which the window is set
        int currentWindowIndex, nextWindowIndex;                       // Used to toggle hidden windows to show animations
        WINDOW* top;                    // Used to track the currently visible window in animations
        WINDOW* getWinFromFile(string filename, int xValue, int yValue, unsigned int colorScheme);

    public:
        Window(int height, int width, int xStart, int yStart);
        Window(string filename, int xStart, int yStart);
        void showBorder();
        static Window* getBackground(int level);
        void setPanelIndex(int PanelNum);
        int getPanelIndex();
        int getX();
        void setX(int newX);
        int getY();
        void setY(int newY);
        int getWidth();
        int getHeight();
        WINDOW* appendAnimation(string filename);
        WINDOW* getTop();
        bool isAnimated();
        void rotate();

};
#endif