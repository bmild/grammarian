CXX=clang++
CXXFLAGS=-std=c++11 -g -O3
GLUT=-framework OpenGL -framework GLUT
# DEPS = automata.h cfg.h regex.h regex.cpp symbol.h grammar.h grammar.cpp vector.h
OBJ = parser.o regex.o grammar.o

parser: $(OBJ)
	$(CXX) -o parser $^ $(CXXFLAGS)

parser.o: automata.h cfg.h regex.o symbol.h grammar.o parser.cpp lexer.h
	$(CXX) $(CXXFLAGS) -c parser.cpp

grammar.o: automata.h cfg.h regex.o symbol.h grammar.h grammar.cpp lexer.h
	$(CXX) $(CXXFLAGS) -c grammar.cpp

regex.o: automata.h cfg.h symbol.h regex.h regex.cpp
	$(CXX) $(CXXFLAGS) -c regex.cpp


graphviz: graphviz.cpp vector.h
	$(CXX) -std=c++11 -O3 -Wno-deprecated graphviz.cpp -o graphviz $(GLUT)
