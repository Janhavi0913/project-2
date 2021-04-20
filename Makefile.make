p2: p2.o strbuf.o file.o analysis.o queue.o
        gcc -pthread -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -o p2 p2.o queue.o file.o analysis.o strbuf.o -lm

p2.o: p2.c queue.h file.c analysis.c strbuf.h
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined p2.c queue.h file.c analysis.c strbuf.h -lm

strbuf.o: strbuf.c strbuf.h
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined strbuf.c strbuf.h -lm

file.o: file.c strbuf.h
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined file.c strbuf.h -lm

analysis.o: analysis.c strbuf.h
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined analysis.c strbuf.h -lm

queue.o: queue.c queue.h
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined queue.c queue.h -lm

# separate version of strbuf.o with debugging text enabled                                                                                                                                                  
dstrbuf.o: strbuf.c strbuf.h
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -DDEBUG -o dstrbuf.o strbuf.c -lm

dqueue.o: queue.c queue.h
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -DDEBUG -o dqueue.o queue.c -lm

dfile.o: file.c
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -DDEBUG -o dfile.o file.c -lm

danalysis.o: analysis.c
        gcc -pthread -c -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -DDEBUG -o danalysis.o analysis.c -lm

dp2: p2.o dqueue.o dfile.o danalysis.o dstrbuf.o
        gcc -pthread -g -std=c99 -Wvla -Wall -fsanitize=address,undefined -o p2 p2.o dstrbuf.o dqueue.o dfile.o danalysis.o -lm

clean:
        rm -f *.o p2 dp2
