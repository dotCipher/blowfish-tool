# Makefile for HW1
# by: Cody Moore
# CSE 306

# C-Compiler (default: GNU C-Compiler)
CC=gcc

# Compile flags (default:
# -O2 : Optimize (more)
# -g  : Debug info for OS
# 
CFLAGS=-g -O2 -Wall -Werror

SRC=bf_cfb64.c \
	bf_enc.c \
	bf_skey.c \
	cipher.c

HDRS=bf_pi.h bf_locl.h

EXEC_FILE=cipher

$(EXEC_FILE): $(SRC) $(HDRS)
	$(CC) $(CFLAGS) -o $@ $(SRC)

clean:
	rm -rf *.o $(EXEC_FILE)
