compile:
	gcc example.c -o ssh_example -lssh -lm

run:
	./ssh_example

clear:
	rm -fr ssh_example