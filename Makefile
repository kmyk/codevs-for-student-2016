.PHONY: archive build client run
default: build

CXX := g++
CXXFLAGS := -std=c++14 -Wall -Wno-sign-compare -O3 -mtune=native

archive:
	- rm Main.zip
	zip Main.zip run.sh Makefile Main.cpp

build: build/fast
build/debug/slow:
	$(CXX) $(CXXFLAGS) -g -fsanitize=undefined -D_GLIBCXX_DEBUG Main.cpp
build/debug:
	$(CXX) $(CXXFLAGS) -g Main.cpp
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

logfile := codevsforstudent/log.txt
tmplogfile := codevsforstudent/log.txt.$(shell date +%s)
log:
	cat ${logfile}
log/update:
	curl http://52.198.238.77/codevsforstudent/log/ | pandoc -f html -t plain | sed 's/^ \+//' >> ${tmplogfile}
	touch ${logfile}
	cat ${logfile} ${tmplogfile} | sort | uniq | sponge ${logfile}

userfile := codevsforstudent/user.json
tmpuserfile := codevsforstudent/user.json.$(shell date +%s)
ranking:
	cat ${userfile} | tail -n 5000 | jq -r '.[] | ( .name, .rating )' | while read user ; do read rating ; win=$$($(MAKE) log | grep kimiyuki_win- | grep $${user}_lose- | wc -l) ; lose=$$($(MAKE) log | grep kimiyuki_lose- | grep $${user}_win- | wc -l) ; echo $$user'\t'$$win-$$lose'\t'$$rating | expand -t 20 ; done
ranking/update:
	curl 'http://52.198.238.77/codevsforstudent/user?course=Hard' > ${userfile}
