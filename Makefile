TARGET  =main.exe
CC      =g++
CFLAGS  =-pedantic -Wall -Werror -Wno-sign-compare -Wno-long-long -lm -c -g 
SRC     =TTimer.cpp TDanger.cpp TPetry.cpp TSatellite.cpp interface.cpp main.cpp 
OBJS    =$(SRC:.cpp=.o)

.PHONY: all clean

all: $(TARGET) 
	@echo DONE

$(TARGET): $(OBJS)
	$(CC) $(DEBUG) $^ -lpthread -o $@ 
	
%.o: %.cpp
	$(CC) $(CFLAGS) $(DEBUG) $<

clean:
	rm -f *.o $(TARGET)

