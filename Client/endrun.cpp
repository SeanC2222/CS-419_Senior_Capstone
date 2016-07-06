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
    std::string message = "{\"message\": \"Hello World\"}";
    const char* cstr = "This is a test c-string!";
    
    //If cliSock is open (i.e. connected)
    if(cliSock.isOpen()){
        //Write messages to socket
        cliSock.writeToSock(message, message.size());
        cliSock.writeToSock(cstr, 512);
        return 0;
    } else {
        return -1;
    }
}