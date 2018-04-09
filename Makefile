C = gcc
NAME = 20131575
OBJS = $(NAME).c
TARGET = $(NAME).out

.SUFFIXES : .c .o

all : $(TARGET)
	
$(TARGET) : $(OBJS) $(NAME).h
			$(CC) -Wall -o $@ $(OBJS)

clean : 
	rm -f $(TARGET)

delete :
	rm -f *.lst *.obj
