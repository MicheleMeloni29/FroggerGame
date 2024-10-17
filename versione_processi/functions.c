#include "libs.h"

bool range(int min, int max, int value) {
    return (min <= value && value <= max);
}

void initScreen(){																				//inizializzazione schermo
    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, true);
    start_color();
    defineColors();
}

entity_s initEntity(int type, char extra, int x, int y, int health) { 							//inizializzazione entita'
    entity_s temp;
    temp.health = health;
    temp.extra = extra;
    temp.pos.x = x;
    temp.pos.y = y;
    temp.type = type;
    return temp;
}

void clearZone(int endX, int endY) {
    for (int y = 0; y < endY; y++) {
        for (int x = 0; x < endX + 10; x++) {
            mvaddch(y, x, ' ');
        }
    }
}

void printEnd(char * msg) {
    flash();
    flash();
    flash();

    int msgSize = strlen(msg);
    
    attron(COLOR_PAIR(2));
    mvprintw(S_HEIGHT / 2, S_WIDTH / 2 - msgSize / 2, msg);
    mvprintw(S_HEIGHT / 2 + 1, S_WIDTH / 2 - 12, "Premi \"ENTER\" per uscire");						//nel caso in cui si voglia uscire dal gioco prima della fine
    attroff(COLOR_PAIR(2));
    //  refresh();
    char c;
    do {
        c = getch();
        cbreak();
    } while (c != '\n');
}

void initCars(entity_s * cars) {																	//inizializzazione macchine

    int street[5] = { S_HEIGHT - 6, S_HEIGHT - 8, S_HEIGHT - 10, S_HEIGHT - 12, S_HEIGHT - 14};     //altezza delle corsie sulle quali scorrono le macchine
    int dir[5] = { -1, 1, -1, 1, -1};																//direzione macchine (-1 = da destra a sinistra, 1 = da sinistra a destraa)

    for (int i = 0; i < CAR_N; i++) {
        int lane = rnd(4, 0);
        cars[i].health = 99;
        cars[i].pos.x = rnd(S_WIDTH - CAR_WIDTH + 1, 0 + CAR_WIDTH); 								//distanza dal bordo
        cars[i].pos.y = street[lane];																//le macchine scorrono lungo le corsie della strada
        cars[i].extra = dir[lane];
    }
}

void initLogs (entity_s * logs){
	int screenWidth = getmaxx(stdscr);
    int space_occupied = LOG_WIDTH * (LOG_N + 2 * LOG_N - 1) + 2;                                 	//dimensione tronchi
    int border = (screenWidth - space_occupied) / 2;
	int river[5] = { S_HEIGHT - 18, S_HEIGHT - 20, S_HEIGHT - 22, S_HEIGHT - 24, S_HEIGHT - 26};	//altezza delle corsie sulle quali scorrono i tronchi
	int dir[5] = { 1, -1, 1, -1, 1};																//direzione tronchi (-1 = da destra a sinistra, 1 = da sinistra a destraa)

	for (int i = 0; i < LOG_N; i++) {
        logs[i].health = 99;
        logs[i].pos.x = rnd(S_WIDTH - (LOG_WIDTH + 1), LOG_WIDTH + 1); 								//distanza dal bordo
        logs[i].pos.y = river[i];																	//i tronchi scorrono lungo le "corsie" del fiume							
		logs[i].extra = dir[i];		
        logs[i].has_enemy = false;
    }
}

void printPlayer(entity_s player) {																	//stampa della rana
    int startX = player.pos.x, startY = player.pos.y;
    for (int i = 0; i < 2; i++) {
        attron(COLOR_PAIR(6));
        mvaddch(startY, startX + i, 'O');
        mvaddch(startY + 1, startX + i, 'O');
        attroff(COLOR_PAIR(6));
    }
    refresh();
}

bool checkPlayerLimits(int x, int y) {
    if (x <= S_WIDTH - 2 && x >= 0 && y >= S_HEIGHT - 26 && y <= S_HEIGHT - 4) { 					// limiti entro i quali la rana puo' muoversi
        return true;
    } else if (y <= S_HEIGHT - 26 && y >= S_WIDTH -38) { 											// limite superiore se mi trovo all'interno delle tane
        if ((x >= 2 && x < 6) || (x >= 10 && x < 14) || (x >= 18 && x < 22) || (x >= 26 && x < 30) || (x >= 34 && x < 38)) {
            return true;
        }
    }
    return false;
}


void printCars(entity_s cars) {																		//stampa delle macchine

    int startX = cars.pos.x, startY = cars.pos.y;
    
    if(startX < 0 - CAR_WIDTH + 2 || startX + CAR_WIDTH >= S_WIDTH || startY < S_HEIGHT - 14 || startY + 2 > S_HEIGHT)
        return;
    
    if(cars.extra == 0){
        attron(COLOR_PAIR(2));
        for (int i = 0; i < CAR_WIDTH; i++) {
            mvaddch(startY, startX + i, 'O');
            mvaddch(startY + 1, startX + i, 'O');
        }
        attroff(COLOR_PAIR(2));
        refresh();
    } else {
        attron(COLOR_PAIR(1));
        for (int i = 0; i < CAR_WIDTH; i++) {
            mvaddch(startY, startX + i, 'O');
            mvaddch(startY + 1, startX + i, 'O');
        }
        attroff(COLOR_PAIR(1));
        refresh();
    }
    
}

void printLog(entity_s log) {																		//stampa dei tronchi
    
    int startX = log.pos.x, startY = log.pos.y;
    
   	if(startX < 0 || startX + LOG_WIDTH >= S_WIDTH || startY < 0 || startY + 2 > S_HEIGHT)
        return;
    
    if(log.has_enemy){
        attron(COLOR_PAIR(7));
        for (int i = 0; i < LOG_WIDTH; i++) {
            mvaddch(startY, startX + i, 'O');
            mvaddch(startY + 1, startX + i, 'O');
        }
        attroff(COLOR_PAIR(7));
        attron(COLOR_PAIR(2));
        for (int i = 0; i < 2; i++){
            mvaddch(startY, startX + 2 + i, '@');
            mvaddch(startY + 1, startX + 2 + i, '-');
        }
        attroff(COLOR_PAIR(2));
    } else {
        attron(COLOR_PAIR(7));
        for (int i = 0; i < LOG_WIDTH; i++) {
            mvaddch(startY, startX + i, 'O');
            mvaddch(startY + 1, startX + i, 'O');
        }
        attroff(COLOR_PAIR(7));
    }
    refresh();
}


entity_s bullet(position_s position) {
    entity_s temp;
    temp.pos = position;
    temp.health = 100;
    temp.pos.y -= 1;

    return temp;
}

bool checkStreetCollision(entity_s cars[], entity_s player) {										//verifico se e' avvenuta una collisione tra una macchina e la rana
    for (int i = 0; i < CAR_N; i++){
        for(int j = 0; j < CAR_WIDTH; j++){
            if((cars[i].pos.x + j == player.pos.x || cars[i].pos.x + j == player.pos.x + 1) && (cars[i].pos.y + 1 == player.pos.y || cars[i].pos.y + 1 == player.pos.y + 1) )
                return true;
        }
    }
    return false;
}

bool checkRiverCollision(entity_s player) {															//verifico se la rana si trova sul fiume
    if( player.pos.y == S_HEIGHT - 18 || player.pos.y == S_HEIGHT - 20 || player.pos.y == S_HEIGHT - 22 || player.pos.y == S_HEIGHT - 24 || player.pos.y == S_HEIGHT - 26){
        return true;
    }
    return false;
}


bool checkNearby(bool right, entity_s current, entity_s cars[]) {
    //direction == true = dx, else sx
    int whereCheck = 0;
    int i = 0;

    if (right)
        whereCheck = CAR_WIDTH;
    else
        whereCheck = S_WIDTH - 5;
    
    //navi di primo tipo contro navi di primo tipo
    for (i = 0; i < CAR_N; i++)
        if (cars[i].pos.x == current.pos.x + whereCheck)
            return true;
    
    return false;
}

bool checkBulletsCollisionWithLogs(entity_s log, entity_s * bullets, int bulletCount) {				//funzione per la collisione tra i proiettili e i nemici(che stanno sui tronchi)
    
    // controllo se c'è un nemico su questo tronco
    if (log.has_enemy) {
        // controlla per ogni proiettile
        for (int j = 0; j < bulletCount; j++) {
            // controlla se il proiettile colpisce il nemico
            if ((bullets[j].pos.y == log.pos.y || bullets[j].pos.y == log.pos.y + 1) && bullets[j].pos.x >= log.pos.x + 2 && bullets[j].pos.x <= log.pos.x + LOG_WIDTH - 2) {
                bullets[j].health = 0;
                return true;
            }
        }
    }
    
	return false;
}

bool checkBulletsCollisionWithCars(entity_s car, entity_s * bullets, int bulletCount) {				// funzione per la collisione tra i proiettile e le macchine

    // controlla per ogni proiettile
    for (int j = 0; j < bulletCount; j++) {
        // controlla se il proiettile colpisce il nemico
        if ((bullets[j].pos.y == car.pos.y || bullets[j].pos.y == car.pos.y + 1) && range(car.pos.x, car.pos.x + CAR_WIDTH, bullets[j].pos.x)) {
            bullets[j].health = 0;
            return true;
        }
    }
    
    return false;
}

bool checkBulletsCollisionWithFrog(entity_s player, entity_s * enemyBullets, int enemyBulletCount) {	//funzione per la collisione tra i proiettili e la rana

    // controlla per ogni proiettile
    for (int j = 0; j < enemyBulletCount; j++) {
        // controlla se il proiettile colpisce il nemico
        if ((enemyBullets[j].pos.y == player.pos.y || enemyBullets[j].pos.y == player.pos.y + 1) && range(player.pos.x, player.pos.x + 1, enemyBullets[j].pos.x)) {
            enemyBullets[j].health = 0;
            return true;
        }
    }
    
    return false;
}

bool checkBulletsCollisionWithBullets(entity_s * bullets, int bulletCount, entity_s * enemyBullets, int enemyBulletCount) {	//funzione per la collisione tra proiettili della rana e proiettili nemici

    // controlla per ogni proiettile
    for (int j = 0; j < enemyBulletCount; j++) {
        for (int i = 0; i < bulletCount; i++){
            // controlla se i proiettili collidono tra di loro
            if (enemyBullets[j].pos.y == enemyBullets[i].pos.y && enemyBullets[j].pos.x == enemyBullets[i].pos.x) {
                enemyBullets[j].health = 0;
                return true;
            }
        }
    }
    
    return false;
}

//disegno la scena di gioco
void drawScene(int points, int countdown, int hp, entity_s player, entity_s cars[], entity_s logs[], entity_s * bullets, int bulletCount, entity_s * enemyBullets, int enemyBulletCount, bool lairs[]) {
    
	init_color(COLOR_MYGREEN, 0, 400, 0);
	clearZone(S_WIDTH, S_HEIGHT - 1);  //cancella schermata

    //il punteggio
	attron(COLOR_PAIR(0));
    mvprintw(S_HEIGHT - 2, 0, "PUNTI: %d", points);
    attroff(COLOR_PAIR(0));

    //tempo
     // Calcola la percentuale del tempo rimanente
    int percent = (countdown * 100) / 1000;

     // Disegna la barra
     // Imposta il colore di sfondo per la parte piena della barra
    for (int i = 0; i < S_WIDTH; i++) {
        if (i < (percent * S_WIDTH / 100)) {
            attron(COLOR_PAIR(6));
            mvprintw(S_HEIGHT - 1, 0+ i, "|");
            attroff(COLOR_PAIR(6)); // Ripristina il colore precedente
        } else {
            mvprintw(S_HEIGHT - 1, 0 + i, " ");
        }
    }
	
	//vita
	attron(COLOR_PAIR(0));
	mvprintw(S_HEIGHT - 2, 35, "HP: %d", hp);
	attroff(COLOR_PAIR(0)); 
    
    // marciapiede
    attron(COLOR_PAIR(4));
    for (int i = 0; i < S_WIDTH; i++){
        mvprintw(S_HEIGHT - 3, i, "O");
        mvprintw(S_HEIGHT - 4, i, "O");
        
        mvprintw(S_HEIGHT - 15, i, "O");
        mvprintw(S_HEIGHT - 16, i, "O");
    }
    attroff(COLOR_PAIR(4));

	//strada
	attron(COLOR_PAIR(0));
	for (int i = 0; i < S_WIDTH; i++){
		if(i % 3){
			mvprintw(S_HEIGHT - 7, i, "_");
			mvprintw(S_HEIGHT - 9, i, "_");
			mvprintw(S_HEIGHT - 11, i, "_");
			mvprintw(S_HEIGHT - 13, i, "_");
		}
	}
	

	//fiume
	attron(COLOR_PAIR(3));
	for (int i = 0; i < S_WIDTH; i++){
		mvprintw(S_HEIGHT - 26, i, " ");
        mvprintw(S_HEIGHT - 25, i, " ");
		mvprintw(S_HEIGHT - 24, i, " ");
        mvprintw(S_HEIGHT - 23, i, " ");
		mvprintw(S_HEIGHT - 22, i, " ");
        mvprintw(S_HEIGHT - 21, i, " ");
		mvprintw(S_HEIGHT - 20, i, " ");
        mvprintw(S_HEIGHT - 19, i, " ");
		mvprintw(S_HEIGHT - 18, i, " ");
        mvprintw(S_HEIGHT - 17, i, " ");
	}
	attroff(COLOR_PAIR(3));
	
    
    // tane
    // riga 30 e 29
    attron(COLOR_PAIR(5));
    for (int i = 0; i < S_WIDTH; i++) {
        mvprintw(S_HEIGHT - 30, i, "O");
		mvprintw(S_HEIGHT - 29, i, "O");
    }
    attroff(COLOR_PAIR(5));
    
    // riga 28-27
    for (int row = 0; row < 2; row++) {								//disegna il prato tranne nella posizione in cui si trova la tana
        for (int col = 0; col < 40; col++) {
            if (col < 2 || col > 37 ) {
                attron(COLOR_PAIR(5));
                mvprintw(S_HEIGHT - 28 + row, col, "O");
                attroff(COLOR_PAIR(5));
            } else if (col >= 2 & col < 6) {
                lairs[0] ? attron(COLOR_PAIR(5)) : attron(COLOR_PAIR(3));
                mvprintw(S_HEIGHT - 28 + row, col, " ");
                lairs[0] ? attroff(COLOR_PAIR(5)) : attroff(COLOR_PAIR(3));
            } else if (col > 5 & col < 10) {
                attron(COLOR_PAIR(5));
                mvprintw(S_HEIGHT - 28 + row, col, "O");
                attroff(COLOR_PAIR(5));
            } else if (col > 9 & col < 14){
                lairs[1] ? attron(COLOR_PAIR(5)) : attron(COLOR_PAIR(3));
                mvprintw(S_HEIGHT - 28 + row, col, " ");
                lairs[1] ? attroff(COLOR_PAIR(5)) : attroff(COLOR_PAIR(3));
            } else if ( col > 13 & col < 18){
                attron(COLOR_PAIR(5));
                mvprintw(S_HEIGHT - 28 + row, col, "O");
                attroff(COLOR_PAIR(5));
            } else if ( col > 17 & col < 22){
                lairs[2] ? attron(COLOR_PAIR(5)) : attron(COLOR_PAIR(3));
                mvprintw(S_HEIGHT - 28 + row, col, " ");
                lairs[2] ? attroff(COLOR_PAIR(5)) : attroff(COLOR_PAIR(3));
            } else if ( col > 21 & col < 26){
                attron(COLOR_PAIR(5));
                mvprintw(S_HEIGHT - 28 + row, col, "O");
                attroff(COLOR_PAIR(5));
            } else if ( col > 25 & col < 30){
                lairs[3] ? attron(COLOR_PAIR(5)) : attron(COLOR_PAIR(3));
                mvprintw(S_HEIGHT - 28 + row, col, " ");
                lairs[3] ? attroff(COLOR_PAIR(5)) : attroff(COLOR_PAIR(3));
            } else if ( col > 29 & col < 34){
                attron(COLOR_PAIR(5));
                mvprintw(S_HEIGHT - 28 + row, col, "O");
                attroff(COLOR_PAIR(5));
            } else {
                lairs[4] ? attron(COLOR_PAIR(5)) : attron(COLOR_PAIR(3));
                mvprintw(S_HEIGHT - 28 + row, col, " ");
                lairs[4] ? attroff(COLOR_PAIR(5)) : attroff(COLOR_PAIR(3));
            }
            
        }
    }
    attroff(COLOR_PAIR(5));

	//tronchi
	for (int i = 0; i < LOG_N; i++) {
        printLog(logs[i]);
    }

	//stampo il giocatore
    printPlayer(player);

	//le macchine
    for (int i = 0; i < CAR_N; i++) {
        printCars(cars[i]);
	}
    
    //i proiettili nemici, per il colore fare riferimento alla funzione defineColors()
    attron(COLOR_PAIR(2));
    for (int i = 0; i < enemyBulletCount; i++) {
        if (enemyBullets[i].health > 0)
            mvaddch(enemyBullets[i].pos.y, enemyBullets[i].pos.x, 'v');
        refresh();
    }
    attroff(COLOR_PAIR(2));

    
    //i proiettili, per il colore fare riferimento alla funzione defineColors()
    attron(COLOR_PAIR(6));
    for (int i = 0; i < bulletCount; i++) {
        if (bullets[i].health > 0)
            mvaddch(bullets[i].pos.y, bullets[i].pos.x, '^');
        refresh();
    }
    attroff(COLOR_PAIR(6));

    //aggiorno il tutto
    refresh();
}

int rnd(int MAX, int MIN) {
    return MIN + rand() % (MAX - MIN + 1);
}

void defineColors() {
    //definisce le coppie di colori
    if (has_colors()) {
        init_pair(0, COLOR_WHITE, COLOR_BLUE);		//info, strada
        init_pair(1, COLOR_WHITE, COLOR_RED);		//car
    	init_pair(2, COLOR_RED, COLOR_WHITE);		//car
        init_pair(3, COLOR_WHITE, COLOR_BLUE);	 	//fiume
        init_pair(4, COLOR_CYAN, COLOR_MAGENTA);	//marciapiede
        init_pair(5, COLOR_MAGENTA, COLOR_GREEN);	//tane
        init_pair(6, COLOR_GREEN, COLOR_MYGREEN);	//player  
    	init_pair(7, COLOR_GREEN, COLOR_YELLOW);	//trochi	
    }
}
