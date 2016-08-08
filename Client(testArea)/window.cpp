#include "window.hpp"
#include <iostream>
#include <fstream>
#include <stdlib.h>


Window::Window(int height, int width, int xStart, int yStart) : height(height), width(width), x(xStart), y(yStart)
{
    this->win.push_back( newwin(height, width, x, y) );
    this->windowIndex = 0;
    this->top = this->win[0];
    this->type = WinType::BACKGROUND;
}

/* Creates a new window with the text from a file.
 * 
                                    * For the color thing...pass in a COLOR_PAIR ? or do we build these things with 
                                    * two colors (first and last char in a row and col is black for an outline maybe) or what?
 */
Window::Window(string filename, int xStart, int yStart, WinType type, int colorScheme) : x(xStart), y(yStart), type(type)
{
    WINDOW* newWindow = getWinFromFile(filename, xStart, yStart, colorScheme);
    this->win.push_back( newWindow );
    getmaxyx(newWindow, this->height, this->width);
    this->windowIndex = 0;
    this->top = this->win[0];
}

Window::~Window()
{
    for(unsigned int i=0; i < this->win.size(); i++)
    {
        delwin(win[i]);
    }
}

void Window::saveScreenLimits(int width, int height)
{
    screenBottomLimit = height;
    screenRightLimit = width;
}


                                                                         /***NEED TO SET A FIXED COLOR SCHEME, UPDATE THIS***/
void Window::appendAnimation(string filename, int colorScheme)
{
    WINDOW* frame = getWinFromFile(filename, this->x, this->y, colorScheme);
    this->win.push_back(frame);
    this->windowIndex = 1;          // This will only be called during initialization, so the "first" next index will always be 1
}

bool Window::isLastAnimationFrame()
{
    return( this->top == this->win[this->win.size()-1] );
}

WINDOW* Window::getSquishedWindow(int howSmall)
{
    int startCol = this->width - howSmall;
    

    WINDOW* squishedWindow = newwin(this->height, howSmall, this->y, 0); 

    if( copywin(this->top, squishedWindow, 0, startCol, 0, 0, this->height-1, howSmall-1, false) == ERR)
    {
        cerr << "Error copying shrunk window" << endl;
        cout << "height = " << this->height << endl;
        cout << "howsmall = " << howSmall << endl;
        cout << "width = " << this->width << endl;
        exit(1);
    }
    return squishedWindow;
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
        string::size_type cols=0;
        int rows = 0;
        while( getline(inputFile, line) )        // Get the max cols and number of rows from the file to create a window the right size
        {
            rows++;
            cols = ( (line.length() > cols) ? line.length() : cols );
        }

      	Window* bg = new Window (rows, cols, 0, 0);
        inputFile.close();               // "Unlocks" the file for processing after reaching EOF
    
        bg->showBgAt(0, 1000, 1000);
        return bg;
    }
}

void Window::showBgAt(int k, int waterStartIdx, int waterEndIdx)
{
    string path = "./Images/background.txt";
    string line;
    ifstream inputFile;
    inputFile.open(path);
    
    if(!inputFile.is_open())
    {
       cerr << "Couldn't open file: " << path;
       exit(1);
    }
     
    int screenXSize, screenYSize;
    getmaxyx(stdscr, screenYSize, screenXSize);
    
    WINDOW* bgFromFile = this->getTop();
    bool screenAllWater = ( k >= waterStartIdx && k <= waterEndIdx );
    // Here we would put -if only water level on screen, use COLOR_SCHEME(7)
    if(screenAllWater) 
        wattron(bgFromFile, COLOR_PAIR(7)); 
    else if(k < 508)
        wattron(bgFromFile, COLOR_PAIR(1));
    else
        wattron(bgFromFile, COLOR_PAIR(9));
    
    for (int r=0; r<screenYSize; r++)
    { // Get the file and put it in the window
        if( getline(inputFile, line) )
        {
            string temp=line;
  	        line.erase(line.begin(), line.begin()+k);
	        temp.erase(temp.begin()+k, temp.end());
	        line.insert(line.length()-1,temp);
	        
	        
    	    if( k < waterStartIdx && k + screenXSize > waterStartIdx)
            {
                wattroff(bgFromFile, COLOR_PAIR(7));
                wattron(bgFromFile, COLOR_PAIR(1));
                for(int col=0; col < screenXSize; col++)
                {
                    if (col == waterStartIdx-k)   
                    {
                        wattroff(bgFromFile, COLOR_PAIR(1));
                        wattron(bgFromFile, COLOR_PAIR(7));
                    }
                    mvwaddch(bgFromFile, r, col, line[col]);
                }
            }    // mvwchgat(bgFromFile, row, waterStartIdx-k , -1, A_INVIS, COLOR_PAIR(7), NULL);
            else if( k < waterEndIdx && k + screenXSize > waterEndIdx)
            {    
                wattroff(bgFromFile, COLOR_PAIR(1));
                wattron(bgFromFile, COLOR_PAIR(7));
                for(int col=0; col < screenXSize; col++)
                {
                    if (col == waterEndIdx-k)   
                    {
                        wattroff(bgFromFile, COLOR_PAIR(7));
                        wattron(bgFromFile, COLOR_PAIR(1));
                    }
                    mvwaddch(bgFromFile, r, col, line[col]);
                }                
            }    
	        else if(!screenAllWater && k >= 508 && r == 29 )
	        {
	            wattroff(bgFromFile, COLOR_PAIR(9));
	            wattron(bgFromFile, COLOR_PAIR(1));
	        }
	        else
                mvwprintw(bgFromFile, r, 0, line.c_str());
        }
    }   
    if(screenAllWater)
        wattroff(bgFromFile, COLOR_PAIR(7));
    else if(k < 508)
        wattroff(bgFromFile, COLOR_PAIR(1));
    else 
        wattroff(bgFromFile, COLOR_PAIR(9));        

    wrefresh(bgFromFile);
    inputFile.close();
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
    
    string line;
    int screenXSize, screenYSize;
    int cols=0;
    int maxCols = 0;
    int rows = 0;
    while( getline(inputFile, line) )        // Get the max cols and number of rows from the file to create a window the right size
    {
        rows++;
        cols = 0;

        for( char c : line )
        {
            if( c < 48 || c > 57)
                cols++;
        }
        maxCols = (cols > maxCols ? cols : maxCols);
    }

    getmaxyx(stdscr, screenYSize, screenXSize);
    
    if(rows > screenYSize || maxCols > screenXSize)
    {
        cerr << "The contents of the file <" << filename << "> are too large to fit on the screen.";
        exit(1);
    }
    
    WINDOW* windowFromFile = newwin(rows, maxCols, yStart, xStart);
    
    inputFile.clear();               // "Unlocks" the file for processing after reaching EOF
    inputFile.seekg(0);             // Return to the beginning of the file
    rows=0;
    
    wbkgd(windowFromFile, colorScheme);        // Fills in the background color where spaces weren't entered 

    unsigned int currentScheme = colorScheme;
    unsigned int prevScheme = colorScheme;
    unsigned int temp = 0;
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
                            temp = prevScheme;
                            prevScheme = currentScheme;
                            currentScheme = temp;
                            wattron(windowFromFile, currentScheme);
                            break;
                    case 49: j--;
                            prevScheme = currentScheme;
                            wattroff(windowFromFile, currentScheme);
                            currentScheme = COLOR_PAIR(1);
                            wattron(windowFromFile, currentScheme);
                            break;
                    case 50: j--;
                            prevScheme = currentScheme;
                            wattroff(windowFromFile, currentScheme);                        
                            currentScheme = COLOR_PAIR(2);
                            wattron(windowFromFile, currentScheme);
                            break;
                    case 51: j--;
                            prevScheme = currentScheme;
                            wattroff(windowFromFile, currentScheme);                        
                            currentScheme = COLOR_PAIR(3);
                            wattron(windowFromFile, currentScheme);
                            break;
                    case 52:j--;
                            prevScheme = currentScheme;
                            wattroff(windowFromFile, currentScheme);                        
                            currentScheme = COLOR_PAIR(4);
                            wattron(windowFromFile, currentScheme);
                            break;
                    case 53:j--;
                            prevScheme = currentScheme;
                            wattroff(windowFromFile, currentScheme);                        
                            currentScheme = COLOR_PAIR(5);
                            wattron(windowFromFile, currentScheme);
                            break;
                    case 54: j--;
                            prevScheme = currentScheme;
                            wattroff(windowFromFile, currentScheme);                        
                            currentScheme = COLOR_PAIR(6);
                            wattron(windowFromFile, currentScheme);
                            break;
                    case 55: j--;
                            prevScheme = currentScheme;
                            wattroff(windowFromFile, currentScheme);                        
                            currentScheme = COLOR_PAIR(7);
                            wattron(windowFromFile, currentScheme);
                            break;
                    case 56: j--;
                            prevScheme = currentScheme;
                            wattroff(windowFromFile, currentScheme);                        
                            currentScheme = COLOR_PAIR(8);
                            wattron(windowFromFile, currentScheme);
                            break;
                    default:
                        mvwaddch(windowFromFile, rows, j, line[i]);
                }
            }
            else
                mvwaddch(windowFromFile, rows, j, line[i]);
        }
        rows++;
    }
    wattroff(windowFromFile, currentScheme);
    return windowFromFile;
}

void Window::rotate()
{
    // If the index points out of the vector, set to 0, otherwise, increment
    windowIndex = ( (unsigned int)this->windowIndex + 1 >= this->win.size() ? 0 : this->windowIndex + 1);
    this->top = win[ this->windowIndex ];                       // The current top window is now the next window in the array
}

void Window::setPanelIndex(int PanelNum)
{
    this->pIdx = PanelNum;
}

int Window::getPanelIndex()
{
    return this->pIdx;
}

bool Window::move(int speed)
{
  bool topWinChanged = false;
  if( this->win.size() > 1 )
  {
      rotate();
      topWinChanged = true;
  }
  x -= 2*speed;
  return topWinChanged;
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