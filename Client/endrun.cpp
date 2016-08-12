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
sig_atomic_t currentHighscore = 0;
sig_atomic_t currentScore = 0;
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

    int rows, cols;
    getmaxyx(stdscr, rows, cols);

    std::vector< std::vector<std::string> > enemies = getEnemyFiles();
    std::vector< std::vector<std::string> > pools = getPoolFiles();
    std::vector< std::vector<std::string> > pits = getPitFiles();
    int bigPit = 4;
    vector<string>javelinFile{"javelin.txt"};
    Window* javelin;

    Hero* hero = NULL;
    vector<Enemy*> activeEnemies;

    setSignalActions(SIGTERM);

    double rate = 1.0;

    int j = 0;

    int pitNum = 0;
    int locNum = 0;
    std::vector<int> pitLoc = {0, 34-(34*2/3), 34-(34*3/4), 34-11, 34-(34*1/3), 34-(34*4/5)};
    int bigPitTop = pitLoc[0];      //Ensure hero can dodge big pit
    int bigPitBottom = pitLoc[3];   //Ensure hero can dodge big pit


    fd_set sock;
    FD_ZERO(&sock);
    struct timeval timeout;
    std::string msg;

    std::chrono::high_resolution_clock::time_point refresh_s, refresh_f, enemy_s, enemy_f;
    refresh_s = std::chrono::high_resolution_clock::now();

    main->update();
    hero = main->getHero();
    bool jav = false;
    int javelinCount = 0;

    while(cliSock.isOpen() && !threadEnd){
        msg = "";
        timeout.tv_sec = 0;
        timeout.tv_usec = (10000);
        FD_SET(cliSock.getFD(), &sock);

        select(cliSock.getFD()+1, &sock, NULL, NULL, &timeout);

        if(FD_ISSET(cliSock.getFD(), &sock)){

            msg = cliSock.readFromSock(512);
            if(msg.size() == 0 || threadEnd){
                cliSock.Close();
                msg = "Server disconnected...";
                attrset(COLOR_PAIR(color::SCORE));
                mvprintw(rows/2, cols/2-10, msg.c_str());
                refresh();
                threadEnd = 1;
                sleep(2);
                break;
            } else if(msg.size() > 0){
	        int area = main->getArea();
                for(int i = 0; i < msg.size(); i++){
                    char ch = msg[i];
                    int e;

                    if(ch == 'H'){
                        ch = msg[++i];
			hero = main->getHero();
                        switch(ch){

                        case 3: //Up Encoding
                            hero->move("up", 1);
                            continue;
                        case 2: //Down Encoding
                            hero->move("down", 1);
                            continue;
                        case 4: //Left Encoding
                            hero->move("left", main->getLevel());
                            continue;
                        case 5: //Right Encoding
                            hero->move("right", main->getLevel());
                            continue;
                        case 'A':
                           //Throw javelin
                            if(!jav){
			       if(area == ARENA){
				 javelin = main->loadImages(javelinFile, WinType::JAVELIN, COLOR_PAIR(color::JAVELIN_A));
			       } else if (area == FOREST){
				 javelin = main->loadImages(javelinFile, WinType::JAVELIN, COLOR_PAIR(color::JAVELIN_F));
			       } else if (area == BEACH){
				 javelin = main->loadImages(javelinFile, WinType::JAVELIN, COLOR_PAIR(color::JAVELIN_B));
			       } else if (area == SHALLOW_WATER || area == DEEP_WATER){
				 javelin = main->loadImages(javelinFile, WinType::JAVELIN, COLOR_PAIR(color::JAVELIN_W));
			       } else {
				 javelin = main->loadImages(javelinFile, WinType::JAVELIN, COLOR_PAIR(color::JAVELIN_A));
			       }
			       main->putOnScreen(javelin, hero->getX(), hero->getY());
			       jav = true;
			    } 
                            continue;
                        case 'J':
                            hero->move("jump", main->getLevel());
                            continue;
                        }
                   } else if (ch == 'E'){
                        ch = msg[++i];
                        switch(ch){

                        case 'S': //Spawn Encoding
                            ch = msg[++i] - 48;
                            if(area == ARENA){
                                if(ch){
                                    e = 0; //Wolf
                                } else {
                                    e = 4; //Baddie
                                }
                            } else if (area == FOREST){
                                if(ch){
                                    e = 1;  //Tiger
                                } else {
                                    e = 2; //Snake
                                }
                            } else if (area == BEACH){
                                if(ch){
                                    e = 1; //Tiger
                                } else {
                                    e = 3; //Croc
                                }
                            } else {
                                if(ch){
                                    e = 2; //Snake
                                } else {
                                    e = 3; //Croc
                                }
                            }
			    Enemy* enemy;
                            if(area == ARENA){
                                if(rand() % 2){
                                    enemy = main->loadEnemy(enemies[e], COLOR_PAIR(color::ARENA_ENEMY_ONE));
                                } else {
                                    enemy = main->loadEnemy(enemies[e], COLOR_PAIR(color::ARENA_ENEMY_TWO));
                                }
                            } else if (area == FOREST){
                                if(rand() % 2){
                                    enemy = main->loadEnemy(enemies[e], COLOR_PAIR(color::FOREST_ENEMY_ONE));
                                } else {
                                    enemy = main->loadEnemy(enemies[e], COLOR_PAIR(color::FOREST_ENEMY_TWO));
                                }
                            } else if (area == BEACH){
                                    enemy = main->loadEnemy(enemies[e], COLOR_PAIR(color::BEACH_ENEMY));
                                
                            } else {
                                if(rand() % 2){
                                    enemy = main->loadEnemy(enemies[e], COLOR_PAIR(color::WATER_ENEMY_ONE));
                                } else {
                                    enemy = main->loadEnemy(enemies[e], COLOR_PAIR(color::WATER_ENEMY_TWO));
                                }
                            }
                            main->putOnScreen(enemy, 200, 10);
			    activeEnemies.push_back(enemy);
                            continue;
                        case '0': //Up Encoding
			    for(int i = 0; i < activeEnemies.size(); i++){
				 if(!main->moveEnemy(activeEnemies[i], "up", 1)){
				      activeEnemies.erase(activeEnemies.begin() + i);
				 }
			    }
                            continue;
                        case '1': //Down Encoding
			    for(int i = 0; i < activeEnemies.size(); i++){
				 if(!main->moveEnemy(activeEnemies[i], "down", 1)){
				      activeEnemies.erase(activeEnemies.begin() + i);
				 }
                            }
                            continue;
                        }
                    } else if (ch == 'P'){
                        ch = msg[++i] - 48; // '0' character is value 48; 48 corrects to 0
			Window* pit = NULL;
                        if(area == SHALLOW_WATER || area == DEEP_WATER){
                           pit = main->loadImages(pools[ch % pools.size()], WinType::PIT, COLOR_PAIR(color::POOL));
                        } else if (area == FOREST){
                           pit = main->loadImages(pits[ch % pits.size()], WinType::PIT, COLOR_PAIR(color::PIT_F));
                        } else if (area == BEACH){
                           pit = main->loadImages(pits[ch % pits.size()], WinType::PIT, COLOR_PAIR(color::PIT_B));
                        } else {
                           pit = main->loadImages(pits[ch % pits.size()], WinType::PIT, COLOR_PAIR(color::PIT_A));
                        }
			if(pit != NULL){
			   if(ch % pits.size() == bigPit){
			       if(rand() % 2){
				   main->putOnScreen(pit, 200, bigPitTop);
			       } else {
				   main->putOnScreen(pit, 200, bigPitBottom);
			       }
			   } else {
			       main->putOnScreen(pit, 200, pitLoc[locNum++]);
			   }
			}
                        if(locNum >= pitLoc.size()){
                            locNum = 0;
                        }

                    } else if (ch == 'S'){
			std::string score;
			for(int j = i+1; j < msg.size(); j++){
			   if(isdigit(msg[j])){
			      score += msg[j];
			   } else {
			      i = j-1;
			      break;
			   }
			}
			currentScore = atoi(score.c_str());
                        continue;

                   } else if (ch == 'K'){
			msg = cliSock.readFromSock(512);
		        std::string finalScore;
			for(int j = 0; j < msg.size(); j++){
			   if(msg[j] == 'S'){
			      for(int k = j+1; k < msg.size(); k++){
				 if(isdigit(msg[k])){
				    finalScore += msg[k];
				 } else {
				    break;
				 }
			      }
			   }
			}
				 
                        cliSock.Close();
                        threadEnd = 1;
                        break;
                    }

                    
                }
            } else {
                endwin();
                std::cout << "oops..." << std::endl;
                exit(-1);
            }
        }

        refresh_f = std::chrono::high_resolution_clock::now();
        
        std::chrono::duration<double, std::milli> tDur = (refresh_f - refresh_s);
        
        if(tDur.count() > (double)(1000.0/30.0) ){
	    if(++javelinCount > 40){
	       jav = false;
	       javelinCount = 0;
	    }
	    
            refresh_s = refresh_f;
            if(main->update()){
                threadEnd = 1;
                break;
            }
        }

    }
    
    if(cliSock.isOpen()){
        cliSock.Close();
    }

    endwin();
    return;
}

void writeFunc(void* argsV){
    
    void** args = (void**)argsV;
    inetSock cliSock(*(inetSock*)args[0]);
    bool playerOne = *(bool*)args[1];
    
    timeout(1000);

    std::string msg;    
    char ch;
    while(cliSock.isOpen() && !threadEnd){
        msg = "";
        if (ch == 'q'){
	    threadEnd = 1;
	    cliSock.Close();
        }
        if( (ch = getch()) != ERR){
            if(playerOne && (ch == 2 || ch == 3 || ch == ' ')){
                msg += ch;
                cliSock.writeToSock(msg);
            } else if (!playerOne && (ch == 4 || ch == 5 || ch == ' ')){
                msg += ch;
                cliSock.writeToSock(msg);
            }
        } else {
            msg = "";
        }
    }
}

//menu page
int menu(inetSock &cliSock, Screen* main, int checkedHowTo) 
{ 
    attrset (COLOR_PAIR(color::MENU_ONE));
    int row, col;
    getmaxyx(stdscr, row, col);
    int x=col/2-35;
    int y=row/2-12;
    ifstream inputfile, inputfile1, inputfile2;
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
        attrset(COLOR_PAIR(color::MENU_TWO));
        x=col/2-4;
        while(getline(inputfile1,line1)){
            mvprintw(y,x,line1.c_str());
            y++;
        }
    }
    else
        mvprintw(y,x,"could not open files");
    y++;
    attrset (COLOR_PAIR(color::MENU_ONE));
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
    refresh();
    std::string hsLabel;
    
     if(checkedHowTo == -1){
        hsLabel = cliSock.readFromSock(512);
        if(hsLabel.size() == 0){
	    attrset(COLOR_PAIR(color::MENU_ONE));
	    mvprintw(row/2,col/2 -12,"No connection to server.\n");
	    refresh();
	    return 0;
        }
        std::string temp = "";
        for(int i = 0; i < hsLabel.size(); i++){
	    if(hsLabel[i] == 'S'){
	       temp = "";
	       for(int j = i+1; j < hsLabel.size(); j++){
		  if(isdigit(hsLabel[j])){
		     temp += hsLabel[j];
		  } else {
		     i = j-1;
		     break;
		  }
	       }
	    }
        }
        hsLabel = temp;
        currentHighscore = atoi(hsLabel.c_str());
        checkedHowTo = currentHighscore;
    } else {
        hsLabel = std::to_string(checkedHowTo);
    }

    hsLabel = "high score: " + std::to_string(currentHighscore);

    int hsY = row/2;
    int hsX = col * 3 / 4;
    attrset(COLOR_PAIR(color::MENU_THREE));
    mvprintw(hsY, hsX, hsLabel.c_str());
    refresh();

    attrset(COLOR_PAIR(color::MENU_ONE));
    wchar_t ch;
    while( (ch =getch() ) != 'q' && !threadEnd)
    {
    if(ch==9){ //if tab is pressed
	    erase(); //clean up window before displaying how to page
        x=0;
        y=0;		
        inputfile2.open("./Images/howto.txt");
        if(inputfile2.is_open()){
            string line2,line3;
            attrset(COLOR_PAIR(7));
            inputfile.close();
            inputfile.open("./Images/endlessrunner.txt");	
            while(getline(inputfile,line2)){
     	        mvprintw(y,x,line2.c_str());
    	        y++;
            }
            attrset(COLOR_PAIR(8));	
            while(getline(inputfile2,line3)){
    	        mvprintw(y,x,line3.c_str());
    	        y++;
            }
        }
        else
	        mvprintw(y,x,"could not open files");

       wchar_t ch;
       while(ch =getch()) 
       {
          if(ch==' '){
    	    erase();
    	    return menu(cliSock, main, checkedHowTo);  //menu returns 1 for 'q' so if menu returned 1 so should this line
          }
       }
     
    } else if(ch==' '){
        std::string msg = "start";
        cliSock.writeToSock(msg);
        y++;
        x=col/2-12;
        mvprintw(y,x, "Waiting on other player...");
        refresh();

        fd_set sock;
        FD_ZERO(&sock);
        struct timeval timeout;

        while(msg != "1" && msg != "2"){

             msg = "";
            timeout.tv_sec = 0;
            timeout.tv_usec = (100000.0);
            FD_SET(cliSock.getFD(), &sock);

            if(threadEnd){
                endwin();
                exit(0);
            }
            select(cliSock.getFD()+1, &sock, NULL, NULL, &timeout);

            if(FD_ISSET(cliSock.getFD(), &sock)){
                msg = cliSock.readFromSock(512);
                if(msg.size() == 0){
                    y=row/2;
                    x=col/2 - 15;
                    mvprintw(y,x, "Lost Server Connection");
                    refresh();
                    sleep(2);
                    endwin();
                    exit(-1);
                } else {
                    for(int i = 0; i < msg.size(); i++){
                        if(msg[i] == 'S'){
			    std::string curScore = "";
			    while(isdigit(msg[++i])){
			       curScore += msg[i];
			    }
                            hsLabel = "high score: " + curScore;
                            attrset(COLOR_PAIR(color::MENU_THREE));
                            mvprintw(hsY, hsX, hsLabel.c_str());
                            refresh();
                        }
                        if(msg[i] == 'P'){
                            i++;
                            msg = msg.substr(i, msg.size()-i);
                            i = 0;
                            continue;
                        }
                    }

                }
            }
        }
        y++;
        x=col/2-12;
        std::string playerMsg = "Player " + msg;
        attrset(COLOR_PAIR(color::MENU_THREE));
        mvprintw(y,x, playerMsg.c_str());
        y++;
        x=col/2-12;
        attrset(COLOR_PAIR(color::MENU_ONE));
        mvprintw(y,x, "Starting in 3...");
        refresh();
        usleep(1000000);
        x=col/2;
        mvprintw(y,x, "2...");
        refresh();
        usleep(1000000);
        x=col/2;
        mvprintw(y,x, "1...");
        refresh();
        usleep(1000000);
        //Clear background -- Makes panels look better
        for(int i = 0; i < col; i++){
            for(int j = 0; j < row; j++){
                mvprintw(j,i, " ");
            }
        }
        refresh();
        return atoi(msg.c_str());
        } else if (threadEnd){
            return 0;
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
    timeout(100000);
    
    setSignalActions(0);
    setSignalActions(SIGINT);
    
    int playerNum = menu(cliSock, &main, -1);
    
    if(playerNum == 0){
        cliSock.Close();
        endwin();
        return 0;
    }
    
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

    if(currentScore < 8500){
       std::cout << "Try again! You can do better! Score: " << currentScore << std::endl;
    } else if (currentScore >=8500 && currentScore < 14500){
       std::cout << "You're getting there! Almost to the Beach! Score: " << currentScore << std::endl;
    } else if (currentScore >= 14500 && currentScore < 16600){
       std::cout << "Keep going! The water isn't too far off! Score: " << currentScore << std::endl;
    } else if (currentScore >= 16600 && currentScore < 24500){
       std::cout << "You're almost out of the water! Score: " << currentScore << std::endl;
    } else if (currentScore >= 24500 && currentScore < 25000){
       std::cout << "ALMOST TO LEVEL 2! Good job! Score: " << currentScore << std::endl;
    } else if (currentScore >=25000 && currentScore < 37500){
       std::cout << "You made it! Level 2! Score: " << currentScore << std::endl;
    } else {
       std::cout << "Holy cow! You made it to level " << main.getLevel() << "! Great job! Score: " << currentScore << std::endl;
    }
    return 0;
}
