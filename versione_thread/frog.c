#include "libs.h"

/* Variabili globali di comunicazione tra i thread */

// Giocatore
volatile pos pos_frog;        				// Posizione corrente della rana
volatile pos pos_bullet;        				// Posizione corrente del proiettile
int bullet_count = 0;								// Proiettili sparati
int bullet_hit = FALSE;								// Flag per l'hit del proiettile
int life = 5;										// Vita
int points = 0;										// Punti
int countdown = 1500;								// Tempo

// Macchine e bus
volatile pos pos_cars[CAR_NUMBER]; 			// Posizione corrente delle macchine
volatile pos pos_bus[CAR_NUMBER];       		// Posizione corrente dei bus
int street_y[CAR_NUMBER] = {MAXY - 6, MAXY - 8, MAXY - 10, MAXY - 12, MAXY - 14}; // Corsie della strada
bool car_dir[CAR_NUMBER] = {true, false, true, false, true};					  // Direzione delle corsie

// Tronchi 
pos pos_logs[LOGS_NUMBER];       			// Posizione corrente dei tronchi
volatile pos pos_enemy[LOGS_NUMBER];			// Posizione corrente dei nemici
volatile pos pos_enemy_bullet[LOGS_NUMBER];  // Posizione corrente dei proiettili nemici
volatile bool log_occupied[LOGS_NUMBER] = { false }; 		 // Array per controllare se il tronco è già occupato 
volatile bool log_occupied_by_frog[LOGS_NUMBER] = { false }; // Array per controllare se il tronco è già occupato dalla rana
volatile bool log_dir[LOGS_NUMBER];							 // Direzioni dei vari tronchi
int enemy_bullet_count = 0;							// Proiettili sparati
int enemy_bullet_hit = FALSE;						// Flag per l'hit del proiettile
int river_y[LOGS_NUMBER] = {MAXY - 18, MAXY - 20, MAXY - 22, MAXY - 24, MAXY - 26}; // Corsie del fiume

volatile int running;            					// Flag di programma non terminato

volatile int lairs[LAIRS_NUMBER];					// Flags per le tane: true = chiusa, false = aperta

restart_msg msg;								// Messaggio da stampare all'inizio della manche

/* Mutex e semafori per la sincronizzazione tra i thread */
pthread_mutex_t semCurses;    						// Semaforo per l'accesso alle funzioni di ncurses
pthread_mutex_t semBullet;    						// Semaforo per l'accesso alle funzioni del proiettile
pthread_mutex_t semEnemyBullet;    					// Semaforo per l'accesso alle funzioni del proiettile nemico
sem_t semGame;                						// Semaforo per sincronizzare il campo con le 'entità'

/* Prototipi dei thread */
void *frog (void *arg);								// Gestisce il movimento della rana					
void *car (void *arg);								// Gestisce il movimento delle auto nella strada
void *logs (void *arg);								// Gestisce il movimento dei tronchi nel fiume
void *bullet (void *arg);							// Gestisce i proiettili sparati dal player
void *enemy_bullet(void *arg);						// Gestisce i proiettili sparati dai nemici 
void map (void);									// Gestisce la stampa delle varie entità e lo svolgimento del gioco
void initGame();									// Inizializza le entità per la nuova manche

int main() {

	pthread_t tCar;        	// Tid del figlio 'auto'
	pthread_t tFrog;        // Tid del figlio 'rana'
	pthread_t tLog;         // Tid del figlio 'tronco'
    
    initscr();            	// Inizializza curses
    noecho();            	// caratteristica della tastiera
    curs_set(0);            // elimina il cursore
    start_color();
    defineColors();
    
    mvprintw(MAXY/2, MAXX/2 - 20, "Premi un tasto qualsiasi per iniziare...");
    getch();
    cbreak();

    running = TRUE;        	// Gioco non ancora finito

    pthread_mutex_init (&semCurses, NULL); 	// Mutex di ncurses
    sem_init (&semGame, 0, -1);    			// Semaforo del gioco, inizialmente chiuso

    /* Creo il thread dell'auto */
    if (pthread_create (&tCar, NULL, car, NULL))
    {
    endwin();        // Torniamo in modalita' normale
    printf ("Non riesco a creare il thread della macchina\n");
    exit;
    }

    /* Creo il thread della rana */
    if (pthread_create (&tFrog, NULL, frog, NULL))
    {
    endwin();        // Torniamo in modalita' normale
    printf ("Non riesco a creare il thread della rana\n");
    exit;
    }
    
    /*Creo il thread del tronco*/
    if (pthread_create (&tLog, NULL, logs, NULL))
    {
    endwin();        // Torniamo in modalita' normale
    printf ("Non riesco a creare il thread del tronco\n");
    exit;
    }

    map ();        // Svolgo il lavoro del padre (gestire il campo)

    /* Attendo la terminazione dei thread */
    pthread_join (tCar, NULL);
    pthread_join (tFrog, NULL);
    pthread_join (tLog, NULL);

    /* Elimino il mutex ed il semaforo */
    pthread_mutex_destroy (&semCurses);
    sem_destroy (&semGame);

    endwin();        // Torno in modalita' normale

    return 0;        // Termine del programma
}

void initGame(){

	srand(time(NULL));
	
	clearScreen();
	
	switch(msg) {
        case CRASH: {
            mvprintw(MAXY/2 - 5, MAXX/2 - 10, "Ti hanno investito!");
            break;
        }
        case SPLASH: {
            mvprintw(MAXY/2 - 5, MAXX/2 - 10, "Sei caduto in acqua!");
            break;
        }
        case ENEMY: {
            mvprintw(MAXY/2 - 5, MAXX/2 - 10, "Il nemico ti ha ucciso!");
            break;
        }
        case CLOSED: {
            mvprintw(MAXY/2 - 5, MAXX/2 - 10, "La tana era gia chiusa!");
            break;
        }
        case TIME: {
            mvprintw(MAXY/2 - 5, MAXX/2 - 10, "Il tempo è scaduto!");
            break;
        }
        case WIN: {
            mvprintw(MAXY/2 - 5, MAXX/2 - 10, "Tana chiusa! Chiudi le altre per vincere..");
            break;
        }
        default: {
            mvprintw(MAXY/2 - 5, MAXX/2 - 10, "Sei morto!");
            break;
        }
    }
	
	mvprintw(MAXY/2, MAXX/2 - 10, "Premi un tasto qualsiasi per iniziare...");
    getch();
    cbreak();
    
    // Inizializza la posizione, la direzione di ogni macchina e bus
    for (int i = 0; i < CAR_NUMBER; i++) {
        pos_cars[i].x = rand() % (MAXX - CAR_SIZE);  	// posizione casuale
        pos_cars[i].y = street_y[i];
        pos_bus[i].x = rand() % (MAXX - BUS_SIZE);  	// posizione casuale
        pos_bus[i].y = street_y[i];
    }
    
    // Inizializza la posizione, la direzione di ogni tronco
    for (int i = 0; i < LOGS_NUMBER; i++) {
        pos_logs[i].x = rand() % (MAXX - LOG_SIZE);  // posizione casuale
        pos_logs[i].y = river_y[i];  
        log_dir[i] = rand() % 2;  					 // direzione casuale
		log_occupied[i] = false;
    }
    
    // Posizione iniziale della rana: in basso al centro
  	pos_frog.x = MAXX / 2;
  	pos_frog.y = MAXY - 4;  
}

/* Funzione 'proiettile nemico'.
 * Una volta sparato si muove verso il basso fino fino a quando non si scontra
 * con un'altra entità oppure esce dallo schermo
 */
void *enemy_bullet(void *arg){

	pos enemy_p = *(pos*)arg;    			// Posizione del nemico che spara, ricevuta come parametro
	
    pos_enemy_bullet->x = enemy_p.x;		// Il proiettile parte dalla posizione x del nemico
    pos_enemy_bullet->y = enemy_p.y + 1;	// e nella posizione y + 1

    int enemy_bullet_hit = FALSE;			// Flag che tiene traccia dell'hit del proiettile
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    while (pos_enemy_bullet->y < MAXY - 4 && !enemy_bullet_hit) { // Loop finche il proiettile non esce oppure colpisce
        
    	pos_enemy_bullet->y++;   			// Il proiettile scende verso il basso
	
		int hit_vehicle = 0; 				// Flag per indicare se il proiettile ha colpito un veicolo
				
		for (int i = 0; i < CAR_NUMBER; i++) {	// Scorre per tutte le macchine e i bus
			
			// Controllo se colpisce le macchine
		    for (int j = 0; j < CAR_SIZE; j++){		    
		        if ((pos_cars[i].x + j == pos_enemy_bullet->x) && (pos_cars[i].y == pos_enemy_bullet->y)) {
		            pthread_mutex_lock(&semEnemyBullet);
		            pos_enemy_bullet->x = AWAY;
					pos_enemy_bullet->y = AWAY;
		            mvaddch(pos_enemy_bullet->y, pos_enemy_bullet->x, ' '); // Cancella la posizione del proiettile
		            enemy_bullet_count--;
		            pthread_mutex_unlock(&semEnemyBullet);
		            enemy_bullet_hit = TRUE;
		            hit_vehicle = 1; // Imposta il flag per indicare che il proiettile ha colpito un veicolo
		            break;
		        }		        
		    }
		    if (hit_vehicle) { // Esci dai loop for se il proiettile ha colpito un veicolo
		        enemy_bullet_count --;
		        break;
		    }
		    
		    // Controllo se colpisce i bus
		    for (int j = 0; j < BUS_SIZE; j++){
		        if ((pos_bus[i].x + j == pos_enemy_bullet->x) && (pos_bus[i].y == pos_enemy_bullet->y)) {
		            pthread_mutex_lock(&semEnemyBullet);
		            pos_enemy_bullet->x = AWAY;
					pos_enemy_bullet->y = AWAY;
		            mvaddch(pos_enemy_bullet->y, pos_enemy_bullet->x, ' '); // Cancella la posizione del proiettile
		            enemy_bullet_count--;
		            pthread_mutex_unlock(&semEnemyBullet);
		            enemy_bullet_hit = TRUE;
		            hit_vehicle = 1; // Imposta il flag per indicare che il proiettile ha colpito un veicolo
		            break;
		        }
		    }
		    if (hit_vehicle) { // Esci dai loop for se il proiettile ha colpito un veicolo
		        bullet_count --;
		        break;
		    }
		}
		
		// Controllo se il player viene colpito
        if ((pos_frog.x == pos_enemy_bullet->x || pos_frog.x + 1 == pos_enemy_bullet->x) && (pos_frog.y == pos_enemy_bullet->y || pos_frog.y + 1 == pos_enemy_bullet->y)) {
        	pthread_mutex_lock(&semEnemyBullet);
            pos_bullet.x = AWAY;
			pos_bullet.y = AWAY;
            mvaddch(pos_enemy_bullet->y, pos_enemy_bullet->x, ' '); // Cancella la posizione del proiettile
            enemy_bullet_count--;
            pthread_mutex_unlock(&semEnemyBullet);
            enemy_bullet_hit = TRUE;
            hit_vehicle = 1; // Imposta il flag per indicare che il proiettile ha colpito un veicolo
            break;
        }
		if (hit_vehicle) { // Esci dai loop for se il proiettile ha colpito un veicolo
	        bullet_count --;
	        break;		    
	    }
			
		if (hit_vehicle) { // Esci dal ciclo while se il proiettile ha colpito un veicolo
		    pthread_cancel(pthread_self());
		}
			
		usleep(DELAY);
		    
		sem_post(&semGame);  // avvisa il campo che c'e' stato un movimento
    
    }
    
	pthread_mutex_lock(&semEnemyBullet);
    enemy_bullet_count--; // Decrementa il numero di proiettili in volo
    pthread_mutex_unlock(&semEnemyBullet);

    pthread_exit(NULL);
}

/* Funzione 'proiettile'.
 * Una volta sparato si muove verso l'alto fino fino a quando non si scontra
 * con un'altra entità oppure esce dallo schermo
 */
void *bullet(void *arg) {

    pos_bullet.x = pos_frog.x;								// Il proiettile parte dalla posizione x della rana
    pos_bullet.y = pos_frog.y - 1;							// e nella posizione y - 1 (da sopra)

    int bullet_hit = FALSE;									// Flag che tiene traccia dell'hit del proiettile
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);	

    while (pos_bullet.y > 4 && !bullet_hit) { 				// Loop fino a quando il proiettile non esce dallo schermo
        
    	pos_bullet.y--;   									// Il proiettile sale verso l'alto
	
    	int hit_vehicle = 0; 								// Flag per indicare se il proiettile ha colpito un veicolo
			
		for (int i = 0; i < CAR_NUMBER; i++) {				// Loop per ogni macchina e bus
			
			// Controllo la collisione con le macchine
		    for (int j = 0; j < CAR_SIZE; j++){				
		        if ((pos_cars[i].x + j == pos_bullet.x) && (pos_cars[i].y == pos_bullet.y)) {
		            pthread_mutex_lock(&semBullet);
		            pos_bullet.x = AWAY;
					pos_bullet.y = AWAY;
		            mvaddch(pos_bullet.y, pos_bullet.x, ' '); // Cancella la posizione del proiettile
		            bullet_count--;
		            pthread_mutex_unlock(&semBullet);
		            bullet_hit = TRUE;
		            hit_vehicle = 1; // Imposta il flag per indicare che il proiettile ha colpito un veicolo
		            break;
		        }
		    }
		    if (hit_vehicle) { // Esci dai loop for se il proiettile ha colpito un veicolo
		        bullet_count --;
		        break;
		    }
		    
		    // Controllo la collisione con i bus
		    for (int j = 0; j < BUS_SIZE; j++){
		        if ((pos_bus[i].x + j == pos_bullet.x) && (pos_bus[i].y == pos_bullet.y)) {
		            pthread_mutex_lock(&semBullet);
		            pos_bullet.x = AWAY;
					pos_bullet.y = AWAY;
		            mvaddch(pos_bullet.y, pos_bullet.x, ' '); // Cancella la posizione del proiettile
		            bullet_count--;
		            pthread_mutex_unlock(&semBullet);
		            bullet_hit = TRUE;
		            hit_vehicle = 1; // Imposta il flag per indicare che il proiettile ha colpito un veicolo
		            break;
		        }
		    }
		    if (hit_vehicle) { // Esci dai loop for se il proiettile ha colpito un veicolo
		        bullet_count --;
		        break;
		    }
		    
		}
    
		for (int i = 0; i < LOGS_NUMBER; i++) {				// Loop per ogni tronco
			
			// Controllo la collisione con i tronchi 
		    for (int j = 0; j < LOG_SIZE; j++){
		        if ((pos_logs[i].x + 1 == pos_bullet.x || pos_logs[i].x - 1 == pos_bullet.x) && (pos_logs[i].y == pos_bullet.y || pos_logs[i].y + 1 == pos_bullet.y)) {
		        	if(log_occupied[i]){					// Controllo se è presente un nemico
				    	flash();
				    	points += 200;						// Incremento il punteggio
				    	countdown += 100;					// Incremento il countdown
				    	log_occupied[i] = false;			// Libero il tronco
				    	pthread_mutex_lock(&semBullet);		
				        pos_bullet.x = AWAY;
						pos_bullet.y = AWAY;
				        mvaddch(pos_bullet.y, pos_bullet.x, ' '); // Cancella la posizione del proiettile
				        bullet_count--;
				        pthread_mutex_unlock(&semBullet);
				        bullet_hit = TRUE;
				        hit_vehicle = 1; // Imposta il flag per indicare che il proiettile ha colpito un veicolo
				        break;
		            }
		        }
		    }
		    if (hit_vehicle) { // Esci dai loop for se il proiettile ha colpito un veicolo
		        bullet_count --;
		        break;
		    }
		}
		
    	if (hit_vehicle) { // Esci dal ciclo while se il proiettile ha colpito un veicolo
        	pthread_cancel(pthread_self());
    	}
		
    	usleep(DELAY);
        
    	sem_post(&semGame);  // avvisa il campo che c'e' stato un movimento
    
    }
    
	pthread_mutex_lock(&semBullet);
    bullet_count--; // Decrementa il numero di proiettili in volo
    pthread_mutex_unlock(&semBullet);

    pthread_exit(NULL);
}

/* Funzione 'macchina'.
 * Si muove orizzontalmente e se esce dallo schermo torna alla posizione iniziale
 */
void *car (void *arg) {
    
    srand(time(NULL));
    
    // Inizializza la posizione, la direzione di ogni macchina e bus
    for (int i = 0; i < CAR_NUMBER; i++) {
        pos_cars[i].x = rand() % (MAXX - CAR_SIZE);  	// posizione casuale
        pos_cars[i].y = street_y[i];
        pos_bus[i].x = rand() % (MAXX - BUS_SIZE);  	// posizione casuale
        pos_bus[i].y = street_y[i];
    }
    
    while (running) {
    
        for (int i = 0; i < CAR_NUMBER; i++) {          // Loop per ogni macchine\bus
            
            // check collisioni tra auto e bus
            for(int k = 0; k < CAR_SIZE + 2; k++){		
            	for(int j = 0; j < BUS_SIZE + 2; j++){
            		if(pos_cars[i].x + k == pos_bus[i].x + j){   			// se collidono
            			if(car_dir[i] == false){
            				pos_cars[i].x += rand() % (SPACE + BUS_SIZE);;	// sposto le cordinate in base alla direzione
            			} else {
            				pos_cars[i].x -= rand() % (SPACE + BUS_SIZE);;
            			}
                	}
            	}
            }
            
            if (car_dir[i] == false){
                // Se la macchina arriva vicino al bordo sinistro, rigenero
                if (pos_cars[i].x + CAR_SIZE <= 0) {
                    pos_cars[i].x = MAXX;
                }
                // Se il bus arriva vicino al bordo sinistro, rigenero
                if (pos_bus[i].x + BUS_SIZE <= 0) {
                    pos_bus[i].x = MAXX;
                }
            }
            
            if (car_dir[i] == true){
                // Se la macchina arriva vicino al bordo destro, rigenero
                if (pos_cars[i].x >= MAXX) {
                    pos_cars[i].x = 0 - CAR_SIZE;
                }
                // Se il bus arriva vicino al bordo destro, rigenero
                if (pos_bus[i].x >= MAXX) {
                    pos_bus[i].x = 0 - BUS_SIZE;
                }
            }
            
            // Aggiorna la posizione delle auto in base alla direzione
            pos_cars[i].x += car_dir[i] ? 1 : -1;
            pos_bus[i].x += car_dir[i] ? 1 : -1;
            
            usleep(20000);  // Pausa per 'dare il ritmo'
        }
        
        sem_post(&semGame);  // avvisa il campo che c'e' stato un movimento
    }
 
    return NULL;
}

/* Funzione 'tronco'
 * Si muove orizzontalmente rimbalzando sui bordi della mappa e con intervalli
 * di tempo casuali genera i nemici e gli spari nemici
 */
void *logs(void *arg) {

    srand(time(NULL));
    
    bool bullet_active[LOGS_NUMBER] = { false };	// Array per tenere traccia dei nemici che hanno gia sparato
    
    pthread_t tEnemyBullet[LOGS_NUMBER];      		// Array di thread per gli spari nemici

    // Inizializza la posizione, la direzione di ogni tronco
    for (int i = 0; i < LOGS_NUMBER; i++) {
        pos_logs[i].x = rand() % (MAXX - LOG_SIZE); // posizione casuale
        pos_logs[i].y = river_y[i];  				
        log_dir[i] = rand() % 2;  					// direzione casuale
    }

    while (running) {
    
        for (int i = 0; i < LOGS_NUMBER; i++) {		// Loop per ogni tronco
        
        	// Genera un nemico sopra il tronco se non è occupato
            if (!log_occupied[i] && !log_occupied_by_frog[i]) {
                if (rand() % 300 == 0) {  // Genera un nemico con intervallo casuale
                    pos_enemy[i].x = pos_logs[i].x + 2;
                    pos_enemy[i].y = pos_logs[i].y;
                    log_occupied[i] = true;
                }
            }
            
		    if (log_occupied[i]) {	//se il tronco è occupato
		        if (rand() % 150 == 0 && !bullet_active[i]) {		        
		            // genera un nuovo proiettile
		            pthread_mutex_lock(&semEnemyBullet); 	// Blocco del semaforo
		            if (enemy_bullet_count < MAX_BULLETS){
		            	pthread_create(&tEnemyBullet[i], NULL, enemy_bullet, &pos_logs[i]);
		            	enemy_bullet_count++;
		            }
		            pthread_mutex_unlock(&semEnemyBullet); 	// Sblocco del semaforo
		        }
		    }
			
            // Se il tronco arriva vicino al bordo sinistro, cambia direzione
            if (pos_logs[i].x <= 0) {
                log_dir[i] = true;
            }
            // Se il tronco arriva vicino al bordo destro, cambia direzione
            if (pos_logs[i].x + LOG_SIZE >= MAXX) {
                log_dir[i] = false;
            }

            // Aggiorna la posizione del tronco in base alla direzione
            pos_logs[i].x += log_dir[i] ? 1 : -1;
            
            // Se c'è un nemico sopra il tronco, muovilo insieme al tronco
            if (log_occupied[i]) {
                pos_enemy[i].x += log_dir[i] ? 1 : -1;
            }
            
            // Verifica se la rana si trova sopra un tronco
			if (pos_frog.y == pos_logs[i].y && pos_frog.x >= pos_logs[i].x &&
				pos_frog.x < pos_logs[i].x + LOG_SIZE) {
				log_occupied_by_frog[i] = true;				// aggiorno il flag
			} else {
				log_occupied_by_frog[i] = false;			// aggiorno il flag
			}
			
            usleep(10000);  // Pausa per 'dare il ritmo'
        }

        sem_post(&semGame);  // avvisa il campo che c'e' stato un movimento
    }
    
    for(int i = 0; i < LOGS_NUMBER; i++){
    	pthread_join (tEnemyBullet[i], NULL);	            // aspetto la terminazione dei thread proiettile
    }

    return NULL;
}


/* Funzione 'rana'.
 * Legge i tasti premuti dall'utente. Sposta la rana nella direzione
 * indicata dalle frecce. Si ferma ai bordi del campo.
 * Segala ogni spostamento al campo tramite il semaforo
 */
void *frog (void *arg) {

  	// Posizione iniziale : in basso al centro
  	pos_frog.x = MAXX / 2;
  	pos_frog.y = MAXY - 4;
  
  	pthread_t tBullet;	// Thread per il proiettile

  	while(running){    	// Ciclo fino a che 'campo' non dice basta
  
  		char c;
  	
    	c = getch();                // Legge il tasto 
  
    	if (c==SU && pos_frog.y  > 0){
        	if( pos_frog.y <= 4 && (pos_frog.x >= 2 && pos_frog.x < 6) || (pos_frog.x >= 10 && pos_frog.x < 14) || (pos_frog.x >= 18 && pos_frog.x < 22) || (pos_frog.x >= 26 && pos_frog.x < 30) || (pos_frog.x >= 34 && pos_frog.x < 38)){
        		pos_frog.y -= 2;
        	} else if (pos_frog.y > 4){
        		pos_frog.y -= 2;
        	} else {
        		continue;
        	}      
   	 	}

		if(c==GIU  && pos_frog.y < MAXY - 4){   // Freccia GIU
		    pos_frog.y += 2;                	// Cresce la y
		}

		if((c==SINISTRA)&&pos_frog.x > 0){      // Freccia SINISTRA
		    pos_frog.x -= 2;                	// Cala la x
		}

		if(c==DESTRA && pos_frog.x < MAXX - 3){ // Freccia DESTRA
		    pos_frog.x += 2;                	// Cresce la x
		}
		
		if(c=='q'){								// Se il giocatore preme q il gioco termina
			running = FALSE;					
		}
	
		if(c == ' ') {							// Se preme spazio spara un proiettile
			pthread_mutex_lock(&semBullet); 	// Blocco del semaforo
			if (bullet_count < MAX_BULLETS){
				pthread_create(&tBullet, NULL, bullet, &bullet_count);
				bullet_count ++;
			}
			pthread_mutex_unlock(&semBullet); 	// Sblocco del semaforo
		}  

    	sem_post (&semGame);            // avviso il campo che c'e' stato un movimento
  	}
  	
  	pthread_join (tBullet, NULL);		// attendo che termini il thread del proiettile
  	return NULL;
}

/* Funzione di controllo (padre).
 * Attende 'eventi' sul semaforo.
 * Stampa le varie entità e segnala le collisioni.
 */
void map () {

	time_t start_time = time(NULL);
    
  	while(running){                							// Ciclo finche' non viene azzerato il flag
    
    	sem_wait (&semGame);            					// Attendo che un figlio si muova
    
    	clearScreen();										// Pulisco lo schermo
      
      	// MARCIAPIEDE
  		for (int i = 0; i < MAXX; i++){
		    pthread_mutex_lock (&semCurses);				// Attendo che curses sia libero
			attron(COLOR_PAIR(4));
			mvprintw(MAXY - 3, i, "O");        
			mvprintw(MAXY - 4, i, "O");        
			mvprintw(MAXY - 15, i, "O");       
			mvprintw(MAXY - 16, i, "O");
			attroff(COLOR_PAIR(4));				
			pthread_mutex_unlock (&semCurses);				// Sblocco curses       
  		}

      	// FIUME
		for (int i = 0; i < MAXX; i++){
			for (int j = 17; j <= 26; j++ ){
		    	pthread_mutex_lock (&semCurses);        	// Attendo che curses sia libero
		      	attron(COLOR_PAIR(3));
		      	mvprintw(MAXY - j , i, " ");
			  	attroff(COLOR_PAIR(3));
		      	pthread_mutex_unlock (&semCurses);        	// Sblocco curses
          	}
      	}
      
      
      	// TANE
      	// RIGA 30 e 29
      	for (int i = 0; i < MAXX; i++) {
        	pthread_mutex_lock (&semCurses);        		// Attendo che curses sia libero
          	attron(COLOR_PAIR(5));
          	mvprintw(MAXY - 30, i, "O");
          	mvprintw(MAXY - 29, i, "O");
          	attroff(COLOR_PAIR(5));
          	pthread_mutex_unlock (&semCurses);        	  	// Sblocco curses
      	}
      
		// RIGA 28-27
		for (int row = 0; row < 2; row++) {                 //disegna il prato tranne nella posizione in cui si trova la tana
			for (int col = 0; col < 40; col++) {
				if (col < 2 || col > 37 ) {
					pthread_mutex_lock (&semCurses);        // Attendo che curses sia libero
					attron(COLOR_PAIR(5));
					mvprintw(MAXY - 28 + row, col, "O");
					attroff(COLOR_PAIR(5));
					pthread_mutex_unlock (&semCurses);       // Sblocco curses
				} else if (col >= 2 & col < 6) {
					pthread_mutex_lock (&semCurses);        	
					lairs[0] ? attron(COLOR_PAIR(5)) : attron(COLOR_PAIR(3));    // Se la tana è aperta il colore è blu se no è verde
					mvprintw(MAXY - 28 + row, col, " ");
					lairs[0] ? attroff(COLOR_PAIR(5)) : attroff(COLOR_PAIR(3));
					pthread_mutex_unlock (&semCurses);        
				} else if (col > 5 & col < 10) {
					pthread_mutex_lock (&semCurses);        	
					attron(COLOR_PAIR(5));
					mvprintw(MAXY - 28 + row, col, "O");
					attroff(COLOR_PAIR(5));
					pthread_mutex_unlock (&semCurses);        
				} else if (col > 9 & col < 14){
					pthread_mutex_lock (&semCurses);        	
					lairs[1] ? attron(COLOR_PAIR(5)) : attron(COLOR_PAIR(3));
					mvprintw(MAXY - 28 + row, col, " ");
					lairs[1] ? attroff(COLOR_PAIR(5)) : attroff(COLOR_PAIR(3));
					pthread_mutex_unlock (&semCurses);        
				} else if ( col > 13 & col < 18){
					pthread_mutex_lock (&semCurses);        	
					attron(COLOR_PAIR(5));
					mvprintw(MAXY - 28 + row, col, "O");
					attroff(COLOR_PAIR(5));
					pthread_mutex_unlock (&semCurses);        
				} else if ( col > 17 & col < 22){
					pthread_mutex_lock (&semCurses);        	
					lairs[2] ? attron(COLOR_PAIR(5)) : attron(COLOR_PAIR(3));
					mvprintw(MAXY - 28 + row, col, " ");
					lairs[2] ? attroff(COLOR_PAIR(5)) : attroff(COLOR_PAIR(3));
					pthread_mutex_unlock (&semCurses);        
				} else if ( col > 21 & col < 26){
					pthread_mutex_lock (&semCurses);        	
					attron(COLOR_PAIR(5));
					mvprintw(MAXY - 28 + row, col, "O");
					attroff(COLOR_PAIR(5));
					pthread_mutex_unlock (&semCurses);        
				} else if ( col > 25 & col < 30){
					pthread_mutex_lock (&semCurses);        	
					lairs[3] ? attron(COLOR_PAIR(5)) : attron(COLOR_PAIR(3));
					mvprintw(MAXY - 28 + row, col, " ");
					lairs[3] ? attroff(COLOR_PAIR(5)) : attroff(COLOR_PAIR(3));
					pthread_mutex_unlock (&semCurses);        
				} else if ( col > 29 & col < 34){
					pthread_mutex_lock (&semCurses);        	
					attron(COLOR_PAIR(5));
					mvprintw(MAXY - 28 + row, col, "O");
					attroff(COLOR_PAIR(5));
					pthread_mutex_unlock (&semCurses);        
				} else {
					pthread_mutex_lock (&semCurses);        	
					lairs[4] ? attron(COLOR_PAIR(5)) : attron(COLOR_PAIR(3));
					mvprintw(MAXY - 28 + row, col, " ");
					lairs[4] ? attroff(COLOR_PAIR(5)) : attroff(COLOR_PAIR(3));
					pthread_mutex_unlock (&semCurses);        
				}
			}
		}
    
		// PUNTEGGIO
		pthread_mutex_lock (&semCurses);
		attron(COLOR_PAIR(0));
		mvprintw(MAXY - 2, 0, "PUNTI: %d", points);
		attroff(COLOR_PAIR(0));
		pthread_mutex_unlock (&semCurses);
    
		//se il timer arriva a 0, il gioco si riavvia con una vita in meno
	 	if (countdown > 0) {
			time_t current_time = countdown--;
			if (current_time - start_time >= 1500) {
			    start_time = current_time;
			    countdown--;
			}
		} else {
			life--;
			countdown = 1500;
			flash();
			msg = TIME;
			initGame();
		}
    
		// TIMER
		// Calcola la percentuale del tempo rimanente
		int percent = (countdown * 100) / 1500;

     	// Disegna la barra
     	// Imposta il colore di sfondo per la parte piena della barra
    	for (int i = 0; i < MAXX; i++) {
        	if (i < (percent * MAXX / 100)) {
		    	pthread_mutex_lock (&semCurses);
		        attron(COLOR_PAIR(6));
		        mvprintw(MAXY - 1, 0+ i, "|");
		        attroff(COLOR_PAIR(6)); // Ripristina il colore precedente
		        pthread_mutex_unlock (&semCurses);
        	} else {
		    	pthread_mutex_lock (&semCurses);
		        mvprintw(MAXY - 1, 0 + i, " ");
		        pthread_mutex_unlock (&semCurses);
        	}
    	}
    	
    	// VITA
		pthread_mutex_lock (&semCurses);
		attron(COLOR_PAIR(0));
		mvprintw(MAXY - 2, 35, "HP: %d", life);
		attroff(COLOR_PAIR(0));
		pthread_mutex_unlock (&semCurses);
		
		// Verifica se la rana si trova sopra un tronco
		for (int i = 0; i < LOGS_NUMBER; i++) {
			if (pos_frog.y == pos_logs[i].y && pos_frog.x >= pos_logs[i].x &&
		    	pos_frog.x <= pos_logs[i].x + LOG_SIZE) {
		    	// La rana si trova sopra il tronco, aggiorna la posizione della rana in base alla
		    	// direzione di movimento del tronco
		    	if(!log_occupied[i]){                              // se non c'e nemico 
		    		pos_frog.x = pos_logs[i].x + LOG_SIZE/2 - 1;   // mi sposto con il tronco
		    	} else {										   // in caso contrario il player viene ucciso
		    		life--;
					points -= 100;
					countdown = 1500;
					flash();
  					msg = ENEMY;
  					initGame();
		   	 	}
			} else if (pos_frog.y == pos_logs[i].y) {				// se non è sopra il tronco cade in acqua e muore
				life--;
				points -= 100;
				countdown = 1500;
				flash();
	  			msg = SPLASH;
	  			initGame();
			}
		}
		
		// Disegna i tronchi nella nuova posizione
		for (int i = 0; i < LOGS_NUMBER; i++) {
			for (int j = 0; j < LOG_SIZE; j++) {
	   			pthread_mutex_lock(&semCurses);
				attron(COLOR_PAIR(7));
				mvaddch(pos_logs[i].y, pos_logs[i].x + j, 'O');
				mvaddch(pos_logs[i].y+1, pos_logs[i].x + j, 'O');  // Disegna anche la riga sotto
				attroff(COLOR_PAIR(7));
				pthread_mutex_unlock(&semCurses);
			}
		}

    	// Disegna i nemici sopra i tronchi 
		for (int i = 0; i < LOGS_NUMBER; i++) {
			for ( int j = 0; j < FROG_SIZE; j++){
				if (log_occupied[i]) {
					pthread_mutex_lock(&semCurses);
					attron(COLOR_PAIR(1));
					mvaddch(pos_enemy[i].y, pos_enemy[i].x + j, 'w');
					mvaddch(pos_enemy[i].y + 1, pos_enemy[i].x + j, '-');
					attroff(COLOR_PAIR(1));
					pthread_mutex_unlock(&semCurses);
				}
			}	
		}
		
		// Disegna l'auto nella nuova posizione
		for(int i = 0; i < CAR_NUMBER; i++){
		    for (int j = 0; j < CAR_SIZE; j++) {
		        if(range(0, MAXX - 1, pos_cars[i].x + j)){
					pthread_mutex_lock(&semCurses);
		        	attron(COLOR_PAIR(1));
		            mvaddch(pos_cars[i].y, pos_cars[i].x + j, 'A');
		            mvaddch(pos_cars[i].y + 1, pos_cars[i].x + j, 'A');  // Disegna anche la riga sotto
		            attroff(COLOR_PAIR(1));
		    		pthread_mutex_unlock(&semCurses);
		            
		        }
		    }        
				
		    // Disegna l'auto nella nuova posizione
		    for (int j = 0; j < BUS_SIZE; j++) {
		        if(range(0, MAXX - 1, pos_bus[i].x + j)){
		    		pthread_mutex_lock(&semCurses);
		        	attron(COLOR_PAIR(2));
		            mvaddch(pos_bus[i].y, pos_bus[i].x + j, 'B');
		            mvaddch(pos_bus[i].y+1, pos_bus[i].x + j, 'B');  // Cancella anche la riga sotto
		            attroff(COLOR_PAIR(2));
		    		pthread_mutex_unlock(&semCurses);
		            
		        }
		    }
		}
		
		pthread_mutex_lock (&semCurses);        		// Attendo che curses sia libero
    	attron(COLOR_PAIR(6));
    	mvaddch( pos_frog.y, pos_frog.x, '#');     		// Disegno la rana 
    	mvaddch( pos_frog.y, pos_frog.x + 1, '#');     	
    	mvaddch( pos_frog.y + 1, pos_frog.x, '#');     	
    	mvaddch( pos_frog.y + 1, pos_frog.x + 1, '#');  
    	attroff(COLOR_PAIR(6));
    	pthread_mutex_unlock (&semCurses);        		// libero curses
		
		pthread_mutex_lock (&semCurses);        // Attendo che curses sia libero
		pthread_mutex_lock(&semBullet);
		attron(COLOR_PAIR(6));
		mvaddch(pos_bullet.y, pos_bullet.x, '^'); // Stampa il proiettile alla nuova posizione
		attroff(COLOR_PAIR(6));
		pthread_mutex_unlock(&semBullet); // Sblocco del semaforo
		pthread_mutex_unlock (&semCurses);        // libero curses
		
		if(pos_bullet.y <= 4){
			pthread_mutex_lock (&semCurses);        // Attendo che curses sia libero
			pthread_mutex_lock(&semBullet);
			attron(COLOR_PAIR(3));
			mvaddch(pos_bullet.y, pos_bullet.x, ' '); // Stampa il proiettile alla nuova posizione
			attroff(COLOR_PAIR(3));
			pthread_mutex_unlock(&semBullet); // Sblocco del semaforo
			pthread_mutex_unlock (&semCurses);        // libero curses
		}
		
		pthread_mutex_lock (&semCurses);        // Attendo che curses sia libero
		pthread_mutex_lock(&semEnemyBullet);
		mvaddch(pos_enemy_bullet->y, pos_enemy_bullet->x, 'v'); // Stampa il proiettile alla nuova posizione
		pthread_mutex_unlock(&semEnemyBullet); // Sblocco del semaforo
		pthread_mutex_unlock (&semCurses);        // libero curses
		
		if(pos_enemy_bullet->y >= MAXY - 4){
			pthread_mutex_lock (&semCurses);        // Attendo che curses sia libero
			pthread_mutex_lock(&semEnemyBullet);
			attron(COLOR_PAIR(4));
			mvaddch(pos_enemy_bullet->y, pos_enemy_bullet->x, ' '); // Stampa il proiettile alla nuova posizione
			attroff(COLOR_PAIR(4));
			pthread_mutex_unlock(&semEnemyBullet); // Sblocco del semaforo
			pthread_mutex_unlock (&semCurses);        // libero curses
		}
    
		for(int i = 0; i < CAR_NUMBER; i++){						// Loop per ogni macchina/bus
		
			for(int s = 0; s < CAR_SIZE; s++){ 						// controllo collisione con le macchine
				if( (pos_frog.x == pos_cars[i].x + s || pos_frog.x + 1 == pos_cars[i].x + s) && pos_frog.y == pos_cars[i].y){     	// se collide il player muore 
					life--;
					points -= 100;
					countdown = 1500;
					flash();
					msg = CRASH;
		  			initGame();
				}
			}
			
			for(int s = 0; s < BUS_SIZE; s++){						// controllo collisione con i bus
				if( (pos_frog.x == pos_bus[i].x + s || pos_frog.x + 1 == pos_bus[i].x + s) && pos_frog.y == pos_bus[i].y){    		// se collide il player muore
					life--;
					points -= 100;
					countdown = 1500;
					flash();
		  			msg = CRASH;
		  			initGame();
				}
			}
		}
    
		if (life < 1){				// se la vita scende a 0 è GAME OVER
			running = FALSE;
		    clearScreen();			
			mvprintw(MAXY/2, MAXX/2, "GAME OVER!");
			getch();
			cbreak();
		}
    
		for(int i = 0; i < MAXX; i++ ){						// Per ogni tana controllo se la rana vi arriva, in caso di tana chiusa il player muore,
			if(pos_frog.y > 1 && pos_frog.y < 4){			// in caso contrario la tana si chiude e si va alla prossima manche
		    	if ( pos_frog.x >= 2 & pos_frog.x < 6 ) {
		        	if(lairs[0] == true){
	                	life--;
	                	points -= 100;
	                    countdown = 1500;
						flash();
	  					msg = CLOSED;
	  					initGame();
		            } else {
	                	lairs[0] = true;
	                    countdown += 300;
	                    points += 500;
						flash();
	  					msg = WIN;
	  					initGame();
		            }
				} else if ( pos_frog.x > 9 & pos_frog.x < 14 ) {
	                if(lairs[1] == true){
	                    life--;
	                	points -= 100;
	                    countdown = 1500;
						flash();
	  					msg = CLOSED;
	  					initGame();
	                } else {
	                	lairs[1] = true;
	                    countdown += 300;
	                    points += 500;
						flash();
	  					msg = WIN;
	  					initGame();
	                }
		        } else if ( pos_frog.x > 17 & pos_frog.x < 22 ) {
	                if(lairs[2] == true){
	                    life--;
	                	points -= 100;
	                    countdown = 1500;
						flash();
	  					msg = CLOSED;
	  					initGame();
	                } else {
	                	lairs[2] = true;
	                    countdown += 300;
	                    points += 500;
						flash();
	  					msg = WIN;
	  					initGame();
	                }
		        } else if ( pos_frog.x > 25 & pos_frog.x < 30 ) {
	                if(lairs[3] == true){
	                    life--;
	                	points -= 100;
	                    countdown = 1500;
						flash();
	  					msg = CLOSED;
	  					initGame();
	                } else {
	                	lairs[3] = true;
	                    countdown += 300;
	                    points += 500;
						flash();
	  					msg = WIN;
	  					initGame();
	                }
		        } else if ( pos_frog.x > 33 & pos_frog.x < 38 ) {
		            if(lairs[4] == true){
		                life--;
		            	points -= 100;
		                countdown = 1500;
						flash();
	  					msg = CLOSED;
	  					initGame();
		            } else {
		            	lairs[4] = true;
		                countdown += 300;
		                points += 500;
						flash();
	  					msg = WIN;
	  					initGame();
		  			}
	            }
	        }
		}
		
		if(lairs[0] && lairs[1] && lairs[2] && lairs[3] && lairs[4]){      // se tutte le tane sono chiuse allora il player vince!
			running = FALSE;
			clearScreen();			
			mvprintw(MAXY/2, MAXX/2, "HAI VINTO!");
			getch();
			cbreak();
		}
    	
    	// check per la collisione proiettile_nemico - player
		if ((pos_frog.x == pos_enemy_bullet->x || pos_frog.x + 1 == pos_enemy_bullet->x) && (pos_frog.y == pos_enemy_bullet->y || pos_frog.y + 1 == pos_enemy_bullet->y)) {
			flash();
			life--;
			points -= 200;
			countdown = 1500;
		    msg = ENEMY;
		    initGame();
		}
    
    	refresh();                    // Aggiorno le modifiche (canc. e disegno)
  	}
}
