.PHONY: jsh

slash : jsh.c
	@gcc -Wall -o jsh jsh.c -lreadline

clean :
	@rm jsh

