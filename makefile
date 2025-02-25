all:
	g++ -g -Wall -o test *.c tests/*.cpp -lCppUTest -lCppUTestExt
clean:
	rm prog test
