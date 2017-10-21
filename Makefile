myshell: myshell.c
	gcc -Wall -Werror -o myshell myshell.c

clean:
	rm -f myshell
