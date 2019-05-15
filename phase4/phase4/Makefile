CXX		= g++ -std=c++11
CXXFLAGS	= -g -Wall
OBJS		= Scope.o Symbol.o Type.o checker.o lexer.o parser.o
PROG		= scc

all:		$(PROG)

$(PROG):	$(OBJS)
		$(CXX) -o $(PROG) $(OBJS)

clean:;		$(RM) $(PROG) core *.o


# dependencies

Scope.o:	Scope.h Symbol.h Type.h
Symbol.o:	Symbol.h Type.h
Type.o:		Type.h
checker.o:	lexer.h checker.h Scope.h Symbol.h Type.h
lexer.o:	lexer.h tokens.h
parser.o:	lexer.h tokens.h checker.h Scope.h Symbol.h Type.h
