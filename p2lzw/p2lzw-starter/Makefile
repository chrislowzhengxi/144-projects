HEADERS = decode.h encode.h bit_shift.h
OBJECTS = program.o decode.o encode.o bit_shift.o

default: program

program.o: main.c $(HEADERS)
	gcc -c main.c -o program.o

decode.o: decode.c decode.h
	gcc -c decode.c -o decode.o

encode.o: encode.c encode.h
	gcc -c encode.c -o encode.o

bit_shift.o: bit_shift.c bit_shift.h
	gcc -c bit_shift.c -o bit_shift.o

program: $(OBJECTS)
	touch encode
	touch decode
	rm -f decode
	rm -f encode
	
	gcc $(OBJECTS) -o program

	ln -s program decode
	ln -s program encode

# Test target for bit_shift.h
test_bit_shift: test_bit_shift.c bit_shift.c bit_shift.h
	gcc test_bit_shift.c bit_shift.c -o test_bit_shift
	./test_bit_shift

clean:
	-rm -f $(OBJECTS)
	-rm -f program
	-rm -f decode
	-rm -f encode
	-rm -f test_bit_shift