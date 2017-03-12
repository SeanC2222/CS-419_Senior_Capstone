#G++
CXX = g++
#Use latest Compiler Tool Chain
CXXFLAGS = -std=c++11
#Show Warnings
#CXXFLAGS += -Wall
#Optimization
CXXFLAGS += -O3
#Strict ISO compilation rules
#CXXFLAGS += -pedantic-errors
#New rule for the screen
CXXSCREENFLAGS = ${CXXFLAGS} -lpanel -lncurses -pthread -g

#Debugging Flag for GDB (DEFAULT COMMENTED OUT)
#CXXFLAGS += -g

SRC1 = endrun.cpp
SRC2 = ../inetLib/inetLib.cpp
SRC3 = screen.cpp
SRC4 = window.cpp
SRC5 = 
SRC6 =
SRC7 =
SRC8 =
SRC9 =

SRCS = ${SRC1} ${SRC2} ${SRC3} ${SRC4}
SCREENC = ${SRC3} ${SRC4} ${SRC5}

HDR1 = ../inetLib/inetLib.hpp
HDR2 = screen.hpp
HDR3 = window.hpp
HDR4 = 
HDR5 =
HDR6 =
HDR7 =
HDR8 =
HDR9 =

HDRS = ${HDR1} ${HDR2} ${HDR3}
SCREENH = ${HDR2} ${HDR3}

PROG = endrun

all: ${PROG}

OBJS: ${SRCS} ${HDRS}
	${CXX} ${CXXFLAGS} ${SRCS} -c $(@.o=.cpp)
	
${PROG}: OBJS
	${CXX} *.o ${HDRS} ${CXXSCREENFLAGS} -o ${PROG}

# Two sections below added by Dylan for screen testing
SCREENOBJS: ${SCREENC} ${SCREENH}
	${CXX} -c $(@.o=.cpp) $^ ${CXXSCREENFLAGS}
	
screenTest: SCREENOBJS 
	$(CXX) -o $@ *.o ${CXXSCREENFLAGS}

clean:
	rm -f  *.o *~ *.gch  ${PROG} screenTest
