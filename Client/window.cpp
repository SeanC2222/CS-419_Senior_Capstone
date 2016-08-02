#include "window.hpp"
#include <iostream>
#include <fstream>
#include <stdlib.h>


Window::Window(int height, int width, int xStart, int yStart) : height(height), width(width), x(xStart), y(yStart)
{
    this->win.push_back( newwin(height, width, x, y) );
    this->nextWindowIndex = this->currentWindowIndex = 0;
    this->top = this->win[0];
    this->type = WinType::BACKGROUND;
}

/* Creates a new window with the text from a file.
 * 
                                    * For the color thing...pass in a COLOR_PAIR ? or do we build these things with 
                                    * two colors (first and last char in a row and col is black for an outline maybe) or what?
 */
Window::Window(string filename, int xStart, int yStart, WinType type) : x(xStart), y(yStart), type(type)
{
    WINDOW* newWindow = getWinFromFile(filename, xStart, yStart, COLOR_PAIR(2));
    this->win.push_back( newWindow );
    getmaxyx(newWindow, this->height, this->width);
    this->nextWindowIndex = this->currentWindowIndex = 0;
    this->top = this->win[0];
}


                                                                         /***NEED TO SET A FIXED COLOR SCHEME, UPDATE THIS***/
WINDOW* Window::appendAnimation(string filename)
{
    WINDOW* frame = getWinFromFile(filename, this->x, this->y, (COLOR_PAIR(2)));
    this->win.push_back(frame);
    this->nextWindowIndex = 1;          // This will only be called during initialization, so the "first" next index will always be 1
    return frame;
}



// Only left in for debugging purposes
void Window::showBorder()
{
    box(this->getTop(), 0, 0);
    wrefresh(this->getTop());
}

/*MARTHA: I COMMENTED OUT ORIGINAL getBackground FUNCTION*/
/* Constructs a background window.  Because it didn't seem right to
   make a constructor with a confusing single parameter or extend the 
   class just to make an appropriately named constructor... 
*/
/*Window* Window::getBackground(int level)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    Window* bg = new Window(rows, cols, 0, 0);              // ****NEED TO DELETE LATER***
    switch(level)
    {
        case 1:
        {
            WINDOW* bgWin = bg->getTop();
            int grassValue = 5;                         // The amount of grass on each line
            wattron(bgWin, COLOR_PAIR(1));
            for(int i = 0; i < rows; i++)
            {
                string bgLine(cols, ' ');          // A row of spaces the width of the screen to be filled in with the bg color yellow
                for(int j = 0; j < grassValue; j++)
                    bgLine[rand() % cols] = 'v';        // Add a green 'v' to represent grass at several random points in the line
                mvwprintw(bgWin, i, 0, bgLine.c_str());                 // Print the line, repeat for all lines
            }
            wattroff(bgWin, COLOR_PAIR(1));
            break;
        }
        default:
            cerr << "No Background Level Selected";
            exit(1);
    }
    return bg;
}*/

Window* Window::getBackground(int level, int k=0)
{
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    Window* bg = new Window(rows, cols, 0, 0);              
    switch(level)
    {
        case 1:
        {
            WINDOW* bgWin = bg->getTop();
            wattron(bgWin, COLOR_PAIR(1));
            for(int i = 0; i < rows; i++)
            {
                int j=0;
                string bgLine(cols, ' ');          // A row of spaces the width of the screen to be filled in with the bg color yellow
                for(int m = k; m < cols; m++){
                    if (i%2==0){
                        if (m%2==0)
                           bgLine[j]='-';
                        else
                           bgLine[j]=' ';
                    }
                    else{
                        if (m%2==0)
                           bgLine[j]=' ';
                        else
                           bgLine[j]='-';
                    }
                    for(int m=cols-1; m>cols-k; m--)
                        bgLine[m]='o';
                    j++;
                mvwprintw(bgWin, i, 0, bgLine.c_str());
                // Print the line, repeat for all lines
                }
            }
            wattroff(bgWin, COLOR_PAIR(1));
            break;
        }
        default:
            cerr << "No Background Level Selected";
            exit(1);
    }
    return bg;
}


//MARTHA: To grab background from file in Images
Window* Window::getBackgroundFromFile(int k)
{
    string path = "./Images/background.txt";
    ifstream inputFile;
    inputFile.open(path);
    
    if(!inputFile.is_open())
    {
       cerr << "Couldn't open file: " << path;
       exit(1);
    }
    
    else
    {
        string line;
        int screenXSize, screenYSize;
        string::size_type cols=0;
        int rows = 0;
        while( getline(inputFile, line) )        // Get the max cols and number of rows from the file to create a window the right size
        {
            rows++;
            cols = ( (line.length() > cols) ? line.length() : cols );
        }
    
        getmaxyx(stdscr, screenYSize, screenXSize);
      	Window* bg = new Window (rows, cols, 0, 0);
        WINDOW* bgFromFile = bg->getTop();
        inputFile.clear();               // "Unlocks" the file for processing after reaching EOF
        inputFile.seekg(0);             // Return to the beginning of the file



        wattron(bgFromFile, COLOR_PAIR(1));
        for (int r=0; r<screenYSize; r++)
        { // Get the file and put it in the window
	        //string line1(cols, ' ');	                ***Removed by Dylan...do we need this?
            if( getline(inputFile, line) ){
                string temp=line;
	  	        line.erase(line.begin(), line.begin()+k);
		        temp.erase(temp.begin()+k, temp.end());
		        line.insert(line.length()-1,temp);
		        //mvwprintw(bgFromFile, r, 0, line1.c_str());     *** and this? it works without it.
                mvwprintw(bgFromFile, r, 0, line.c_str());
	        }
        }        
        wattroff(bgFromFile, COLOR_PAIR(1));
        wrefresh(bgFromFile);
        return bg;
    }
}


WINDOW* Window::getWinFromFile(string filename, int xStart, int yStart, unsigned int colorScheme)
{
    string path = "./Images/" + filename;
    ifstream inputFile;
    inputFile.open(path);
    
    if(!inputFile.is_open())
    {
       cerr << "Couldn't open file: " << path;
       exit(1);
    }
    
    else
    {
        string line;
        int screenXSize, screenYSize;
        string::size_type cols=0;
        int rows = 0;
        while( getline(inputFile, line) )        // Get the max cols and number of rows from the file to create a window the right size
        {
            rows++;
            cols = ( (line.length() > cols) ? line.length() : cols );
        }
    
        getmaxyx(stdscr, screenYSize, screenXSize);
        
        if(rows > screenYSize || cols > (unsigned int)screenXSize)
        {
            cerr << "The contents of the file <" << filename << "> are too large to fit on the screen.";
            exit(1);
        }
        
        WINDOW* windowFromFile = newwin(rows, cols, xStart, yStart);
        
        inputFile.clear();               // "Unlocks" the file for processing after reaching EOF
        inputFile.seekg(0);             // Return to the beginning of the file
        rows=0;
        
        wbkgd(windowFromFile, colorScheme);        // Fills in the background color where spaces weren't entered 

        unsigned int currentScheme = colorScheme;
        wattron(windowFromFile, colorScheme);
        int j = 0;
        while( getline(inputFile, line) ){        // Get the file and put it in the window
            j=0;
            for (unsigned int i=0; i<line.length(); i++, j++){
                if( (int)line[i] > 47 || (int)line[i] < 58 )     // If values are ASCII numbers
                {
                    switch((int)line[i])
                    {       // 0 in file, the rest of the numbers follow
                        case 48: 
                                j--;
                                wattroff(windowFromFile, currentScheme);    // just swaps with previous scheme
                                wattron(windowFromFile, colorScheme);                            
                                break;
                        case 49: j--;
                                currentScheme = COLOR_PAIR(1);
                                wattron(windowFromFile, COLOR_PAIR(1));
                                break;
                        case 50: j--;
                                currentScheme = COLOR_PAIR(2);
                                wattron(windowFromFile, COLOR_PAIR(2));
                                break;
                        case 51: j--;
                                currentScheme = COLOR_PAIR(3);
                                wattron(windowFromFile, COLOR_PAIR(3));
                                break;
                        case 52:j--;
                                currentScheme = COLOR_PAIR(4);
                                wattron(windowFromFile, COLOR_PAIR(4));
                                break;
                        case 53:j--;
                                currentScheme = COLOR_PAIR(5);
                                wattron(windowFromFile, COLOR_PAIR(5));
                                break;
                        case 54: j--;
                                currentScheme = COLOR_PAIR(6);
                                wattron(windowFromFile, COLOR_PAIR(6));
                                break;
                        default:
                            mvwaddch(windowFromFile, rows, j, line[i]);
                    }
                }
                else
                    mvwaddch(windowFromFile, rows, j, line[i]);

            //   if (line[i]=='w'||line[i]=='L'){ //apply specific colors for specific characters
            //   wattron(windowFromFile, COLOR_PAIR(3));
            //   }
            //   else{
            //   wattron(windowFromFile, COLOR_PAIR(2));
            //   char * ptr1=&line[i];
            //   mvwaddch(windowFromFile, rows, i, line[i]);
            //   }
            }
        rows++;
        }
        //mvwprintw(windowFromFile, rows++, 0, line.c_str());

        wattroff(windowFromFile, currentScheme);
       
        return windowFromFile;
    }
}

void Window::rotate()
{
    this->top = win[ this->nextWindowIndex ];                       // The current top window is now the next window in the array
    this->currentWindowIndex = this->nextWindowIndex;               // Update the currentWindow Index to reflect that change
    if((unsigned int)this->nextWindowIndex + 1 >= this-> win.size() ) 
        this->nextWindowIndex = 0;                                  // If the next index value would be more than the # of windows in the vector
    else                                                            // Reset the next window to be the first window in the vector,  otherwise increment
        this->nextWindowIndex++;
}

void Window::setPanelIndex(int PanelNum)
{
    this->pIdx = PanelNum;
}

int Window::getBasePanelIndex()
{
    return this->pIdx;
}

int Window::getPanelIndex()
{
    if(this->isAnimated())
        return(this->pIdx + this->currentWindowIndex);
    else
        return this->pIdx;
}

int Window::getX()
{
    return this->x;
}

void Window::setX(int newX)
{
    this->x = newX;
}

int Window::getY()
{
    return this->y;
}

void Window::setY(int newY)
{
    this->y = newY;
}

int Window::getWidth()
{
    return this->width;
}

int Window::getHeight()
{
    return this->height;
}

WINDOW* Window:: getTop()
{
    return this->top;
}

bool Window::isAnimated()
{
    return (this->win.size() > 1);
}

WinType Window::getWinType()
{
    return this->type;
}