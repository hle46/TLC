CC = g++-5
CFLAGS = -Wall -Wextra -std=c++14 -g -funroll-loops -O3
LDFLAGS = -L/usr/local/lib -ljpeg -lpng
#LIBS = -stdlib=libc++
SRCS = main.cc imtoolbox.cc tlc.cc
OBJS = $(SRCS:.cc=.o)
EXES = main

all: $(EXES)

main: $(OBJS)
	$(CC) $(LIBS)  $^ -o $@ $(LDFLAGS)

%.o:%.cc
	$(CC) -c $(LIBS) $(CFLAGS) $< -o $@

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CC) $(CFLAGS) -MM $^>>./.depend;

include .depend

clean:
	rm -f *.o $(EXES) .depend
.PHONY: all clean


