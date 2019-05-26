CXX			= g++ -std=c++11
CXXFLAGS	= -g -Wall
OBJS		= *.cpp
PROG		= scc

all:		$(PROG)

$(PROG):	$(OBJS)
			$(CXX) -o $(PROG) $(OBJS)

clean:;		$(RM) $(PROG) core *.o *.s *.out
