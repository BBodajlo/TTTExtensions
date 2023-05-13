CC = gcc -fsanitize=address -static-libasan -g
tttfunctions.o: tttfunctions.h tttfunctions.c
	$(CC) -c -g tttfunctions.c
ttts.o: ttts.c tttfunctions.h
	$(CC) -c -g ttts.c -pthread
ttt.o: ttt.c tttfunctions.h
	$(CC) -c -g ttt.c
ttt: ttt.o tttfunctions.o
	$(CC) -o ttt ttt.o tttfunctions.o -pthread
ttt2.o: ttt2.c tttfunctions.h
	$(CC) -c -g ttt2.c
ttt2: ttt2.o tttfunctions.o
	$(CC) -o ttt2 ttt2.o tttfunctions.o -pthread
ttts: ttts.o tttfunctions.o
	$(CC) -o ttts ttts.o tttfunctions.o 
clean:
	$(RM) -f *.o 