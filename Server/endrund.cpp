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

//Custom Libraries
#include "../inetLib/inetLib.hpp"

//Definitions
#define DEFAULT_PORT "52010"

//Tokenizes Command Line Arguments in vector
std::vector<std::string> getArgs(int, char**);
//Parses Command Line Arguments
std::pair<std::string,bool> parseArgs(std::vector<std::string>);
//Starts Server
int buildServer(std::string);
//Main Server Game Loop
void playGame(std::pair<int,int>);

int main(int argc, char* argv[]){
	
	//Default
	int status = chdir("/home/ubuntu/workspace");

	std::vector<std::string> args;
	
	std::string portno = DEFAULT_PORT;
	
	if(argc > 1){
		args = getArgs(argc, argv);
		std::pair<std::string, bool> status = parseArgs(args);
		if(!status.second){
			std::cerr << "endrund: Error with argument \"" + status.first + "\"" << std::endl;
			return -1;
		} else {
			if(status.first == "--shutdown"){
				return 0;
			} else {
				portno = status.first;
			}
		}
	}
	
	std::cout << "endrund: Starting daemon on port " << portno << std::endl;
	pid_t pid = fork();
	bool child = (pid == 0) ? true : false;

	if(child){

		std::ofstream logs("logs/endrund/endrundlog.txt", std::ofstream::out | std::ofstream::app);
		umask(0);
		pid_t sid = setsid();
		if(sid < 0){
			logs << "endrund: Unable to create new SID for daemon." << std::endl;
			exit(-1);
		}
		close(STDIN_FILENO);
		
		//std::streambuf *coutbuf = std::cout.rdbuf();
		//std::cout.rdbuf(logs.rdbuf());
		//std::streambuf *cerrbuf = std::cerr.rdbuf();
		//std::cerr.rdbuf(logs.rdbuf());
		
		buildServer(portno);
		//std::cout.rdbuf(coutbuf);
		//std::cerr.rdbuf(cerrbuf);
		exit(0);
	} else {
		if(pid < 0){
			std::cerr << "endrund: Daemon startup unsuccessful" << std::endl;
		} else {
			std::cout << "endrund: Daemon started as PID: " << pid << std::endl;
			std::ofstream pidLog("logs/endrund/endrund.pid", std::ofstream::out);
			pidLog << pid << std::endl;
			pidLog.close();
		}
	}
	return 0;
}

std::vector<std::string> getArgs(int argc, char* argv[]){
	std::vector<std::string> args;
	for(int i = 1; i < argc; i++){
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
			std::cout << "endrund: Shutting down current daemon" << std::endl;
			std::ifstream pidLog("logs/endrund/endrund.pid", std::ifstream::in);
			pid_t currentPID;
			pidLog >> currentPID;
			pidLog.close();

			if(currentPID){
				int status = kill(currentPID, SIGTERM);
				if(status){
					std::cout << "endrund: Could not shut down daemon or daemon not found" << std::endl;
					return std::pair<std::string, bool>(args[i], false);
				} else {
					std::cout << "endrund: Daemon shutdown successfully" << std::endl;
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
					  << "\t\tStarts daemon on port 52010" << std::endl << std::endl
					  << "\t\t./endrund [-p|--port] [port_number]" << std::endl
					  << "\t\tStarts daeomon on specified port number" << std::endl << std::endl
					  << "\t\t./endrund --shutdown" << std::endl
					  << "\t\tShuts down current daemon process" << std::endl;
			return std::pair<std::string, bool>("--shutdown", true);
		} else {
			std::cerr << "endrund: Invalid argument \"" << args[i] << "\"" << std::endl;
			return std::pair<std::string, bool>(args[i], false);
		}
	}
	return std::pair<std::string, bool>(DEFAULT_PORT, true);
}

int buildServer(std::string portno){
	//Create socket to listen on; Port currently arbitrary
	inetSock servSock(portno);
	
	//Listen on open file descriptor, accept up to 10 connections
	listen(servSock.getFD(), 10);
	
	//Set FD to non-blocking mode
	//fcntl(servSock.getFD(), F_SETFL, fcntl(servSock.getFD(), F_GETFL) | O_NONBLOCK);

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
	
 
	//Server Loop
	while(1){
		select(servSock.getFD()+1, &acceptfd, NULL, NULL, NULL);
		int fd = -1;
		//Accept a connection if available
		if(FD_ISSET(servSock.getFD(), &acceptfd)){
			fd = accept(servSock.getFD(), NULL, NULL);
		}
		//Allows binding of socket unless already being listened on
		//Not necessary, but useful for program crashes/reboots on the same port
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, NULL, 0);
		//If a connection was accepted...
		if(fd != -1){
			//If # of players is odd...
			if(playerCount % 2 == 0){
				//Store player in first players slot
				players.first = fd;
				write(players.first, "1", 2);
				playerCount++;
			} else {
				//Store player in second players slotj
				players.second = fd;
				write(players.second, "2", 2);
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
	
	fd_set players;
	FD_ZERO(&players);

	int highFD = (player1.getFD() > player2.getFD()) ? player1.getFD() : player2.getFD();
	int lowFD  = (player1.getFD() > player2.getFD()) ? player2.getFD() : player1.getFD();

	struct timeval timeout;

	while(player1.isOpen() && player2.isOpen()){
		
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
	
		FD_SET(player1.getFD(), &players);
		FD_SET(player2.getFD(), &players);
		
		std::string p1msg, p2msg;		
		select(highFD+1, &players, NULL, NULL, &timeout);
		
		if(FD_ISSET(player1.getFD(), &players)){
			p1msg = player1.readFromSock(512);
		}
		
		if(FD_ISSET(player2.getFD(), &players)){
			p2msg = player2.readFromSock(512);
		}
		
		if(p1msg.size() > 0){
			std::cout << "Player 1 pressed: " << p1msg << std::endl;
			std::cout << "Message Size: " << p1msg.size() << std::endl;
			player2.writeToSock(p1msg, 512);			
		}

		if (p2msg.size() > 0){
			std::cout << "Player 2 pressed: " << p2msg << std::endl;
			std::cout << "Message Size: " << p2msg.size() << std::endl;
			player1.writeToSock(p2msg, 512);
		}
	}
	
}