main:main.o search.o status.o utils.o
	g++ main.o search.o  status.o utils.o -o main.out
status.o:status.cpp
	g++ -c status.cpp -o status.o
utils.o:utils.cpp
	g++ -c utils.cpp -o utils.o
search.o:search.cpp
	g++ -c search.cpp -o search.o
main.o:main.cpp
	g++ -c main.cpp -o main.o
clean:
	rm -rf *.o
