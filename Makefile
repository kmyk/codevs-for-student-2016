.PHONY: archive build client run
default: build

CXX := g++
CXXFLAGS := -std=c++14 -Wall -Wno-sign-compare -O3 -mtune=native

archive:
	- rm Main.zip
	zip Main.zip run.sh Makefile Main.cpp

build: build/fast
build/debug:
	$(CXX) $(CXXFLAGS) -g -fsanitize=undefined -D_GLIBCXX_DEBUG Main.cpp
build/fast:
	$(CXX) $(CXXFLAGS) -DNDEBUG Main.cpp
build/fast/profile-generate:
	- rm a.out Main.gcda
	$(CXX) $(CXXFLAGS) -DNDEBUG -fprofile-generate Main.cpp
	# note: use "sh -c ./a.out"
build/fast/profile-use:
	[ -e Main.gcda ]
	$(CXX) $(CXXFLAGS) -DNDEBUG -fprofile-use Main.cpp

client:
	javaws codevs_for_student.jnlp &
