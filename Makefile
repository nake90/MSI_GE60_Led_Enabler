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
OBJS=$(COBJS) $(CPPOBJS)
CFLAGS+=`pkg-config --cflags hidapi-libusb` -Wall -g -c
LIBS=-ludev `pkg-config --libs hidapi-libusb`

CPPOBJS_SENSORS=sensors_sample.o
CFLAGS_SENSORS+=$(CFLAGS)
LIBS_SENSORS+=-lsensors $(LIBS)

COBJS_DAEMON=daemon.o
CFLAGS_DAEMON+=$(CFLAGS)
LIBS_DAEMON+=-lsensors $(LIBS)

all: msiledenabler sensors_sample msige60d

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

msige60d: $(COBJS_DAEMON)
	g++ -Wall -g $^ $(LIBS_DAEMON) -o msige60d

$(COBJS_DAEMON): %.o: %.c
	$(CC) $(CFLAGS_DAEMON) $< -o $@

clean:
	rm -f *.o msiledenabler $(CPPOBJS) sensors_sample $(CPPOBJS_SENSORS) msige60d $(CPPOBJS_DAEMON)

.PHONY: clean
