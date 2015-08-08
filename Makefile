###########################################
# Simple Makefile for HIDAPI test program
#
# Alan Ott
# Signal 11 Software
# 2010-07-03
#
# Alfonso Arbona Gimeno
# 2015-08-08
# hidapi linked externally via pkg-config
###########################################

all: msiledenabler

CC=gcc
CXX=g++
COBJS=
CPPOBJS=msiledenabler.o
OBJS=$(COBJS) $(CPPOBJS)
CFLAGS+=`pkg-config --cflags hidapi-libusb` -Wall -g -c
LIBS=-ludev `pkg-config --libs hidapi-libusb`


msiledenabler: $(OBJS)
	g++ -Wall -g $^ $(LIBS) -o msiledenabler

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -f *.o msiledenabler $(CPPOBJS)

.PHONY: clean
