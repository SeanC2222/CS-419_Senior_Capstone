//C/Unix Libraries
#include <unistd.h>     //Read/Write, Child Processes
#include <ncurses.h>    //Graphics Handling
#include <signal.h>     //Handle Signals 
#include <stdlib.h>     //Open/Close/etc.
#include <sys/time.h>   //System Time
#include <sys/types.h>  //System Types
#include <fcntl.h>  //Non-Blocking Socket
//C++ Libraries
#include <iostream> //IO Handling
#include <vector>   //Data Container
#include <string>   //Input Container
#include <fstream>  //Debugging with nCurses
#include <chrono>   //Timer for setting Frame Rate
#include <thread>   //Threads for read/write functions

//Custom Libraries
#include "../inetLib/inetLib.hpp"
#include "screen.hpp"
#include "window.hpp"


//Global Flag to tell threads to shutdown
sig_atomic_t threadEnd = 0;

//Signal handler for signals to shutdown
void intAction(int sigNum){
   threadEnd = 1;
}

//Sets Signal Handlers for passed signal
void setSignalActions(int setSig){
    switch(setSig){
    
        case SIGINT:
            struct sigaction act;
            act.sa_handler = intAction;
            act.sa_flags = 0;
            sigaction(SIGINT, &act, NULL);
            break;
        
        case SIGTERM:
            struct sigaction term;
            term.sa_handler = intAction;
            term.sa_flags = 0;
            sigaction(SIGTERM, &term, NULL);
            break;
        
        case 0:
            struct sigaction nah;
            nah.sa_handler = SIG_IGN;
            nah.sa_flags = 0;
            sigaction(SIGINT, &nah, NULL);
            sigaction(SIGTERM, &nah, NULL);
            sigaction(SIGSTOP, &nah, NULL);
            break;
        default:
            break;
    }
    
    return;
}

//Main Game/Graphics Update Function
void readFunc(void* argsV){

    Screen main = Screen();
    main.init();

    vector <string> enemyFiles {"wolfy.txt", "wolfy2.txt" };
    vector <string> heroFiles {"gladiatorFacing.txt", "gladiatorStep.txt", "gladiatorBack.txt", "gladiatorStep.txt"};

    Window* hero = main.loadHero(heroFiles, 10,10);
    Window* enemy = main.loadImages(enemyFiles, WinType::ENEMY);      // Need a more elegant way to add things.  Maybe call these from a single function in Screen.
    
    main.putOnScreen(hero, 10, 10);
    main.putOnScreen(enemy, 150, 10);

    main.update(0);

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    timeout(1000000);

    setSignalActions(SIGINT);
    setSignalActions(SIGTERM);
    
    void** args = (void**)argsV;
    inetSock cliSock(*(inetSock*)args[0]);
    bool playerOne = *(bool*)args[1];
    
    int j=0;
    
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
    
    int num = rand() % 20;
    
    //std::ofstream ofs ("player" + std::to_string(num) + ".txt", std::ofstream::out);
    
    double backgroundTracker = 0.0;
    double backgroundUpdatePoint = 15.0;
    double rate = 1.0;
    
    int frame = 1;
    
    while(cliSock.isOpen() && !threadEnd){
        std::string msg = cliSock.readFromSock(512);
        if(msg.size() > 0){
            //ofs << msg << std::endl;
            for(int i = 0; i < msg.size(); i++){
                
                char ch = msg[i];
                if(ch == 'H'){
                    ch = msg[++i];
                    switch(ch){
                        
                    case 3: //Up Encoding
                        main.move("up", hero);
                        //ofs << "UP" << std::endl;
                        break;
                    case 2: //Down Encoding
                        main.move("down", hero);
                        //ofs << "down" << std::endl;
                        break;
                    case 4: //Left Encoding
                        main.move("left", hero);
                        //ofs << "left" << std::endl;
                        break;
                    case 5: //Right Encoding
                        main.move("right", hero);
                        //ofs << "right" << std::endl;
                        break;
                    }
                }
                
                if (ch == 'E'){
                    ch = msg[++i];
                    switch(ch){
                        
                    case '0': //Up Encoding
                        main.move("up", enemy);
                        //ofs << "UP" << std::endl;
                        break;
                    case '1': //Down Encoding
                        main.move("down", enemy);
                        //ofs << "down" << std::endl;
                        break;
                    case '2': //Left Encoding
                        main.move("left", enemy);
                        //ofs << "left" << std::endl;
                        break;
                    case '3': //Right Encoding
                        main.move("right", enemy);
                        //ofs << "right" << std::endl;
                        break;
                    }
                }
            }
        }

        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> tDur = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        
        if(tDur.count() > ((double)1.0/60.0)){
            t1 = t2;
            backgroundTracker += rate;
            if(backgroundTracker >= backgroundUpdatePoint){
                j++;
                backgroundTracker = 0;
                if (j==175){  //max column size of bg file and should be updated accordingly
                    j=0;
                    backgroundUpdatePoint -= backgroundUpdatePoint*rate;
                    rate *= .9;
                }
            }
            main.scrollBg(j);
            main.update(frame);
            frame++;
        }
    }
    
    main.cleanup();
    endwin();
    return;
}

void writeFunc(void* argsV){
    
    void** args = (void**)argsV;
    inetSock cliSock(*(inetSock*)args[0]);
    bool playerOne = *(bool*)args[1];
    
    int num = rand() % 20;
    //std::ofstream ofs (std::to_string(num) + "player.txt", std::ofstream::out);
    
    std::string msg;
    
    char ch;
    while(cliSock.isOpen() && !threadEnd){
        msg = "";
        if( (ch = getch()) != ERR){
            if(playerOne && (ch == 2 || ch == 3)){
                msg += ch;
                cliSock.writeToSock(msg, msg.size());
            } else if (!playerOne && (ch == 4 || ch == 5)){
                msg += ch;
                cliSock.writeToSock(msg, msg.size());
            }
            //ofs << msg << " " << (int)msg[0] << std::endl;
        } else {
            msg = "";
        }
    }
}

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

    std::cout << "Waiting on other player to connect..." << std::endl;
    
    int playerNum = atoi(cliSock.readFromSock(2).c_str());
    bool playerOne = (playerNum == 1) ? true : false;
    
    std::cout << "Connected as player " << playerNum << std::endl;
    std::cout << "Starting in 3..." << std::endl;
    usleep(1000000);
    std::cout << "2..." << std::endl;
    usleep(1000000);
    std::cout << "1..." << std::endl;
    usleep(1000000);
    

    fcntl(cliSock.getFD(), F_SETFL, fcntl(cliSock.getFD(), F_GETFL,0) | O_NONBLOCK);

    void** args = (void**)malloc(2 * sizeof(void*));
    args[0] = (void*)&cliSock;
    args[1] = (void*)&playerOne;

    std::thread tRead(readFunc, (void*)args);
    std::thread tWrite(writeFunc, (void*)args);
    tRead.join();
    tWrite.join();
    free(args);

    return 0;
}