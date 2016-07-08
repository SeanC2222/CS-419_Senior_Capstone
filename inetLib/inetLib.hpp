#ifndef _INETLIB_H
#define _INTELIB_H

#include <string>
#include <netinet/ip.h>
#include <netdb.h>

class inetSock{

private:
   int sFD;                      //Socket File Descriptor
   int pNo;                      //Socket port number
   int open;                     //Bool check for socket bound or not (i.e., is client or not)
   struct addrinfo addr;         //Stores client address info if b == 0
   struct addrinfo hints;        //Hints to create Socket
   struct addrinfo *addrRes;     //Internal use for _getAddressInfo

   void _getAddressInfo(const char*);
   void _getAddressInfo(const char*, const char*);
   
   void _bindSocket(const char*);
   void _connectSocket(const char*, const char*);
   
   void _dumpState();
   
public:

   //Bare constructor
      //sFD		   = 0
      //pNo		   = 0
      //b		   = 0
      //addr		   = 0
      //hints.ai_family	   = AF_INET
      //hints.ai_socktype  = SOCK_STREAM
      //hints.ai_flags	   = AI_PASSIVE
      //hints.ai_protocol  = 0
   inetSock();
   //Copy constructor
   inetSock(const inetSock&);
   //Listening Socket constructor
   inetSock(const char*);
   inetSock(const std::string);
   //Connecting Socket constructor
   inetSock(const char*, const char*,const char*);
   inetSock(const std::string, const std::string,const std::string);
   //FD Wrapper for Read Operations
   inetSock(int);
   //Copy Operator
   inetSock& operator=(const inetSock&);
   //Destructor
   ~inetSock();
   
   int getFileDescriptor() const;
   int getPortNumber() const;
   int isOpen() const;
   struct addrinfo getAddr() const;
   struct addrinfo getHints() const;

   //Data Functions
   int writeToSock(const std::string, const int);
   int writeToSock(const char*, const int);
   std::string readFromSock(const int);
};

#endif