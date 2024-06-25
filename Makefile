compile:
	gcc example.c -o ssh_example -lssh

run:
	./ssh_example

clear:
	rm -fr ssh_example