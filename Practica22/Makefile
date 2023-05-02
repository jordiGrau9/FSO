cocos3 : cocos3.c winsuport2.o winsuport2.h memoria.o memoria.h
	gcc -Wall cocos3.c winsuport2.o memoria.o -o cocos3 -lcurses

Fantasmas3 : Fantasmas3.c winsuport2.o winsuport2.h memoria.o memoria.h
	gcc -Wall Fantasmas3.c winsuport2.o memoria.o -o Fantasmas3 -lcurses

winsuport2.o : winsuport2.c winsuport2.h
	gcc -Wall -c winsuport2.c -o winsuport2.o

memoria.o : memoria.c memoria.h
	gcc -Wall -c memoria.c -o memoria.o 

clean:
	rm -f cocos3 memoria.o Fantasmas3  winsuport2.o 
