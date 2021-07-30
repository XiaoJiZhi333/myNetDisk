SRCS:=$(wildcard *.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))
ELF:=server
CC:=gcc
CFLAGS:=-Wall -I /usr/include/mysql/ -I ../include/
$(ELF):$(OBJS)
		gcc $^ -o $@ -lpthread -lcrypt -lmysqlclient -lcrypto
.PYONY:clean
	clean:
		rm -rf $(OBJS) $(ELF)
		
