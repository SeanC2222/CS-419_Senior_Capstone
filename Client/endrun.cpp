//C/Unix Libraries
#include "unistd.h" //Read/Write, Child Processes
#include "signal.h" //Handle Signals 
//C++ Libraries
#include <iostream> //IO Handling
#include <vector>   //Data Container
#include <string>   //Input Container

//Custom Libraries
#include "../inetLib/inetLib.hpp"

int main(int argc, char* argv[]){
    
    //Create client socket and connect to server
    inetSock cliSock("localhost", "52010", argv[1]);
    
    //Create fake JSON style message, and c-string
    std::string message;
    
    if(argc >= 3){
        message = argv[2];
    } else {
        message = "This is a test message from port ";
        message += argv[1];
    }
    //If cliSock is open (i.e. connected)
    if(cliSock.isOpen()){
        //Write messages to socket
        cliSock.writeToSock(message, message.size());
        return 0;
    } else {
        return -1;
    }
}