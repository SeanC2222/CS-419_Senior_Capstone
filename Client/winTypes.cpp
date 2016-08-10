#include "window.hpp"

class Hero : public Window
{
    private:
        int actionFrame;
        bool swimming, jumping, climbingOut;
        vector<WINDOW*> swimmingWins;
        vector<WINDOW*> jumpingWins;
        vector<WINDOW*> climbingOutWins;
    
    public:
    Hero(string filename, int xStart, int yStart, WinType type, int colorScheme):
        Window(filename, xStart, yStart, type, colorScheme) 
    {
        swimming = false;
        jumping = false;
        climbingOut = false;
    }

    void startSwimming()
    {
        // Play the jump first, set windowIndex = 0, then
        jumping = true;
        this->windowIndex = 0;
        actionFrame = 0;
    }
    
    void stopSwimming()
    {
        swimming = false;
        climbingOut = true;
        actionFrame = 0;
        this->windowIndex = 0;
    }

    
    void rotate()
    {
        if(swimming && !jumping)
        {
            this->top = swimmingWins[windowIndex];
            windowIndex = ( (unsigned int)windowIndex + 1 >= swimmingWins.size() ? 0 : windowIndex + 1 );
        }
        else if(jumping)
        {
            actionFrame++;
            //if(actionFrame <= 2 )   { /* Just move right */ }
            if(actionFrame <= 4 )   
            { 
                if(this->getY() + this->getHeight() + 1 < screenBottomLimit) 
                    this->setY(this->getY()+1); 
            }
            if(actionFrame <= 6 )   
            { 
                if(this->getY() - 1 > 0) 
                    this->setY(this->getY()+1); this->setY(this->getY()-1); 
            }
            
            if(actionFrame == 8)
            {
                swimming = ( swimming ? false : true );
                jumping = false;
            }
            
            this->top = jumpingWins[windowIndex];
            windowIndex = ( (unsigned int)windowIndex + 1 >= jumpingWins.size() ? 0 : windowIndex + 1 );
        }
        else if(climbingOut)
        {
            actionFrame++;
            if(actionFrame == 1)   
            { 
                int newY = ( this->getY() - 4 < 0 ? 0 : this->getY() - 4 );
                this->setY(newY);
            }

            if(actionFrame == 3)
                climbingOut = false;
                        
            this->top = climbingOutWins[windowIndex];
            windowIndex = ( (unsigned int)windowIndex + 1 >= climbingOutWins.size() ? 0 : windowIndex + 1 );
        }
        else
        {
            this->top = win[windowIndex];
            windowIndex = ( (unsigned int)windowIndex + 1 >= win.size() ? 0 : windowIndex + 1 );
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
           // if(xPos + this->getWidth() > screenRightLimit)
              // xPos -= 2*speed;
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
            if( !swimming && (yPos + this->getHeight() > screenBottomLimit) )
               yPos -= speed;
             else if(yPos + this->getHeight() - 6 > screenBottomLimit)
                yPos -= speed;
        }
        if(direction == "jump")
        {
           xPos += 12;
            //if(xPos + this->getWidth() > screenRightLimit)
               //xPos -= 12;
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
        if(type == "climb")
            this->climbingOutWins.push_back(frame);
    }

};

class Enemy : public Window
{
    public:
    Enemy(string filename, int xStart, int yStart, WinType type, int colorScheme):
        Window(filename, xStart, yStart, type, colorScheme) {}
        
    bool move(string direction, int speed)
    {
        int xPos = this->getX();
        int yPos = this->getY();
        
        if(direction == "left") 
           xPos -= 2*speed;

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

};

class Score : public Window
{
    public:
    Score(string filename, int xStart, int yStart, WinType type, int colorScheme):
        Window("", xStart, yStart, type, colorScheme) {}
        
    void update(int score){
        attrset(COLOR_PAIR(10));
       mvwprintw(this->getTop(), 0,0, std::to_string(score).c_str());
    }
    
    bool move(string direction, int speed){
        return true;
    }
        
    
};