P2: p2.o queue.o file.o analysis.o strbuf.o
        gcc -pthread -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -o p2 p2.o queue.o file.o analysis.o strbuf.o -lm

P2.o: p2.c queue.h file.c analysis.c strbuf.h
        gcc —pthread c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined p2.c -lm

file.o: file.c strbuf.h
	gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined file.c -lm

analysis.o: analysis.c strbuf.h
	gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined analysis.c -lm

queue.o: queue.c queue.h
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined queue.c -lm

strbuf.o: strbuf.c strbuf.h
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined strbuf.c -lm

# separate version of strbuf.o with debugging text enabled
dstrbuf.o: strbuf.c strbuf.h
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -DDEBUG -o dstrbuf.o strbuf.c -lm

dqueue.o: queue.c queue.h 	gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -DDEBUG -o dqueue.o queue.c -lm

dfile.o: file.c
	gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -DDEBUG -o dfile.o file.c -lm

danalysis.o: analysis.c
	gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -DDEBUG -o danalysis.o analysis.c -lm

dp2: p2.o dqueue.o dfile.o danalysis.o dstrbuf.o
        gcc -pthread -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -o p2 p2.o dstrbuf.o dqueue.o dfile.o danalysis.o -lm

clean:
        rm -f *.o p2 dp2
