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
		
		std::streambuf *coutbuf = std::cout.rdbuf();
		std::cout.rdbuf(logs.rdbuf());
		std::streambuf *cerrbuf = std::cerr.rdbuf();
		std::cerr.rdbuf(logs.rdbuf());
		
		buildServer(portno);
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
	
	//Server Loop
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
	std::string play1Msg = player1.readFromSock(512);
	std::cout << "Server: Read from player1 - " << play1Msg << std::endl;
	player2.writeToSock(play1Msg, play1Msg.size());
	
	std::cout << "Server: Testing " << player.second << " read..." << std::endl;
	std::string play2Msg = player2.readFromSock(512);
	std::cout << "Server: Read from player2 - " << play2Msg << std::endl;
	player1.writeToSock(play2Msg, play2Msg.size());
	
}