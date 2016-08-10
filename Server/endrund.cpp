//C/Unix Libraries
#include "unistd.h"     //Read/Write, Child Processes
#include "signal.h"     //Handle Signals 
#include "fcntl.h"      //File control
#include "sys/wait.h"   //Wait for processes
#include "sys/types.h"  //pid_t
#include "sys/stat.h"   
#include "string.h"
//C++ Libraries
#include <iostream>     //IO Handling
#include <fstream>
#include <vector>       //Data Container
#include <string>       //Input Container
#include <chrono>		//Time points
//Custom Libraries
#include "../inetLib/inetLib.hpp"

//Definitions
#define DEFAULT_PORT "52010"
#define POLL_PERIOD_MILLI POLL_PERIOD_MICRO/1000.0 //Must be per Micro
#define POLL_PERIOD_MICRO 500000                   //Needs to be int

//Set up Signal Handlers
void setSignalActions(int);
//Tokenizes Command Line Arguments in vector
std::vector<std::string> getArgs(int, char**);
//Parses Command Line Arguments
std::pair<std::string,bool> parseArgs(std::vector<std::string>);
//Create Logs/endrund Path if not exists
int checkLogsPath();
//Starts Server
int buildServer(std::string);
//Main Server Game Loop
void playGame(std::pair<int,int>);


sig_atomic_t signalFlag = 0; //Global flag in parent to check for "--shutdown" signal
sig_atomic_t currentHighscore = 0;

int main(int argc, char* argv[]){

	std::vector<std::string> args;
	
	std::string portno = DEFAULT_PORT;
	
	if(argc > 1){
		args = getArgs(argc, argv);
		std::pair<std::string, bool> status = parseArgs(args);
		//If there was an error, announce error and exit
		if(!status.second){
			std::cerr << "endrund: Error with argument \"" + status.first + "\"" << std::endl;
			return -1;
		//Else no error and continue
		} else {
			//If command executed successfully was --shutdown, exit
			if(status.first == "--shutdown"){
				return 0;
			//Else set portno to first
			} else {
				portno = status.first;
			}
		}
	}

	int status = checkLogsPath();
	if(status){
		std::cerr << "endrund: Error validating/creating logs path" << std::endl;
		return -1;
	} 
	
	std::cout << "endrund: Starting background process on port " << portno << std::endl;
	pid_t pid = fork();
	bool child = (pid == 0) ? true : false;

	if(child){

		std::ofstream logs("logs/endrund/endrundlog.txt", std::ofstream::out | std::ofstream::app);

		//Daemon Functionality turned off...
		//umask(0);
		//pid_t sid = setsid();
		//if(sid < 0){
		//	logs << "endrund: Unable to create new SID for daemon." << std::endl;
		///	exit(-1);
		//}
		close(STDIN_FILENO);
		
		//std::streambuf *coutbuf = std::cout.rdbuf();
		//std::cout.rdbuf(logs.rdbuf());
		//std::streambuf *cerrbuf = std::cerr.rdbuf();
		//std::cerr.rdbuf(logs.rdbuf());

		buildServer(portno);

//		std::cout.rdbuf(coutbuf);
//		std::cerr.rdbuf(cerrbuf);
		std::cout << "Current HighScore : " << currentHighscore << std::endl;
		return 0;
	} else {
		usleep(20000);
		if(!kill(pid, 0)){
			std::cout << "endrund: Process started as PID: " << pid << std::endl;
			std::ofstream pidLog("logs/endrund/endrund.pid", std::ofstream::out);
			pidLog << pid << std::endl;
			pidLog.close();
		} else {
			std::cerr << "endrund: Process startup unsuccessful" << std::endl;
		}
	}
	return 0;
}

void usr1Action(int signalNumber){
	signalFlag = 1;
	return;
}

void termAction(int signalNumber){
		union sigval data = {currentHighscore};
		sigqueue(getppid(), SIGUSR2, data);
		exit(0);
}

void usr2Action(int signalNumber,  siginfo_t* data, void* other){
	if(data->si_value.sival_int > currentHighscore){
		currentHighscore = data->si_value.sival_int;
		int fd = open("logs/endrund/endrund.hs", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
		if(fd != -1){
			int n = write(fd, &currentHighscore, sizeof(int));
			close(fd);
		}
	}
	return;
}

void setSignalActions(int setSig){
	switch(setSig){

	case SIGUSR1:
		struct sigaction act;
		act.sa_handler = usr1Action;
		act.sa_flags = 0;
		sigaction(SIGUSR1, &act, NULL);
		break;
		
	case SIGTERM:
		struct sigaction term;
		term.sa_handler = termAction;
		term.sa_flags = 0;
		sigaction(SIGTERM, &term, NULL);
		break;
	
	case SIGUSR2:
		struct sigaction sdAct;
		sdAct.sa_handler = SIG_IGN;
		sdAct.sa_sigaction = usr2Action;
		sdAct.sa_flags = SA_SIGINFO;
		sigaction(SIGUSR2, &sdAct, NULL);
		break;
		
	case SIGPIPE:
		struct sigaction pAct;
		pAct.sa_handler = SIG_IGN;
		pAct.sa_flags = 0;
		sigaction(SIGPIPE, &pAct, NULL);
		break;
	
	default:
		break;
	}
	return;
}

std::vector<std::string> getArgs(int argc, char* argv[]){
	std::vector<std::string> args;
	for(int i = 1; i < argc; i++){
		//If "--shutdown" exists, insert as first argument
		if(std::string(argv[i]) == "--shutdown"){
			args.insert(args.begin(), argv[i]);
		} else {
			args.push_back(argv[i]);
		}
	}
	return args;
}

std::pair<std::string, bool> parseArgs(std::vector<std::string> args){
	for(unsigned int i = 0; i < args.size(); i++){
		if(args[i] == "--shutdown"){
			std::cout << "endrund: Shutting down current process" << std::endl;
			std::ifstream pidLog("logs/endrund/endrund.pid", std::ifstream::in);
			pid_t currentPID;
			pidLog >> currentPID;
			pidLog.close();

			if(currentPID){
				int status = kill(currentPID, SIGUSR1);
				if(status){
					std::cout << "endrund: Could not shut down process or PID not found" << std::endl;
					return std::pair<std::string, bool>(args[i], false);
				} else {
					std::cout << "endrund: Process shutdown successfully" << std::endl;
					std::remove("logs/endrund/endrund.pid");
					return std::pair<std::string, bool>(args[i], true);;
				}
			} else {
				std::cout << "endrund: Could not retrieve current PID, corrupt logs" << std::endl;
				return std::pair<std::string, bool>(args[i], false);
			}
		} else if (args[i] == "--port" || args[i] == "-p"){
			i++;
			if (i >= args.size()){
				std::cerr << "endrund: No port number specified";
				return std::pair<std::string, bool>(args[i], false);
			} else {
				return std::pair<std::string, bool>(args[i], true);
			}
		} else if (args[i] == "--help" || args[i] == "-h") {
			std::cout << "endrund: Usage:" << std::endl
					  << "\tDefault: " << std::endl
					  << "\t\t./endrund " << std::endl 
					  << "\t\tStarts process on port 52010" << std::endl << std::endl
					  << "\t\t./endrund [-p|--port] [port_number]" << std::endl
					  << "\t\tStarts process on specified port number" << std::endl << std::endl
					  << "\t\t./endrund --shutdown" << std::endl
					  << "\t\tShuts down current process" << std::endl;
			return std::pair<std::string, bool>("--shutdown", true);
		} else {
			std::cerr << "endrund: Invalid argument \"" << args[i] << "\"" << std::endl;
			return std::pair<std::string, bool>(args[i], false);
		}
	}
	return std::pair<std::string, bool>(DEFAULT_PORT, true);
}

//Checks if Log path Exists
int checkLogsPath(){
	
	int status = access("logs", F_OK);
	if(status){
		status = mkdir("logs", S_IRWXU | S_IRWXG);
		if(status){
			return -1;
		}
	}
	status = access("logs/endrund", F_OK);
	if(status){
		status = mkdir("logs/endrund", S_IRWXU | S_IRWXG);
		if(status){
			return -1;
		}
	}
	status = access("/logs/endrund/endrund.hs", F_OK);
	int hs = open("logs/endrund/endrund.hs", 0);
	if(hs != -1){
		int n = read(hs, &currentHighscore, sizeof(int));
		if(n != sizeof(int)){
			currentHighscore = 0;
		}
		close(hs);
	} else {
		currentHighscore = 0;
	}

	return 0;
}


int buildServer(std::string portno){
	//Create socket to listen on; Port currently arbitrary
	inetSock servSock(portno);
	
	//Listen on open file descriptor, accept up to 10 connections
	listen(servSock.getFD(), 10);
	
	//Set up fd_set to poll the listening file descriptor for events
	fd_set acceptfd;
	FD_ZERO(&acceptfd);
	FD_SET(servSock.getFD(), &acceptfd);
	
	//Game, and gamePID tracking
	std::vector< std::pair<int,int> > games;
	std::vector<int> gamePID;
	
	//Temporary players pair
	std::pair<int,int> players;
	//Keeps count of current total players
	int playerCount = 0;
 
	setSignalActions(SIGUSR1);
	setSignalActions(SIGUSR2);
	setSignalActions(SIGPIPE);
	
	char* buf1, *buf2;
	buf1 = (char*)malloc(16 * sizeof(char));
	buf2 = (char*)malloc(16 * sizeof(char));
	
	//Server Loop
	while(1){
		
		bzero(buf1, 16 * sizeof(char));
		bzero(buf2, 16 * sizeof(char));
		
		if(signalFlag){
			//Closing actions go here!
			//signal child processes
			for(int i = 0; i < gamePID.size(); i++){
				kill(gamePID[i], SIGTERM);
			}
			//retrieve high scores as signals come in
			//wait for child processes
			for(int i = 0; i < gamePID.size(); i++){
				waitpid(gamePID[i], NULL, 0);
			}
			int fd = open("logs/endrund/endrund.hs", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
			if(fd != -1){
				int n = write(fd, &currentHighscore, sizeof(int));
				close(fd);
			}
			free(buf1);
			free(buf2);
			//exit
			return 0;
		} 
		
		int fd = accept(servSock.getFD(), NULL, NULL);

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
				std::string chs = "S" + std::to_string(currentHighscore);
				int n1 = write(players.first, chs.c_str(), chs.size());   //First HS
				if(n1 == 0 || n1 == -1){
					players.first =  0;
					playerCount--;
				}
			} else {
				//Store player in second players slot
				players.second = fd;
				playerCount++;
				std::string chs = "S" + std::to_string(currentHighscore);
				
				int n1 = write(players.first, chs.c_str(), chs.size());   //First HS
				int n2 = write(players.second, chs.c_str(), chs.size());
				
				if(n1 == 0 || n1 == -1){
					players.first = players.second;
					players.second = 0;
					playerCount--;
					continue;
				}
				
				n1 = read(players.first, buf1, 16*sizeof(char));	//Read for start
				n2 = read(players.second, buf2, 16*sizeof(char));		//CTRL+C or Q will close cliSock
				if(std::string(buf1) == "start" && std::string(buf2) == "start"){ //If both start
					n1 = write(players.first, "1", chs.size());
					n2 = write(players.second, "2", chs.size());
				} else {
					
					if(n2 == 0 || n2 == -1){
						std::cout << "p2 bad read: " << buf2 << std::endl;
						players.second = 0;
						playerCount--;
					}
					if(n1 == 0 || n1 == -1){
						std::cout << "p1 bad read: " << buf1 << std::endl;
						players.first = players.second;
						players.second = 0;
						playerCount--;
					}
					continue;
				}
				
				//Store pair of players
				games.push_back(players);
				//Fork a game for players
				pid_t pid = fork();
				//If pid == 0, child = true, else child = false
				bool child = (pid == 0) ? true : false;
				//If process is child....
				if(child){
					//Sets SIGTERM actions for game process
					setSignalActions(SIGTERM);
					//Child Process
					playGame(games[playerCount/2 - 1]);
					free(buf1);
					free(buf2);
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
	srand(time(NULL));
	//Creates dummy inetSockets to allow reading from file descriptors
	inetSock player1(player.first);
	inetSock player2(player.second);

	std::string p1msg, p2msg;

	fd_set players;
	FD_ZERO(&players);

	int highFD = (player1.getFD() > player2.getFD()) ? player1.getFD() : player2.getFD();

	struct timeval timeout;

	sleep(3);
	
    std::chrono::high_resolution_clock::time_point score1 = std::chrono::high_resolution_clock::now();
    std::chrono::high_resolution_clock::time_point enemy_life_s, enemy_s, enemy_f, pit_s, pit_f;

	enemy_s = std::chrono::high_resolution_clock::now();
	
	pit_s = std::chrono::high_resolution_clock::now();
	int pitRandPeriod = rand() % 8;
	int enemyRand = rand() % 6;
	
	while(player1.isOpen() && player2.isOpen()){

		p1msg = p2msg = "H";
		
		timeout.tv_sec = 0;
		timeout.tv_usec = POLL_PERIOD_MICRO;
		
		FD_SET(player1.getFD(), &players);
		FD_SET(player2.getFD(), &players);
		
		select(highFD+1, &players, NULL, NULL, &timeout);
		if(FD_ISSET(player1.getFD(), &players)){
			p1msg += player1.readFromSock(512);
			if (p1msg == "H "){
				player1.writeToSock("HA");
				player2.writeToSock("HA");
			} else if(p1msg.size() == 1){
				player1.Close();
				player2.writeToSock("KILL");
			} else {
				player1.writeToSock(p1msg);
				player2.writeToSock(p1msg);
			}
		}
		
		if(FD_ISSET(player2.getFD(), &players)){
			p2msg += player2.readFromSock(512);
			if (p2msg == "H "){
				player1.writeToSock("HJ");
				player2.writeToSock("HJ");
			} else if(p2msg.size() == 1){
				player2.Close();
				player1.writeToSock("KILL");
			} else {
				player1.writeToSock(p2msg);
				player2.writeToSock(p2msg);
			}
		}
		
		enemy_f = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> enemy_cont = (enemy_f - enemy_s);
	
		if(enemy_cont.count() > (double)POLL_PERIOD_MILLI/2.0){
			
			enemy_s = enemy_f;			//Reset start point
			std::chrono::duration<double, std::milli> enemy_life = (enemy_f - enemy_life_s);
			std::string enemyCommand = "E";//Prep msg
			if(enemy_life.count() < 5000.0 + (enemyRand * POLL_PERIOD_MILLI)){
				int enemyDir = rand() % 2;	//Select direction randomly
	
				//Up Encoding
				if(enemyDir == 0){
				      enemyCommand += "0";		//Up
				//Down Encoding
				} else {
				      enemyCommand += "1";		//Down
				}
				
			} else {
				int randE = rand() % 2;
				enemyRand = rand() % 6;
				enemyCommand += "S";
				enemyCommand += std::to_string(randE);			//Spawn
				std::cout << enemyCommand << std::endl;
				enemy_life_s = enemy_f;
			}
			player1.writeToSock(enemyCommand);
			player2.writeToSock(enemyCommand);
		}
		pit_f = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double, std::milli> pit_cont = (pit_f - pit_s);
		
		if(pit_cont.count() > (4 + pitRandPeriod)*(double)POLL_PERIOD_MILLI){
			pit_s = pit_f;				//Reset clock
			pitRandPeriod = rand() % 8;	//New random number of periods
			int randPit = rand() % 10;
			std::cout << "P" << randPit << std::endl;
			player1.writeToSock("P" + std::to_string(randPit));	//Trigger new pit
			player2.writeToSock("P" + std::to_string(randPit));	//Trigger new pit
		}
	
		std::chrono::high_resolution_clock::time_point score2 = std::chrono::high_resolution_clock::now();
	    std::chrono::duration<double> score = std::chrono::duration_cast<std::chrono::duration<double>>(score2 - score1);
		
		currentHighscore = (int)(100 * score.count());
		player1.writeToSock("S" + std::to_string(currentHighscore));
		player2.writeToSock("S" + std::to_string(currentHighscore));
	}
	player1.Close();
	player2.Close();
	termAction(0); //Passed int is irrelevant
}