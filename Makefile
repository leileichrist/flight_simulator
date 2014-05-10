lCXX = g++
LD = g++
CXXFLAGS = -Wall  -g -ggdb
EXEC = fly
LDFLAGS = -o $(EXEC) -L/usr/lib -lGL -lGLU -lglut -lGLEW
SOURCES = fly.cpp
OBJS = $(SOURCES:.cpp=.o)


$(EXEC) : $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS)  

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^

clean:
	rm -rf *.o $(EXEC)

