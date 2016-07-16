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
    
    struct timeval timeout;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;
    
    fd_set readfds, writefds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    FD_SET(0, &readfds);    //Adds STDIN to readfds
    FD_SET(cliSock.getFD(), &writefds);
    
    std::string msg;
    WINDOW* myWin;
    myWin = initscr();
    //cbreak();
    wchar_t c;
    msg = " ";
    while(cliSock.isOpen()){
        
        if( (c = getch()) != ERR){
            msg[0] = c;
            int n = cliSock.writeToSock(msg, msg.size()+1);
            std::cout << "n = " << n << ", c = " << c << std::endl;
        }
        usleep(100000);
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
    
}