project:
	stty sane
	rm -f ./Frogger
	gcc -o Frogger frog.c functions.c -lncurses -pthread -lm
	clear
	./Frogger
	
	
build:
	gcc -o Frogger frog.c functions.c -lncurses -pthread -lm
clean: Frogger
	rm Frogger
fix:
	stty sane
	clear
run:
	Frogger
