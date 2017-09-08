
TARGET			:= procstat
OBJS			:= main.o procstat.o

CC              := gcc
CXX             := g++
CFLAGS			:= -Wall -pipe

.PHONY:	clean

all:	$(TARGET)

clean:
	@rm $(TARGET) $(OBJS)

$(TARGET):	$(OBJS)
	$(CC) -o $@ $^ $(LFALGS)

#main.o:	main.c

#procstat.o:	procstat.c

