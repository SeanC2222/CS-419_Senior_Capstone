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

PROG = endrund

all: ${PROG}

OBJS: ${SRCS} ${HDRS}
	${CXX} ${CXXFLAGS} ${SRCS} -c $(@.o=.cpp)
	
${PROG}: OBJS
	${CXX} ${CXXFLAGS} *.o ${HDRS} -o ${PROG}

clean:
	rm -f  *.o *~ *.gch  ${PROG}