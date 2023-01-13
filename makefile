CC = arm-linux-gnueabi-gcc
CCFLAGS = -Wall -static
FLAGS = -lev3dev-c -lm -lpthread
LIBS = -L../robot/ev3dev-c/lib
INC = -I../robot/ev3dev-c/source/ev3
OBJ = main.o communication.o mdd.o myev3.o time_util.o workers.o
EXEC = main
$(EXEC) : %: %.o $(OBJ)
	$(CC) $(CCFLAGS) -o $@ $^ $(FLAGS)
%.o : %.c
	$(CC) $(LIBS) $(INC) -o $@ -c $<

all : $(EXEC)