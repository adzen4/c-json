FLAGS = -Wall -Wextra -pedantic -std=c23

json.o: json.c
	cc json.c -c -o json.o $(FLAGS)
json_test.o: json_test.c
	cc json_test.c -c -o json_test.o $(FLAGS)

test: json_test.o json.o
	cc json_test.o json.o -o test $(FLAGS)

.PHONY: clean
clean:
	rm test *.o
