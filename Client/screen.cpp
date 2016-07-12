#include "screen.hpp"


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
    
    // Should probably put these in a function later: initColors(int level)
    init_pair(1, COLOR_GREEN, COLOR_YELLOW);    
    init_pair(2, COLOR_BLACK, COLOR_YELLOW);
    

    cbreak();                   // Don't wait for user to hit 'enter'
    noecho();                   // Don't display user input on screen
    curs_set(0);                // Make the cursor invisible --Doesn't work :(
    keypad(stdscr, TRUE);       // For getting arrow keys
    this->bgWindow = Window::getBackground(1);   // Get the first level's background and set it to panel level 0
                                                            printf("\nAdding the Background Image:\n");
    addToPanelLevel(this->bgWindow);
    this->level = 1;
    
    // I'm thinking maybe we put createBackgroundElements(int level)
    // and createOpponents(int level) in this module and then call them here...
}

/* This function will be called at the refresh rate
   so it will wait for a character input, if it doesn't
   happen in REFRESH_RATE time, it just refreshes the screen
*/   
void Screen::update()
{
    // Scroll the background here
    // Check for input and timeout and doUpdate() if none entered
        //halfdelay(x/10 seconds --probably needs to be an init function somewhere else)
    update_panels();
    doupdate();
}

void Screen::addToPanelLevel(Window* image)      // There is also a user pointer for each panel I think...used to store anything if we need it
{
                                                        printf("\nAdding Panel Index #%i to the window\n", panelLevel.size() );
    image->setPanelIndex( panelLevel.size() );
    this->panelLevel.push_back( new_panel(image->getTop()) );
    update_panels();
}

/* Load an animation from an array of filenames, in the order that you want the 
 * animation to proceed.  Actually could be used for all image files...
 */
Window* Screen::loadImages(vector<string> filenames, int xPos, int yPos)
{
    Window* baseWindow = new Window(filenames[0], xPos, yPos);
    
                                                        printf("\nAdding %s to panel#%i", filenames[0], panelLevel.size());
    baseWindow->setPanelIndex(panelLevel.size() );
    this->panelLevel.push_back( new_panel(baseWindow->getTop()) );
    
    // For multiple files (only happens for animations)
    for(unsigned int i = 1; i < filenames.size(); i++) 
    {
                                                        printf("\nAdding %s to panel#%i", filenames[i], panelLevel.size());
        WINDOW* nextImage = baseWindow->appendAnimation( filenames[i] );
        this->panelLevel.push_back( new_panel(nextImage) );  // Add all the other screen 
    }
    update_panels();
    this->movingWindows.push_back(baseWindow);
    return baseWindow;
}





void Screen::move(std::string direction, Window* baseWindow)
{
    int levelSpeed = this->level;

    // If it does, make the current image hidden, make the next image visible and change the top Window
    if(baseWindow->isAnimated())
    {
int oldPanelIndex = baseWindow->getPanelIndex();   
mvwprintw(bgWindow->getTop(), 0, 0, "Old Panel Index: %i", oldPanelIndex  );

        
        hide_panel( panelLevel[ oldPanelIndex ] );                // Hide the top window
        baseWindow->rotate();                                                   // Changes the top window

oldPanelIndex = baseWindow->getPanelIndex();   
mvwprintw(bgWindow->getTop(), 3, 3, "New Panel Index: %i", oldPanelIndex  );

        show_panel( panelLevel[ oldPanelIndex ] );                //  Shows the (new) top window
    }

    int xPos = baseWindow->getX();
    int yPos = baseWindow->getY();
    
    if(direction == "left" && xPos > 0 )                   // Only move left if not at the left edge
                baseWindow->setX(xPos - 2*levelSpeed);
    if(direction == "right" && (xPos + baseWindow->getWidth())  < bgWindow->getWidth() )          
                baseWindow->setX(xPos + 2*levelSpeed);
    if(direction == "up" && yPos > 0 )                              
                baseWindow->setY(yPos - levelSpeed);
    if(direction == "down" &&(yPos + baseWindow->getHeight()) < bgWindow->getHeight() )          
                baseWindow->setY(yPos + levelSpeed );

            move_panel( this->panelLevel[ baseWindow->getPanelIndex() ] , yPos, xPos );
}

void Screen::scrollBg(Window* bgWindow)             // I'm thinking later change params to an array of Window pointers for all objects in the background
{
    // There's a command moveVLine or something to move a vertical line...
    // just move from col 0 to col max-1 and move the rest over?
}


void Screen::cleanup()
{
    delete bgWindow;
    for( auto *window : movingWindows)
        delete window;
}
