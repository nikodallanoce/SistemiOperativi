all: main etcs1 etcs2 RBC
	
main: main.o
	cc main.o -o main
main.o: main.c
	cc -c main.c

etcs1: etcs1.o trainUtils.o
	cc etcs1.o trainUtils.o -o etcs1
etcs1.o: etcs1.c trainUtils.h
	cc -c etcs1.c

etcs2: etcs2.o trainUtils.o
	cc etcs2.o trainUtils.o -o etcs2
etcs2.o: etcs2.c trainUtils.h
	cc -c etcs2.c

RBC: RBC.o trainUtils.o
	cc RBC.o trainUtils.o -o RBC
RBC.o: RBC.c trainUtils.h
	cc -c RBC.c

trainUtils.o: trainUtils.c trainUtils.h
	cc -c trainUtils.c
