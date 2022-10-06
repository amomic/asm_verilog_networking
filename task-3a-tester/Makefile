SHELL := /bin/bash

.PHONY: test, run, clean
test: clean
	rsync -aq --ignore-missing-args ../task-3a/{*.cpp,*.h} .
	mkdir -p framework
	cp __test/test.cpp __test.cpp
	cp __test/framework/userspace.h framework
	mkdir -p test_output
	rm -f test_output/*
	g++ -std=c++17 -Wall -Werror -g *.cpp -o test_firewall
	rm -f *.cpp *.h
	valgrind --leak-check=full --show-leak-kinds=all --show-error-list=yes ./test_firewall

clean:
	rm -rf test_firewall test_output framework *.cpp *.h

run: test
