//C/Unix Libraries
#include <unistd.h> //Read/Write, Child Processes
#include <ncurses.h>
#include <signal.h> //Handle Signals 
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
//C++ Libraries
#include <iostream> //IO Handling
#include <vector>   //Data Container
#include <string>   //Input Container

//Custom Libraries
#include "../inetLib/inetLib.hpp"
#include "screen.hpp"

int main(int argc, char* argv[]){
    
    std::string host, port, connPort;
    
    if(argc > 3){
        host = std::string(argv[1]);
        port = std::string(argv[2]);
        connPort = std::string(argv[3]);
    } else {
        std::cout << "Current usage: " << std::endl
                  << "endrun <dest_host> <dest_port> <conn_port>" << std::endl;
        exit(0);
    }

    //Create client socket and connect to server
    inetSock cliSock(host, port, connPort);
    
    int playerNum = atoi(cliSock.readFromSock(2).c_str());
    bool playerOne = (playerNum == 1) ? true : false;
    
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    fd_set readfds, writefds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET(0, &readfds);    //Adds STDIN to readfds
    FD_SET(cliSock.getFD(), &writefds);
    
    std::string msg;
    Screen main = Screen();
    main.init();

    vector <string> wolfFiles {"wolfy.txt", "wolfy2.txt" };
    vector <string> heroFiles {"gladiatorFacing.txt", "gladiatorStep.txt", "gladiatorBack.txt", "gladiatorStep.txt"};
    Window* hero = main.loadImages(heroFiles, 10,10);

    Window* wolf = main.loadImages(wolfFiles, 150, 10);      // Need a more elegant way to add things.  Maybe call these from a single function in Screen.

    main.update();
    wchar_t ch;
    msg = " ";

    int j=0;
    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    timeout(100);

    while(cliSock.isOpen()){
        
        if( (ch = getch()) != ERR){
            msg[0] = ch;
        } else {
            msg[0] = ' ';
        }
        int n = cliSock.writeToSock(msg, 2);

        /*added by martha*/
        main.scrollBg(j);
        j++;
        if (j==175)  //max column size of bg file and should be updated accordingly
	        j=0;
        /*added by martha*/
        int wolfDir = atoi(cliSock.readFromSock(2).c_str());
        
        switch(wolfDir){
            case 0: 
                main.move("left", wolf, true);
                break;
            case 1:
                main.move("right", wolf, true);
                break;
            case 2:
                main.move("down", wolf, true);
                break;
            case 3:
                main.move("up", wolf, true);
                break;
        }

        if(playerOne){
            switch(ch)
            {
                case KEY_UP:
                    main.move("up", hero, false);
                    break;
                case KEY_DOWN:
                    main.move("down", hero, false);
                    break;
            }
            ch = cliSock.readFromSock(2)[0];
        } else {
            switch(ch)
            {
            case KEY_LEFT:
                main.move("left", hero, false);
                break;
            case KEY_RIGHT:
                main.move("right", hero, false);
                break;
            }
            ch = cliSock.readFromSock(2)[0];
        }
        
        switch(ch)
        {
            case KEY_LEFT:
                main.move("left", hero, false);
                break;
            case KEY_RIGHT:
                main.move("right", hero, false);
                break;
            case KEY_UP:
                main.move("up", hero, false);
                break;
            case KEY_DOWN:
                main.move("down", hero, false);
                break;
        }
        
        main.update();
    }
    main.cleanup();
    endwin();
    return 0;
/*        FD_SET(0, &readfds);    //Adds STDIN to readfds
        FD_SET(cliSock.getFD(), &writefds);
        select(5, &readfds, NULL, NULL, NULL);

        msg = "";
        if(FD_ISSET(0, &readfds)){ 
            char c;
            while( (c = getch()) != ERR && msg.size() < 10){
                msg += c;
            }
            int n = cliSock.writeToSock(msg, msg.size()+1);
            std::cout << "n = " << n << ", msg = " << msg << std::endl;
        }
*/
}