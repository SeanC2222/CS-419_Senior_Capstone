/*
* Author: Sean C Mulholland
* Date: 7/5/2016
* Description: This library is designed to manage simple ipv4 socket
*     creation and handling. Typical usage would be:
*     
*     Server:
*        inetSock servSock(port_number);  //Constructs listening socket
*     
*     Client:
*        inetSock cliSock(dest_host, dest_port, local_port); //Connects local_port to dest_port
*     
*     -------------------------------------------------------------------------
*
*     The library also supports simple sending and recieving of information if
*     connection was successful.
*     
*     Server | Client:
*        std::string data = sock.readFromSock(size_to_read);
*        int written = sock.writeToSock(message, size_to_write);
*
*     -------------------------------------------------------------------------
*  
*     No guarantees are made about connection states. If a peer disconnects,
*     the active object may reflect that the connection is still open. When
*     the destructor is called, the file descriptor is closed and any allocated
*     memory is freed.
*/ 


//C & Unix Libraries
#include <netinet/ip.h>    //Includes socket.h & netinet/in.h
#include <unistd.h>	      //Unix Standard
#include <sys/wait.h>	   //Flags for waiting functions
#include <string.h>        //Memory manipulations and c-string manipulations
#include <netdb.h>         //Struct addrinfo
#include <stdlib.h>
//C++ Libraries
#include <string>          //std::string
#include <iostream>        //cout, cerr
//Header
#include "inetLib.hpp"

//Internal function to get address info for binding
void inetSock::_getAddressInfo(const char* portno){
   //Get the address info in results
   int s = getaddrinfo(NULL, portno, &this->hints, &this->addrRes);
   //If there was an error...
   if(s != 0){
      std::cerr << "inetSock: Could not retrieve address info" << std::endl;
      std::cerr << "Error: " << gai_strerror(s) << std::endl;
      exit(-1);
   }
}

//Internal function to get address info for connecting
void inetSock::_getAddressInfo(const char* host, const char* portno){
   //Get the address info in results
   int s = getaddrinfo(host, portno, &this->hints, &this->addrRes);
   //If there was an error...
   if(s != 0){
      std::cerr << "inetSock: Could not retrieve address info" << std::endl;
      std::cerr << "Error: " << gai_strerror(s) << std::endl;
      exit(-1);
   }
}

//Internal function to bind current to socket
void inetSock::_bindSocket(const char* portno){
   struct addrinfo *ip;
   //Check results for an address that binds
   for(ip = this->addrRes; ip != NULL; ip = ip->ai_next){
      this->sFD = socket(ip->ai_family, ip->ai_socktype, ip->ai_protocol);
      if(this->sFD == -1) continue;
      //If the file descriptor binds, break
      if(!bind(this->sFD, ip->ai_addr, ip->ai_addrlen)){
	      break;
      }
      close(this->sFD); //Close unbound file descriptor
   }
   
   //If ip == NULL, no bound file descriptor, dump state and exit
   if(ip == NULL){
      std::cerr << "inetSock: Could not bind" << std::endl;
      _dumpState();
      exit(-1);
   }
   //Save portno, and set flag as open
   this->pNo = atoi(portno);
   this->open = 1;
   //Save peer address internally
   memcpy(&this->addr, ip, sizeof(struct addrinfo));
   //Free unused addrinfo linkedlist
   freeaddrinfo(this->addrRes);
   this->addrRes = NULL;
}

//Internal function to connect socket to destination socket
void inetSock::_connectSocket(const char* host, const char* portno){
   struct addrinfo *ip;
   //Check results for an address that connects
   for(ip = this->addrRes; ip != NULL; ip = ip->ai_next){
      this->sFD = socket(ip->ai_family, ip->ai_socktype, ip->ai_protocol);
      if(this->sFD == -1) continue;
      //If the file descriptor is connected, break
      if(!connect(this->sFD, ip->ai_addr, ip->ai_addrlen)){
	      break;
      }
      close(this->sFD); //Close unconnected file descriptor
   }
   //If ip == NULL, no connection, dump state and exit
   if(ip == NULL){
      std::cerr << "inetSock: Could not connect" << std::endl;
      _dumpState();
      exit(-1);
   }
   //Save portno, and set flag as open
   this->pNo = atoi(portno);
   this->open = 1;
   //Save peer address internally
   memcpy(&this->addr, ip, sizeof(struct addrinfo));
   //Free unused addrinfo linkedlist
   freeaddrinfo(this->addrRes);
   this->addrRes = NULL;
}

//Dumps state information as basic char interpretation
void inetSock::_dumpState(){
   std::cerr << "inetSock: State Data - " << std::endl
             << "\t sFD = " << this->sFD << std::endl
             << "\t pNo = " << this->pNo << std::endl
             << "\t open = " << this->open << std::endl
             << "\t addr = ";
   for(unsigned int i = 0; i < sizeof(struct addrinfo); i++){
      std::cerr << ((char*)&this->addr)[i];
   }
   std::cerr << std::endl << "\t hints = ";
   for(unsigned int i = 0; i < sizeof(struct addrinfo); i++){
      std::cerr << ((char*)&this->hints)[i];
   }
   std::cerr << std::endl << "\t addrRes = ";
   for(unsigned int i = 0; i < sizeof(struct addrinfo); i++){
      std::cerr << (char*)&this->addrRes[i];
   }
   std::cerr << std::endl;
}

//Bare constructor
inetSock::inetSock(){
   this->sFD = 0;
   this->pNo = 0;
   this->open = 0;
   memset(&this->addr, 0, sizeof(struct addrinfo));
   //Prepare address hints
   memset(&this->hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;
      hints.ai_protocol = 0;
   this->addrRes = NULL;
}

//Copy constructor
inetSock::inetSock(const inetSock &sock){
   this->sFD = sock.getFD();
   this->pNo = sock.getPortNumber();
   this->open = sock.isOpen();
   if(this->open == 1){
      this->addr = sock.getAddr();
   } else {
      memset(&this->addr, 0, sizeof(struct addrinfo));
   }
   this->hints = sock.getHints();
   this->addrRes = sock.addrRes;
   
}

//Listening Port Specific Constructor
   //This constructor accepts a port number as a c-string, then finds and binds
   //a socket to be listen ready.
   //this->sFD will be the file descriptor of the prepared socket
inetSock::inetSock(const char* portno){
   memset(&this->hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;
  
   _getAddressInfo(portno);
   _bindSocket(portno);
}
inetSock::inetSock(const std::string portno){
   memset(&this->hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
      hints.ai_flags = AI_PASSIVE;
  
   _getAddressInfo(portno.c_str());
   _bindSocket(portno.c_str());
}

//Connecting Port Specific Constructor
inetSock::inetSock(const char* destHost, const char* destPort, const char* portno){
   memset(&this->hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
   _getAddressInfo(destHost, destPort);
   _connectSocket(destHost, destPort);
}
inetSock::inetSock(const std::string destHost, const std::string destPort, const std::string portno){
   memset(&this->hints, 0, sizeof(struct addrinfo));
      hints.ai_family = AF_INET;
      hints.ai_socktype = SOCK_STREAM;
   _getAddressInfo(destHost.c_str(), destPort.c_str());
   _connectSocket(destHost.c_str(), destPort.c_str());
}
//FD Wrapper for Read Operations
inetSock::inetSock(int openFD){
   memset(this, 0, sizeof(inetSock));
   this->sFD = openFD;
   this->open = 1;
   this->addrRes = NULL;
}

//Copy operator
inetSock& inetSock::operator=(const inetSock& target){
   if(this != &target){
      this->sFD = target.getFD();
      this->pNo = target.getPortNumber();
      this->open = target.isOpen();
      this->addr = target.getAddr();
      this->hints = target.getHints();
   }
   return *this;
}

//Destructor
inetSock::~inetSock(){
   close(this->sFD);
   if(this->addrRes != NULL){
      freeaddrinfo(this->addrRes);
      this->addrRes = NULL;
   }   
}

//Get Member Functions
int inetSock::getFD() const {return this->sFD;}
int inetSock::getPortNumber() const {return this->pNo;}
int inetSock::isOpen() const {return this->open;}
int inetSock::Close(){
   this->open = close(this->sFD);
   return this->open;
}
struct addrinfo inetSock::getAddr() const {return this->addr;}
struct addrinfo inetSock::getHints() const{return this->hints;}

//Interaction Functions
int inetSock::writeToSock(const std::string data, const int size){
   int progress = 0, writeN = 0, writeCount = 0;
   //Tries to complete writing data up to 4 times
   while(progress < size && writeCount < 4){
      writeN += write(this->sFD, data.c_str() + progress, size - progress);
      progress += writeN;
      if(writeN == 0){
         writeCount++;
      } else {
         writeCount = 0;
      }
   }
   return progress;
}

int inetSock::writeToSock(const char* data, const int size){
   int progress = 0, writeN = 0, writeCount = 0;
   //Tries to complete writing data up to 4 times
   while(progress < size && writeCount < 4){
      writeN += write(this->sFD, data + progress, size - progress);
      progress += writeN;
      if(writeN == 0){
         writeCount++;
      } else {
         writeCount = 0;
      }
   }
   return progress;
}

std::string inetSock::readFromSock(const int size){
   char* data = (char*)malloc(size * sizeof(char));
   bzero(data, size * sizeof(char));
   int readN = read(this->sFD, data, size);
   std::string retString(data);
   free(data);
   if(readN != 0){
      return retString;
   } else {
      return "";
   }
}
