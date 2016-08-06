#include "window.hpp"

class Hero : public Window
{
    private:
        int screenRightLimit, screenBottomLimit, jumpingFrame;
        bool swimming, jumping;
        vector<WINDOW*> swimmingWins;
        vector<WINDOW*> jumpingWins;
    
    public:
    Hero(string filename, int xStart, int yStart, WinType type, int colorScheme):
        Window(filename, xStart, yStart, type, colorScheme) 
    {
        swimming = false;
        jumping = false;
    }

    void saveScreenLimits(int width, int height)
    {
        screenBottomLimit = height;
        screenRightLimit = width;
    }
    
    void startSwimming()
    {
        // Play the jump first, set nextWindowIndex = 0, then
        jumping = true;
        this->nextWindowIndex=0;
        jumpingFrame = 0;
    }
    
    void rotate()
    {
        if(swimming && !jumping)
        {
            nextWindowIndex = ( (unsigned int)nextWindowIndex + 1 >= swimmingWins.size() ? 0 : nextWindowIndex + 1 );
            this->top = swimmingWins[nextWindowIndex];
        }
        else if(jumping)
        {
            jumpingFrame++;
            if(jumpingFrame <= 2 )   { /* Just move right */ }
            if(jumpingFrame <= 4 )   { this->setY(this->getY()+1); }
            if(jumpingFrame <= 6 )   { this->setY(this->getY()-1); }
            
            if(jumpingFrame == 7)
            {
                swimming = ( swimming ? false : true );
                jumping = false;
            }
            
            nextWindowIndex = ( (unsigned int)nextWindowIndex + 1 >= jumpingWins.size() ? 0 : nextWindowIndex + 1 );
            this->top = jumpingWins[nextWindowIndex];
        }
        else
        {
            nextWindowIndex = ( (unsigned int)nextWindowIndex + 1 >= win.size() ? 0 : nextWindowIndex + 1 );
            this->top = win[nextWindowIndex];
        }
    }
    
    bool move(string direction, int speed)
    {

        int xPos = this->getX();
        int yPos = this->getY();
        
        if(direction == "left") 
        {
           xPos -= 2*speed;
            if(xPos < 0 )
               xPos += 2*speed;
        }
        if(direction == "right")
        {
           xPos += 2*speed;
            if(xPos + this->getWidth() > screenRightLimit)
               xPos -= 2*speed;
        }
        if(direction == "up")   
        {
           yPos -= speed;
            if(yPos < 0 )
               yPos += speed;
        }
        if(direction == "down") 
        {
           yPos += speed;
            if(yPos + this->getHeight() > screenBottomLimit)
               yPos -= speed;
        }
        this->setX(xPos);
        this->setY(yPos);
        rotate();            // Changes the top window index
        return true;
    }
    
    void appendOtherAnimation(string filename, string type, int colorScheme)
    {
        WINDOW* frame = getWinFromFile(filename, this->getX(), this->getY(), colorScheme);

        if(type == "swim") 
            this->swimmingWins.push_back(frame);
        if(type == "jump")
            this->jumpingWins.push_back(frame);
    }

};