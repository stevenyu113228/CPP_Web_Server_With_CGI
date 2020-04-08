all: Host get post

CC = g++
Host: host.cpp
	$(CC) -o Host host.cpp

get: get.cpp
	$(CC) -o get.cgi get.cpp

post: post.cpp
	$(CC) -o post.cgi post.cpp


.PHONY: clean,run
clean:
	rm Host post.cgi get.cgi

test:
	./Host