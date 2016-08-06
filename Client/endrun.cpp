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
            sigaction(SIGPIPE, &nah, NULL);
            break;
        default:
            break;
    }
    
    return;
}

std::vector< std::vector<std::string> > getEnemyFiles(){
    std::vector<std::string> wolf {"wolfy.txt", "wolfy2.txt"};
    std::vector<std::string> tiger {"tiger.txt", "tiger2.txt"};
    std::vector<std::string> snake {"snake1.txt", "snake2.txt"};
    std::vector<std::string> croc {"croc1.txt", "croc2.txt"};
    std::vector<std::string> baddie {"baddie.txt", "baddie2.txt"};
    std::vector< std::vector<std::string> > enemies;
    enemies.push_back(wolf);
    enemies.push_back(tiger);
    enemies.push_back(snake);
    enemies.push_back(croc);
    enemies.push_back(baddie);
    return enemies;
    
}

std::vector< std::vector<std::string> > getPitFiles(){
    std::vector<std::string> pit1 {"pit1.txt"};
    std::vector<std::string> pit2 {"pit2.txt"};
    std::vector<std::string> pit3 {"pit3.txt"};
    std::vector<std::string> pit4 {"pit4.txt"};
    std::vector<std::string> pit5 {"pit5.txt"};
    std::vector< std::vector<std::string> > pits;
    pits.push_back(pit1);
    pits.push_back(pit2);
    pits.push_back(pit3);
    pits.push_back(pit4);
    pits.push_back(pit5);
    return pits;
}

std::vector< std::vector<std::string> > getPoolFiles(){
    std::vector<std::string> pool1 {"pool1-1.txt", "pool1-2.txt"};
    std::vector<std::string> pool2 {"pool2-1.txt", "pool2-2.txt"};
    std::vector< std::vector<std::string> > pools;
    pools.push_back(pool1);
    pools.push_back(pool2);
    return  pools;
    
}

//Main Game/Graphics Update Function
void readFunc(void* argsV){

    void** args = (void**)argsV;
    inetSock cliSock(*(inetSock*)args[0]);
    bool playerOne = *(bool*)args[1];
    Screen* main = (Screen*)args[2];

    std::vector< std::vector<std::string> > enemies = getEnemyFiles();
    std::vector< std::vector<std::string> > pools = getPoolFiles();
    std::vector< std::vector<std::string> > pits = getPitFiles();
    
    vector <std::string> heroFiles {"gladiatorFacing.txt", "gladiatorStep.txt", "gladiatorBack.txt", "gladiatorStep.txt"};

    main->setHero(10,10);
    Window* enemy;      // Need a more elegant way to add things.  Maybe call these from a single function in Screen.
    Window* pit;
    
    main->update();

    int rows, cols;
    getmaxyx(stdscr, rows, cols);
    timeout(1000000);

    setSignalActions(0);
    setSignalActions(SIGINT);
    setSignalActions(SIGTERM);
    
    double backgroundTracker = 0.0;
    double backgroundUpdatePoint = 15.0;
    double rate = 1.0;
    
    int frame = 0;
    int j = 0;
    
    int pitNum = 0;
    int locNum = 0;
    std::vector<int> pitLoc = {0,10, 40, 50};
    
    std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();

    std::ofstream ofs ("ofs.txt", std::ofstream::out | std::ofstream::app);
    while(cliSock.isOpen() && !threadEnd){
        std::string msg = cliSock.readFromSock(512);
        if(msg.size() > 0){
            ofs << msg << std::endl;
            for(int i = 0; i < msg.size(); i++){
                char ch = msg[i];
                if(ch == 'H'){
                    ch = msg[++i];
                    switch(ch){
                        
                    case 3: //Up Encoding
                        main->moveHero("up");
                        break;
                    case 2: //Down Encoding
                        main->moveHero("down");
                        break;
                    case 4: //Left Encoding
                        main->moveHero("left");
                        break;
                    case 5: //Right Encoding
                        main->moveHero("right");
                        break;
                    }
                }
                
                int e;
                if (ch == 'E'){
                    ch = msg[++i];
                    switch(ch){
                    
                    case 'S': //Spawn Encoding
                        //e = main->getLevel();
                        //enemy = main->loadImages(enemies[e], WinType::ENEMY, COLOR_PAIR(1 + main->getLevel() % 8));
                        //main->putOnScreen(enemy, 150, 10);
                        break;
                    case '0': //Up Encoding
                        //main->moveWin("up", enemy);
                        break;
                    case '1': //Down Encoding
                        //main->moveWin("down", enemy);
                        break;
                    case '2': //Left Encoding
                        //main->moveWin("left", enemy);
                        break;
                    case '3': //Right Encoding
                        //main->moveWin("right", enemy);
                        break;
                    }
                }
                
                if (ch == 'P'){
                    //pit = main->loadImages(pits[pitNum], WinType::PIT, COLOR_PAIR(1 + main->getLevel() % 8));
                    //main->putOnScreen(pit, 150, pitLoc[locNum++]);
                    //if(locNum >= pitLoc.size()){
                    //    locNum = 0;
                    //}
                    
                }
                
            }
        }

        std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> tDur = std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1);
        
        if(tDur.count() > ((double)1.0/10.0)){
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
            main->update();
        }
    }
    
    endwin();
    return;
}

void writeFunc(void* argsV){
    
    void** args = (void**)argsV;
    inetSock cliSock(*(inetSock*)args[0]);
    bool playerOne = *(bool*)args[1];
    
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
        } else {
            msg = "";
        }
    }
}

//menu page
int menu(inetSock &cliSock, Screen* main) 
{ 
    
    init_pair(7,COLOR_GREEN,COLOR_BLACK);   
    init_pair(8,COLOR_YELLOW, COLOR_BLACK);
    init_pair(9,COLOR_RED, COLOR_BLACK);
    attrset (COLOR_PAIR(7));                 
    int row, col;
    getmaxyx(stdscr, row, col);
    int x=col/2-35;
    int y=row/2-12;
    ifstream inputfile, inputfile1;
    inputfile.open("./Images/endlessrunner.txt");
    inputfile1.open("./Images/gladiatorFacing.txt");
    if(inputfile.is_open()&&inputfile1.is_open()){
        std::string line,line1;
        while(getline(inputfile,line)){
        //mvprintw(y,x,"ingetline");
        //y++;
            mvprintw(y,x,line.c_str());
            y++;
        }
        attrset(COLOR_PAIR(8));	
        x=col/2-4;
        while(getline(inputfile1,line1)){
            mvprintw(y,x,line1.c_str());
            y++;
        }
    }
    else
        mvprintw(y,x,"could not open files");
    y++;
    attrset (COLOR_PAIR(7));
    x=col/2-2;
    mvprintw(y,x,"by:\n");
    y++;
    x=col/2-8;
    mvprintw(y,x,"dylan kosloff\n");
    y++;
    x=col/2-9;
    mvprintw(y, x,"martha gebremariam &\n");
    y++;
    x=col/2-8;
    mvprintw(y,x,"sean mulholland\n");
    y=y+2;
    x=col/2-15;
    mvprintw(y,x,"press space key to start game\n");
    x=col/2-12;
    y++;
    mvprintw(y,x,"press tab key for how-to\n");
    y++;
    x=col/2-8;
    mvprintw(y,x,"press 'q' to quit\n");
    
    std::string hs = cliSock.readFromSock(512);
    int hsY = row/2;
    int hsX = col * 3 / 4;
    std::string hsLabel = "high score: ";
    attrset(COLOR_PAIR(9));
    mvprintw(hsY, hsX, hsLabel.c_str());
    hsX += hsLabel.size();
    mvprintw(hsY, hsX, hs.c_str());
    refresh();
    attrset(COLOR_PAIR(7));
    wchar_t ch;
    while( (ch =getch() ) != 'q' )
    {
    if(ch==9) //to be updated later
        return 0;
    else if(ch==' '){
        std::string start = "start";
        cliSock.writeToSock(start, start.size());
        y++;
        x=col/2-6;
        mvprintw(y,x, "Waiting on other player...");
        refresh();
        start = cliSock.readFromSock(512);
        y++;
        x=col/2-12;
        if (start.size() > 1){
            mvprintw(y,x, start.c_str());
            refresh();
            usleep(1000000);
            return 0;
        }
        std::string playerMsg = "Player " + start;
        attrset(COLOR_PAIR(9));
        mvprintw(y,x, playerMsg.c_str());
        y++;
        x=col/2-12;
        attrset(COLOR_PAIR(7));
        mvprintw(y,x, "Starting in 3...");
        refresh();
        usleep(1000000);
        y++;
        x=col/2;
        mvprintw(y,x, "2...");
        refresh();
        usleep(1000000);
        y++;
        x=col/2;
        mvprintw(y,x, "1...");
        refresh();
        usleep(1000000);
        return atoi(start.c_str());
        }
    }    
    return 0;
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

    Screen main = Screen();
    main.init();
    
    int menuOpt = menu(cliSock, &main);
    
    if(menuOpt == 0){
        cliSock.Close();
        endwin();
        return 0;
    }
    
    int playerNum = atoi(cliSock.readFromSock(2).c_str());
    bool playerOne = (playerNum == 1) ? true : false;
    
    fcntl(cliSock.getFD(), F_SETFL, fcntl(cliSock.getFD(), F_GETFL,0) | O_NONBLOCK);

    void** args = (void**)malloc(2 * sizeof(void*));
    args[0] = (void*)&cliSock;
    args[1] = (void*)&playerOne;
    args[2] = (void*)&main;
    
    std::thread tRead(readFunc, (void*)args);
    std::thread tWrite(writeFunc, (void*)args);
    tRead.join();
    tWrite.join();
    free(args);

    return 0;
}