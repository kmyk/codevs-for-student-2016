.PHONY: build client
default: build

build:
	g++ -std=c++11 -O2 -Wall -Wno-sign-compare -LLOCAL -g -fsanitize=undefined -D_GLIBCXX_DEBUG Main.cpp
build/fast:
	g++ -std=c++14 -O3                    -mtune=native -Wall -Wno-sign-compare -LLOCAL -DNDEBUG Main.cpp
build/fast/profile-generate:
	g++ -std=c++14 -O3 -fprofile-generate -mtune=native -Wall -Wno-sign-compare -LLOCAL -DNDEBUG Main.cpp
build/fast/profile-use:
	g++ -std=c++14 -O3 -fprofile-use      -mtune=native -Wall -Wno-sign-compare -LLOCAL -DNDEBUG Main.cpp
build/remote:
	g++ -std=c++11 -O2 Main.cpp

client:
	javaws codevs_for_student.jnlp &
