#G++
CXX = g++
#Use latest Compiler Tool Chain
CXXFLAGS = -std=c++11
#Show Warnings
CXXFLAGS += -Wall
#Optimization
CXXFLAGS += -O3
#Strict ISO compilation rules
CXXFLAGS += -pedantic-errors
#Debugging Flag for GDB (DEFAULT COMMENTED OUT)
#CXXFLAGS += -g

SRC1 = endrund.cpp
SRC2 = ../inetLib/inetLib.cpp
SRC3 =
SRC4 =
SRC5 =
SRC6 =
SRC7 =
SRC8 =
SRC9 =

SRCS = ${SRC1} ${SRC2}

HDR1 = ../inetLib/inetLib.hpp
HDR2 = 
HDR3 =
HDR4 =
HDR5 =
HDR6 =
HDR7 =
HDR8 =
HDR9 =

HDRS = ${HDR1}

OBJ1 = endrund.o
OBJ2 = inetLib.o
OBJ3 = 
OBJ4 = 
OBJ5 = 
OBJ6 = 
OBJ7 = 
OBJ8 = 
OBJ9 = 
OBJS = ${OBJ1} ${OBJ2}

PROG = endrund

all: ${PROG}

${OBJS}: ${SRCS} ${HDRS}
	${CXX} ${CXXFLAGS} ${SRCS} -c $(@.o=.cpp)
	
${PROG}: ${OBJS}
	${CXX} ${CXXFLAGS} ${OBJS} ${HDRS} -o ${PROG}

clean:
	rm -f  *.o *~ *.gch  ${PROG}