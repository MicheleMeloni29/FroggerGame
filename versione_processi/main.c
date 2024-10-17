#include "libs.h"

//prototipi dei processi
void mainProcess(int);
void playerProcess(int pipe);
void directionGeneratorProcess(int pipe, int type);

int main(){
    int pIDS[PROCESS_N], p[2];
    srand((unsigned int) time(NULL));
	
    initScreen();

    pipe(p);

    mvprintw(S_HEIGHT/2, S_WIDTH/2 - 20, "Premi un tasto qualsiasi per iniziare...");
    getch();
    cbreak();

    /*
    * Generiamo i vari processi
    * Processo Principale = main (contenente il loop principale)
    * Processo player = acquisisce tutte le informazioni da tastiera e stabilisce comportamento player
    * Processo CAR = genera la direzione delle auto che passano nella strada
	* Processo LOG = genera i tronchi che passano nel fiume
    */

    pIDS[0] = fork();
    if(!pIDS[0])
        playerProcess(p[1]);
    else {
        pIDS[1] = fork();
        if(!pIDS[1])
            directionGeneratorProcess(p[1], CAR); //pipe e entità generata CAR
        else {
			pIDS[2] = fork();
			if(!pIDS[2])
				directionGeneratorProcess(p[1], LOG);  //pipe e entità generata LOG
			else {
            mainProcess(p[0]);
			}
		}
    }
    
    kill(pIDS[0], 1);
    kill(pIDS[1], 1);
    kill(pIDS[2], 1);
    
    endwin();
    printf("\nPartita terminata\n");
    return 0;
}

void mainProcess(int pipe) {

    /*
    * Struttura del programma :
    * Il processo principale (questo) legge dal buffer il dato di tipo entità
    * a seconda del tipo viene fatto un controllo differente
    * Le informazioni della tastiera vengono messe nel dato di tipo player
    */

    //variabile generiche
    int random;
    char *endGame = "";     															//il messaggio stampato a fine partita
    bool cycle = true;      															//usata per capire se rimanere nel loop
    int points = 0;         															//punteggio del giocatore
	int countdown = COUNTDOWN;  														//countdown di 30 secondi
	int hp = 5;																			//vita 
    int street[5] = { S_HEIGHT - 6, S_HEIGHT - 8, S_HEIGHT - 10, S_HEIGHT - 12, S_HEIGHT - 14};	//corsie dove scorrono le auto
	time_t start_time = time(NULL);
    bool lairs[5] = { false, false, false, false, false };								//tane
    int dir[5] = { -1, 1, -1, 1, -1};													//direzioni auto
    int lane;

    //dati
    entity_s player = initEntity(PLAYER, -1,S_WIDTH/2-2, S_HEIGHT-BOTTOM_PADDING, 100); //giocatore (e inizializzazione)
	entity_s cars[CAR_N]; 																//veicoli
	initCars (cars);            														//inizializzazione lista veicoli
	entity_s logs[LOG_N];       														//tronchi
	initLogs (logs);																	//inizializzazione lista tronchi

    entity_s *bullets =  NULL; 															//array dinamico contenente i proiettili sparati dal player
    entity_s *enemyBullets =  NULL; 													//array dinamico contenente i proiettili sparati dal player
    int bulletCount = 0;
    int enemyBulletCount = 0;

    //ciclo principale
    do {

        entity_s entity = initEmpty(); 													//la variabile dove vengono salvati i dati letti dal buffer, viene pulita a ogni ciclo

        read(pipe, &entity, sizeof(entity_s)); 											//lettura dei dati

        /*
        * A seconda del tipo di entità letto aggiorno la posizione di conseguenza
        * controllando che la nuova posizione sia valida
        * in caso sia il player analizziamo l'informazione extra che contiene
        * il tasto premuto dalla tastiera
        */
        
        switch(entity.type) {
            case PLAYER: {
                //posizione
				if (checkPlayerLimits(player.pos.x + entity.pos.x, player.pos.y + entity.pos.y)) {
					player.pos.x += entity.pos.x;
					player.pos.y += entity.pos.y;
				}
                //gestione tastiera
                switch(entity.extra) {
                //creo un proiettile
                    case ' ': {
                        if(bullets == NULL)
                            bullets = (entity_s*)malloc(sizeof(entity_s));
                        bulletCount+=1;
                        //faccio spazio per due nuovi proiettili
                        bullets = (entity_s*)realloc(bullets, bulletCount*sizeof(entity_s));
                        bullets[bulletCount-1] = bullet(player.pos);
						break;
                    }
                    case '': //ESC
                    case 'q': {
                        //se il giocatore vuole smettere di giocare
                        cycle = false;
                        endGame = "Hai abbandonato la partita";
                        break;
                    }
                    default: {
                        break;
                    }
                }
                break;
            }
            case CAR: {
                for(int i = 0; i < CAR_N; i++){
                    int posX = cars[i].pos.x;
                    if(posX + entity.pos.x > 0 - CAR_WIDTH - 2 && posX + entity.pos.x <= S_WIDTH + CAR_WIDTH + 2) {
                        if(cars[i].extra == 1){
                            cars[i].pos.x += entity.pos.x;
                        } else {
                            cars[i].pos.x -= entity.pos.x;
                        }
                    }
                    
                    if(cars[i].extra == 1){
                        if(cars[i].pos.x == S_WIDTH){
                            lane = rnd(5, 0);
                            cars[i].pos.y = street[lane];
                            cars[i].extra = dir[lane];
                            if(cars[i].extra == 1){
                                 cars[i].pos.x = 0 - CAR_WIDTH;
                                while(checkNearby(true, cars[i], cars))
                                    cars[i].pos.x -= 1;
                            } else {
                                cars[i].pos.x = S_WIDTH + CAR_WIDTH;
                                while(checkNearby(false, cars[i], cars))
                                    cars[i].pos.x += 1;
                            }
                        }
                    } else {
                        if(cars[i].pos.x + CAR_WIDTH - 2 == 0){
                            lane = rnd(5, 0);
                            cars[i].pos.y = street[lane];
                            cars[i].extra = dir[lane];
                            if(cars[i].extra == 1){
                                cars[i].pos.x = 0 - CAR_WIDTH;
                                while(checkNearby(true, cars[i], cars))
                                    cars[i].pos.x -= 1;
                            } else {
                                cars[i].pos.x = S_WIDTH + CAR_WIDTH;
                                while(checkNearby(false, cars[i], cars))
                                    cars[i].pos.x += 1;
                            }
                        }
                    }
                }
                break;
            }
			case LOG: {
                for(int i = 0; i < LOG_N; i++){
					int posX = logs[i].pos.x; 								// la posizione attuale del tronco, in una variabile per comodità
					// controllo che la posizione del tronco sia valida
					switch(logs[i].extra){
						case -1: {
							logs[i].pos.x += entity.pos.x;
							break;
						}
						case 1: {
							logs[i].pos.x -= entity.pos.x;
							break;
						}
					}

					if(logs[i].pos.x == 1){
						logs[i].extra *= -1;
                        // controllo se il player è sopra il tronco
                        if(player.pos.y == logs[i].pos.y && player.pos.x >= logs[i].pos.x && player.pos.x < logs[i].pos.x + LOG_WIDTH) {
                            if(logs[i].extra == 1)
                                player.pos.x += 2;
                            else
                                player.pos.x -= 2;							// muovo il player insieme al tronco
                        }
					}

					if(logs[i].pos.x + LOG_WIDTH - 1 == S_WIDTH - 2){
						logs[i].extra *= -1;
                        // controllo se il player è sopra il tronco
                        if(player.pos.y == logs[i].pos.y && player.pos.x >= logs[i].pos.x && player.pos.x < logs[i].pos.x + LOG_WIDTH) {
                            if(logs[i].extra == 1)
                                player.pos.x += 2;
                            else
                                player.pos.x -= 2;							// muovo il player insieme al tronco
                        }
					}
                     
                     // controllo se il player è sopra il tronco
                     if(player.pos.y == logs[i].pos.y && player.pos.x >= logs[i].pos.x && player.pos.x < logs[i].pos.x + LOG_WIDTH) {						
                         if(logs[i].extra == 1)
                             player.pos.x -= 1;
                         else
                             player.pos.x += 1;								// muovo il player insieme al tronco
                     }
                     
                     // aspetta un intervallo di tempo casuale tra 1 e 5 secondi
                     int waitTime = rand() % 100 + 1; // tempo
                     
                     // sceglie casualmente un tronco e imposta la variabile has_enemy a true
                     if(waitTime == 10){
                         int enemyLogIndex = rand() % LOG_N;
                         if(player.pos.y == logs[enemyLogIndex].pos.y && player.pos.x >= logs[enemyLogIndex].pos.x && player.pos.x < logs[enemyLogIndex].pos.x + LOG_WIDTH)
                             logs[enemyLogIndex].has_enemy = false;
                         else
                             logs[enemyLogIndex].has_enemy = true;
                     }
                     
                     //spari nemici
                     if(logs[i].has_enemy){
                         // aspetta un intervallo di tem
                         int frequency = rand() % 60 + 1; // tempo
                         
                         // sceglie casualmente un tronco e imposta la variabile has_enemy a true
                         if(frequency == 10){
                             if(enemyBullets == NULL)
                                 enemyBullets = (entity_s*)malloc(sizeof(entity_s));
                             enemyBulletCount+=1;
                             //faccio spazio per un nuovo proiettile
                             enemyBullets = (entity_s*)realloc(enemyBullets, enemyBulletCount*sizeof(entity_s));
                             enemyBullets[enemyBulletCount-1] = bullet(logs[i].pos);
                         }
                     }
				}

                if(checkRiverCollision(player)){
                    // controllo se il player non è più sopra il tronco
                    bool onLog = false;
                    for(int i = 0; i < LOG_N; i++){
                        if(player.pos.y == logs[i].pos.y && player.pos.x >= logs[i].pos.x && player.pos.x - 1 < logs[i].pos.x + LOG_WIDTH) {
                            onLog = true;
							if(logs[i].has_enemy){
								hp--;
								countdown = 1000;
								player.pos.x = S_WIDTH/2-2;
								player.pos.y = S_HEIGHT-BOTTOM_PADDING;
								flash();
								flash();
								attron(COLOR_PAIR(2));
								mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "Ti ha ucciso un nemico!");
								attroff(COLOR_PAIR(2));
								getch();
								cbreak();
							}
                            break;
                        }
                     }
	
                     if(!onLog){
                         hp--;
                         countdown = 1000;
                         player.pos.x = S_WIDTH/2-2;
                         player.pos.y = S_HEIGHT-BOTTOM_PADDING;
                         attron(COLOR_PAIR(2));
                         mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10 , "Sei caduto in acqua!");
                         attroff(COLOR_PAIR(2));
                         getch();
                         cbreak();
                     }
                }
                
				break;
				
				
			}
        }

        /*
        * Aggiornamento della posizione dei proiettili
        * e controllo delle collisioni nemico-proiettile.
        * In assenza di una funzione tipo "trim" per rimuovere
        * i proiettili "morti" dalla lista riduciamo la loro vita a 0 e non verrano più considerati
        */

        if(bullets != NULL) {
            //per ogni proiettile in lista
            for(int i = 0; i<bulletCount; i++) {
                int y = bullets[i].pos.y;
                int x = bullets[i].pos.x;
                //se il proiettile è fuori dallo schermo la vita va 0
                if( y < 0  || x < 0 || x > S_WIDTH)
                    bullets[i].health = 0;
                //altrimenti
                else {
                    //aggiorno la posizione
                    bullets[i].pos.y--;
                }
            }
        }
        
        if(enemyBullets != NULL) {
            //per ogni proiettile in lista
            for(int i = 0; i<enemyBulletCount; i++) {
                int y = enemyBullets[i].pos.y;
                int x = enemyBullets[i].pos.x;
                //se il proiettile è fuori dallo schermo la vita va 0
                if( y < 0  || x < 0 || x > S_WIDTH || y > S_HEIGHT - 2)
                    enemyBullets[i].health = 0;
                //altrimenti
                else {
                    //aggiorno la posizione
                    enemyBullets[i].pos.y++;
                }
            }
        }

		//se il timer arriva a 0, il gioco si riavvia con una vita in meno
	 	if (countdown > 0) {
		    time_t current_time = countdown--;
		    if (current_time - start_time >= 1000) {
		        start_time = current_time;
		        countdown--;
		    }
		}
        
        //disegno la scena
        drawScene(points, countdown, hp, player, cars, logs, bullets, bulletCount, enemyBullets, enemyBulletCount, lairs);
        
        for(int i = 0; i < CAR_N; i++){
            checkBulletsCollisionWithCars(cars[i], bullets, bulletCount);						//controllo collisioni proiettili-macchine
            checkBulletsCollisionWithCars(cars[i], enemyBullets, enemyBulletCount);
        }
         
        checkBulletsCollisionWithBullets(bullets, bulletCount, enemyBullets, enemyBulletCount);	//controllo collisioni priettili rana-proiettili nemici
        
        for(int i = 0; i < LOG_N; i++){
            if (checkBulletsCollisionWithLogs(logs[i], bullets, bulletCount)){					//collisione proiettile-nemico
                flash();
                points += 100;
                countdown += 50;
                logs[i].has_enemy = false;
            }
        }
        
        if(checkBulletsCollisionWithFrog(player, enemyBullets, enemyBulletCount)){				//collisione proiettile-rana
            hp--;
            countdown = 1000;
            player.pos.x = S_WIDTH/2-2;
            player.pos.y = S_HEIGHT-BOTTOM_PADDING;
            flash();
            flash();
            attron(COLOR_PAIR(2));
            mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "Ti hanno sparato!");
            attroff(COLOR_PAIR(2));
            getch();
            cbreak();
        }
        
        if(checkStreetCollision(cars, player)){													//collisione macchine-rana
            hp--;
            countdown = 1000;
            player.pos.x = S_WIDTH/2-2;
            player.pos.y = S_HEIGHT-BOTTOM_PADDING;
            flash();
            flash();
            attron(COLOR_PAIR(2));
            mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "Ti hanno investito!");
            attroff(COLOR_PAIR(2));
            getch();
            cbreak();
        }
        
        if (countdown == 800)																	//man mano che il tempo scorre, viene decrementato il punteggio
            points -= 50;
        if (countdown == 600)
            points -= 50;
        if (countdown == 400)
            points -= 50;
        if (countdown == 200)
            points -= 50;
        
        if (countdown == 0) {																	//se scade il tepo termina la manche e viene decrementata la vita
            hp--;
            countdown = 1000;
            player.pos.x = S_WIDTH/2-2;
            player.pos.y = S_HEIGHT-BOTTOM_PADDING;
            flash();
            flash();
            attron(COLOR_PAIR(2));
            mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "Hai esaurito il tempo!");
            attroff(COLOR_PAIR(2));
            getch();
            cbreak();
        }
        
        for(int i = 0; i < S_WIDTH; i++ ){        											//controlli per verificare se la tana e' libera, se libera la chiudiamo, altrimenti perdiamo una vta.
            if(player.pos.y > 1 & player.pos.y < 4){
                if ( player.pos.x >= 2 & player.pos.x < 6 ) {
                    if(lairs[0] == true){
                        hp--;
                        countdown = 1000;
                        player.pos.x = S_WIDTH/2-2;
                        player.pos.y = S_HEIGHT-BOTTOM_PADDING;
                        flash();
                        flash();
                        attron(COLOR_PAIR(2));
                        mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "La tana era chiusa!!");
                        attroff(COLOR_PAIR(2));
                        getch();
                        cbreak();
                    } else {
                        points += 500;
                        countdown += 500;
                        lairs[0] = true;
                        player.pos.x = S_WIDTH/2-2;
                        player.pos.y = S_HEIGHT-BOTTOM_PADDING;
                        flash();
                        flash();
                        attron(COLOR_PAIR(2));
                        mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "Tana chiusa!");
                        attroff(COLOR_PAIR(2));
                        getch();
                        cbreak();
                    }
                } else if ( player.pos.x > 9 & player.pos.x < 14 ) {
                    if(lairs[1] == true){
                        hp--;
                        countdown = 1000;
                        player.pos.x = S_WIDTH/2-2;
                        player.pos.y = S_HEIGHT-BOTTOM_PADDING;
                        flash();
                        flash();
                        attron(COLOR_PAIR(2));
                        mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "La tana era chiusa!!");
                        attroff(COLOR_PAIR(2));
                        getch();
                        cbreak();
                    } else {
                        points += 500;
                        countdown += 500;
                        lairs[1] = true;
                        player.pos.x = S_WIDTH/2-2;
                        player.pos.y = S_HEIGHT-BOTTOM_PADDING;
                        flash();
                        flash();
                        attron(COLOR_PAIR(2));
                        mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "Tana chiusa!");
                        attroff(COLOR_PAIR(2));
                        getch();
                        cbreak();
                    }
                } else if ( player.pos.x > 17 & player.pos.x < 22 ) {
                    if(lairs[2] == true){
                        hp--;
                        countdown = 1000;
                        player.pos.x = S_WIDTH/2-2;
                        player.pos.y = S_HEIGHT-BOTTOM_PADDING;
                        flash();
                        flash();
                        attron(COLOR_PAIR(2));
                        mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "La tana era chiusa!!");
                        attroff(COLOR_PAIR(2));
                        getch();
                        cbreak();
                    } else {points += 500;
                        countdown += 500;
                        lairs[2] = true;
                        player.pos.x = S_WIDTH/2-2;
                        player.pos.y = S_HEIGHT-BOTTOM_PADDING;
                        attron(COLOR_PAIR(2));
                        mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "Tana chiusa!");
                        flash();
                        flash();
                        attroff(COLOR_PAIR(2));
                        getch();
                        cbreak();
                    }
                } else if ( player.pos.x > 25 & player.pos.x < 30 ) {
                    if(lairs[3] == true){
                        hp--;
                        countdown = 1000;
                        player.pos.x = S_WIDTH/2-2;
                        player.pos.y = S_HEIGHT-BOTTOM_PADDING;
                        flash();
                        flash();
                        attron(COLOR_PAIR(2));
                        mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "La tana era chiusa!!");
                        attroff(COLOR_PAIR(2));
                        getch();
                        cbreak();
                    } else {
                        points += 500;
                        countdown += 500;
                        lairs[3] = true;
                        player.pos.x = S_WIDTH/2-2;
                        player.pos.y = S_HEIGHT-BOTTOM_PADDING;
                        attron(COLOR_PAIR(2));
                        mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "Tana chiusa!");
                        flash();
                        flash();
                        attroff(COLOR_PAIR(2));
                        getch();
                        cbreak();
                    }
                } else if ( player.pos.x > 33 & player.pos.x < 38 ) {
                    if(lairs[4] == true){
                        hp--;
                        countdown = 1000;
                        player.pos.x = S_WIDTH/2-2;
                        player.pos.y = S_HEIGHT-BOTTOM_PADDING;
                        flash();
                        flash();
                        attron(COLOR_PAIR(2));
                        mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "La tana era chiusa!!");
                        attroff(COLOR_PAIR(2));
                        getch();
                        cbreak();
                    } else {
                        points += 500;
                        countdown += 500;
                        lairs[4] = true;
                        player.pos.x = S_WIDTH/2-2;
                        player.pos.y = S_HEIGHT-BOTTOM_PADDING;
                        attron(COLOR_PAIR(2));
                        mvprintw(S_HEIGHT/2 - 10, S_WIDTH/2 - 10, "Tana chiusa!");
                        flash();
                        flash();
                        attroff(COLOR_PAIR(2));
                        getch();
                        cbreak();
                    }
                }
            }
        }
        
        if(hp < 0){
            endGame = "Hai esaurito le vite.";												//la partita finisce quando si esauriscono tutte le vite, in questo caso abbiamo perso
            break;
        }
        
        if (lairs[0] & lairs[1] & lairs[2] & lairs[3] & lairs[4]){							//per vincere si devono conquistare tutte e 5 le tane
            endGame = "Hai vinto!";
            break;
        }
        
    } while(cycle);

    printEnd(endGame);
    return;
}

void playerProcess(int pipe) {

    entity_s player;

    while(true){
        player.pos.x = 0;
        player.pos.y = 0;
        player.type = PLAYER;
        player.extra = -1;
		nodelay(stdscr, TRUE); //abilita l'input non bloccante
        char c = getch();
		cbreak();
        
		switch(c) {
            //movimento a sinistra
            case KEY_LEFT&A_CHARTEXT:
            case 'a':
            case 'A': {
                player.pos.x = -2;
                break;
            }
            //movimento a destra
            case KEY_RIGHT&A_CHARTEXT:
            case 'd':
            case 'D': {
                player.pos.x = +2;
                break;
            }
			//movimento verso il basso
            case KEY_UP&A_CHARTEXT:
            case 'w':
            case 'W': {
                player.pos.y = -2;
                break;
            }
			//movimento verso l'alto
            case KEY_DOWN&A_CHARTEXT:
            case 's':
            case 'S': {
                player.pos.y = +2;
                break;
            }
            default: {
                player.extra = c;
            }
        };

        write(pipe, &player, sizeof(entity_s));

        usleep(DELAY);
    };
}
                
void directionGeneratorProcess(int pipe, int type) {
    while (true) {
        entity_s enemy;
        enemy.pos.x = 0;
        enemy.pos.y = 0;
        enemy.type = type;
        enemy.health = 100;
        
        enemy.pos.x = 1;
        
        usleep(DELAY);
    
        write(pipe, & enemy, sizeof(entity_s));
    }
}
