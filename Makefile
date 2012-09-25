# Makefile for HW1
# by: Cody Moore
# CSE 306

# C-Compiler (default: GNU C-Compiler)
CC=gcc

# Compile flags (default:
# -O2 : Optimize (more)
# -g  : Debug info for OS
# -Wall -Werror : all warnings = errors
CFLAGS=-g -O2 -Wall -Werror

SRC=bf_cfb64.c \
	bf_enc.c \
	bf_skey.c \
	cipher.c

OBJECTS=bf_cfb64.o \
	bf_enc.o \
	bf_skey.o \
	cipher.o

EXEC_FILE=cipher

##### MAIN RULE ######
$(EXEC_FILE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $(OBJECTS)

##### OBJECT RULES #####
#%.o: %.c
#	$(CC) $(CFLAGS) -c $<

bf_cfb64.o: bf_cfb64.c blowfish.h bf_locl.h
	$(CC) $(CFLAGS) -c $<

bf_enc.o: bf_enc.c blowfish.h bf_locl.h
	$(CC) $(CFLAGS) -c $<

bf_skey.o: bf_skey.c blowfish.h bf_locl.h bf_pi.h
	$(CC) $(CFLAGS) -c $<

cipher.o: cipher.c blowfish.h
	$(CC) $(CFLAGS) -c $<

##### CLEAN RULE #####
clean:
	rm *.o $(EXEC_FILE)
