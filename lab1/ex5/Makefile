CC = gcc
CFlags = -Wall
OBJS = bar.o foo.o 
T = main

$(T): bar.o foo.o
	$(CC) $(CFlags) $^ -o $@

bar.o: bar.c bar.h 
	$(CC) $(CFlags) -c $< -o $@

foo.o: foo.c foo.h bar.h 
	$(CC) $(CFlags) -c $< -o $@

clean: 
	rm -f $(OBJS) $(T)


# main: bar.o foo.o
# 	gcc -Wall bar.o foo.o -o main

# bar.o: bar.c bar.h
# 	gcc -Wall -c bar.c -o bar.o

# foo.o: foo.c foo.h bar.h
# 	gcc -Wall -c foo.c -o foo.o

# clean:
# 	rm -f foo.o bar.o main


