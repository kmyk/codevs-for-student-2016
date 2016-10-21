.PHONY: build client
default: build

build:
	g++ -std=c++11 -O2 -Wall -Wno-sign-compare -LLOCAL -g -fsanitize=undefined -D_GLIBCXX_DEBUG Main.cpp
build/fast:
	g++ -std=c++14 -O3 -Wall -Wno-sign-compare -LLOCAL -DNDEBUG Main.cpp
build/remote:
	g++ -std=c++11 -O2 Main.cpp

client:
	javaws codevs_for_student.jnlp | nkf &
