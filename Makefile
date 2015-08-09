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

all: msiledenabler sensors_sample

CC=gcc
CXX=g++
COBJS=
CPPOBJS=msiledenabler.o
CPPOBJS_SENSORS=sensors_sample.o
OBJS=$(COBJS) $(CPPOBJS)
CFLAGS+=`pkg-config --cflags hidapi-libusb` -Wall -g -c
LIBS=-ludev `pkg-config --libs hidapi-libusb`
CFLAGS_SENSORS+=$(CFLAGS)
LIBS_SENSORS+=-lsensors $(LIBS)

msiledenabler: $(OBJS)
	g++ -Wall -g $^ $(LIBS) -o msiledenabler

$(COBJS): %.o: %.c
	$(CC) $(CFLAGS) $< -o $@

$(CPPOBJS): %.o: %.cpp
	$(CXX) $(CFLAGS) $< -o $@

sensors_sample: $(CPPOBJS_SENSORS)
	g++ -Wall -g $^ $(LIBS_SENSORS) -o sensors_sample

$(CPPOBJS_SENSORS): %.o: %.cpp
	$(CXX) $(CFLAGS_SENSORS) $< -o $@

clean:
	rm -f *.o msiledenabler $(CPPOBJS) sensors_sample $(CPPOBJS_SENSORS)

.PHONY: clean
