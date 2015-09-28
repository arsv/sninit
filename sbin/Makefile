CC = gcc
CFLAGS = -g -Wall

all = trap slogdg slogst logger

all: $(all)

trap: trap.c

slogdg: slogdg.c

slogst: slogst.c

logger: logger.c

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(all) devlog daemon.log
