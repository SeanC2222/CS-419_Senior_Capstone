//C/Unix Libraries
#include "unistd.h"     //Read/Write, Child Processes
#include "signal.h"     //Handle Signals 
#include "fcntl.h"      //File control
#include "sys/wait.h"   //Wait for processes
//C++ Libraries
#include <iostream>     //IO Handling
#include <vector>       //Data Container
#include <string>       //Input Container

//Custom Libraries
#include "../inetLib/inetLib.hpp"

//Main Server Game Loop
void playGame(std::pair<int,int>);

int main(){
    //Create socket to listen on
    inetSock servSock("52010");
    
    //Listen on open file descriptor, accept up to 10 connections
    listen(servSock.getFileDescriptor(), 10);
    //Set FD to non-blocking mode
    fcntl(servSock.getFileDescriptor(), F_SETFL, fcntl(servSock.getFileDescriptor(), F_GETFL) | O_NONBLOCK);

    //Game, and gamePID tracking
    std::vector< std::pair<int,int> > games;
    std::vector<int> gamePID;
    
    //Temporary players pair
    std::pair<int,int> players;
    
    //Keeps count of current total players
    int playerCount = 0;
    
    while(1){
        //Accept a connection if available
        int fd = accept(servSock.getFileDescriptor(), NULL, NULL);
        //Allows binding of socket unless already being listened on
        //Not necessary, but useful for program crashes/reboots on the same port
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, NULL, 0);
        //If a connection was accepted...
        if(fd != -1){
            //If # of players is odd...
            if(playerCount % 2 == 0){
                //Store player in first players slot
                players.first = fd;
                playerCount++;
            } else {
                //Store player in second players slot
                players.second = fd;
                playerCount++;
                //Store pair of players
                games.push_back(players);
                //Fork a game for players
                pid_t pid = fork();
                //If pid == 0, child = true, else child = false
                bool child = (pid == 0) ? true : false;
                //If process is child....
                if(child){
                    //Child Process
                    playGame(games[playerCount/2 -1]);
                    exit(0);
                //Else process is parent...
                } else {
                    //Parent Process
                    gamePID.push_back(pid);
                }
            }
            
        }
        
        //Check gamePID for shutdown processes
        for(unsigned int i = 0 ; i < gamePID.size(); i++){
            //Check if a game has exited, don't wait if it hasn't
            if(waitpid(gamePID[i], NULL, WNOHANG)){
                //Close file descriptors
                close(games[i].first);
                close(games[i].second);
                //Erase game/gamePID from records
                games.erase(games.begin()+i);
                gamePID.erase(gamePID.begin()+i);
                //Decrement by 2 players
                playerCount -= 2;
            }
        }
    }
}

//Child process... I.e. Game loop goes here
void playGame(std::pair<int,int> player) {
    //Creates dummy inetSockets to allow reading from file descriptors
    inetSock player1(player.first);
    inetSock player2(player.second);
    std::cout << "Server: Players " << player.first << ", " << player.second << std::endl;

    //Simple test once two players are connected to see if server can read
    std::cout << "Server: Testing " << player.first << " read..." << std::endl;
    std::cout << "Player 1: " << player1.readFromSock(512) << std::endl;
    
    std::cout << "Server: Testing " << player.second << " read..." << std::endl;
    std::cout << "Player 2: " << player2.readFromSock(512) << std::endl;
    
    
}