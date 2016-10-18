.PHONY: build client
default: build

build:
	g++ -std=c++11 -O2 -Wall -g -fsanitize=undefined -D_GLIBCXX_DEBUG Main.cpp
build/fast:
	g++ -std=c++11 -O2 -Wall -DNDEBUG Main.cpp

client:
	javaws codevs_for_student.jnlp | nkf &
