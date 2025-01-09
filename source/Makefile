GTKFLAGS=`pkg-config --cflags gtk+-3.0`
CFLAGS=-O2
LIBFLAGS=`pkg-config --libs gtk+-3.0`

all: sgidls-gtk
	make clean; mv -f sgidls-gtk ../

debug: sgidls-gtk
	make clean; mv -f sgidls-gtk ../

debug: CFLAGS:=-g

sgidls-gtk: main.o parser.o scanner.o strings.o table.o
	gcc $(GTKFLAGS) $(CFLAGS) -o sgidls-gtk main.o parser.o scanner.o strings.o table.o $(LIBFLAGS)

main.o: main.c
	gcc $(GTKFLAGS) $(CFLAGS) -o main.o -c main.c $(LIBFLAGS)

parser.o: parser.c
	gcc $(GTKFLAGS) $(CFLAGS) -o parser.o -c parser.c $(LIBFLAGS)

scanner.o: scanner.c
	gcc $(GTKFLAGS) $(CFLAGS) -o scanner.o -c scanner.c $(LIBFLAGS)

strings.o: strings.c
	gcc $(GTKFLAGS) $(CFLAGS) -o strings.o -c strings.c $(LIBFLAGS)

table.o: table.c
	gcc $(GTKFLAGS) $(CFLAGS) -o table.o -c table.c $(LIBFLAGS)

clean:
	rm -f *.o

remove:
	rm -f *.o; rm -f ../sgidls-gtk
