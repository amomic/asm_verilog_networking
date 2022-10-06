.PHONY: run, analyze, valgrind, clean

server: *.cpp
	g++ -std=c++17 -Wno-unused-variable -Wall -Werror -ggdb -O0 *.cpp -lavcodec -lavformat -lavutil -lm -o server 

run: server
	./server $(fname)

analyze: *.cpp
	cppcheck --suppress=missingIncludeSystem --enable=all .

clean:
	rm -rf server

valgrind: server
	valgrind --leak-check=full --suppressions=./suppression.supp ./server $(fname)

asan: *.cpp
	g++ -std=c++17 -fsanitize=address -Wno-unused-variable -Wall -Werror -ggdb -O0 *.cpp -lavcodec -lavformat -lavutil -lm -o server && ./server

