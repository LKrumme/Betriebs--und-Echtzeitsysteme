/* FreeRTOS.org includes. */
#include "FreeRTOS.h"
#include "wrap.h"
#include "startup.h"
#include "task.h"
#include "semphr.h"
#include "framebuffer.h"
#include "font.h"
#include "platform.h"

#define REG(P) (*(volatile uint32_t *) (P))

#define GPIO_BASE 0x10012000
#define GPIO_PUE 0x10

#define PLIC_BASE 0x0C000000
#define PLIC_ENABLE 0x2000

//PINS
#define BLUE_LED 5
#define GREEN_BUTTON 18
#define BLUE_BUTTON 19
#define YELLOW_BUTTON 20
#define RED_BUTTON 21

const int buttons[4] = {GREEN_BUTTON, BLUE_BUTTON, RED_BUTTON, YELLOW_BUTTON};

//CONSTANTS
#define STANDARD_WAIT_SEMAPHORE 2 //ms
#define UPDATE_RATE_BALL 20 //ms
#define UPDATE_RATE_PLAYER 5 //ms
#define GAME_REFRESHRATE 60 //hz
#define PLAYER_STEP 1
#define DEBOUNCE_THRESHOLD 10 

//Structs
struct key_pres
{
	int button_pressed;
	int button_num;
} key_press_size, key_press_player, key_press_keyboard; //Drei variablen, damit diese nicht Daten untereinadner austauschen, falls etwas schiefgeht.

//Tasks
void ball_logic(void *pvParameters);
void player_logic(void *pvParameters);
void score(void *pvParameters);
void keyboard(void *pvParameters);
void display(void *pvParameters);

//Variables

//BALL
float fVelocityX = 1; //Später random wählen
float fVelocityY = 1;
float fPositionX = 7;
float fPositionY = DISP_H/2;

//PLAYERS

//Links ist Spieler 1
const int player1 = 1;
const int fPositionPlayer1X = 5;
const int fWidthPlayer1X= 1;
float fPositionPlayer1Y = DISP_H/2;
const float fWidthPlayer1Y = 5;
int iScorePlayer1 = 0;

//Rechts ist Spieler 2
const int player2 = 2;
const int fPositionPlayer2X = DISP_W-5;
const int fWidthPlayer2X = 1;
float fPositionPlayer2Y = DISP_H/2;
const float fWidthPlayer2Y = 5;
int iScorePlayer2 = 0;

//INTERPROCESS COMMUNICATION
//EVERY ACTION WITH THESE NEEDS TO BE SECURED
SemaphoreHandle_t xDraw_screen;
SemaphoreHandle_t xScore_Update_Mutex;
QueueHandle_t xScore_Update;
SemaphoreHandle_t xKey_Queue_Mutex;
QueueHandle_t xKey_Queue;

// interrupt handler for handling all unclaimed interrupts
void irq_handler(void)
{
	// do nothing
	asm volatile ("nop");
}

int main( void )
{
	// deactivate PLIC
    REG(PLIC_BASE + PLIC_ENABLE) = 0;
    REG(PLIC_BASE + PLIC_ENABLE + 4) = 0;

	//Init LED
	// setup LED as output
	// disable pin-specific functions
	REG(GPIO_BASE + GPIO_IOF_EN) &= ~(1 << BLUE_LED);
	// disable input function of the pin
	REG(GPIO_BASE + GPIO_INPUT_EN) &= ~(1 << BLUE_LED);
	// enable output function of the pin
	REG(GPIO_BASE + GPIO_OUTPUT_EN) |= 1 << BLUE_LED;
	// set led pin to high
	REG(GPIO_BASE + GPIO_OUTPUT_VAL) |= (1 << BLUE_LED);


	for(int i=0; i<4; i++){
		// setup BUTTON as input
		// disable pin-specific functions
		REG(GPIO_BASE + GPIO_IOF_EN) &= ~(1 << buttons[i]);
		// enable pull-up resistor
		REG(GPIO_BASE + GPIO_PUE) |= 1 << buttons[i];
		// enable input function of the pin
		REG(GPIO_BASE + GPIO_INPUT_EN) |= 1 << buttons[i];
		// disable output function of the pin
		REG(GPIO_BASE + GPIO_OUTPUT_EN) &= ~(1 << buttons[i]);
		// set button pin to low
		REG(GPIO_BASE + GPIO_OUTPUT_VAL) &= ~(1 << buttons[i]);
	}

	oled_init();

	fb_init();


	//TODO Tasks initialisieren

	//Setup Semaphores etc.
	xDraw_screen = xSemaphoreCreateBinary();
	xScore_Update = xQueueCreate(2, sizeof(int)); //Könnte Speicheroptimierung vertragen, da man für Zwei Spieler nur einen Byte braucht, aber er hat mich nicht machen lassen
	xScore_Update_Mutex =xSemaphoreCreateMutex();
	xKey_Queue = xQueueCreate(10, sizeof(key_press_size));
	xKey_Queue_Mutex = xSemaphoreCreateMutex();
	
	if(xDraw_screen == NULL || xScore_Update == NULL || xScore_Update_Mutex == NULL || xKey_Queue == NULL || xKey_Queue_Mutex == NULL){ //Kritischer Fehler. Programm kann ohne nicht laufen
		//led blinken lassen
		volatile uint32_t i = 0;
		while(1){
			// set led pin to low
			REG(GPIO_BASE + GPIO_OUTPUT_VAL) &= ~(1 << BLUE_LED);
			// wait..
			for (i = 0; i < 100000; i++){}

			// set led pin to high
			REG(GPIO_BASE + GPIO_OUTPUT_VAL) |= (1 << BLUE_LED);
			// wait..
			for (i = 0; i < 100000; i++){}
		}
	}
	else{
		vTaskStartScheduler();
	}

	// dummy loop
	for (;;) {
		// does nothing
		asm volatile ("nop");
	}

	return 0;
}

void ball_logic(void *pvParameters){
	//Bewegung des Balls ✅
	//Komponenten fVelocityX, fVelocityY ✅
	//Prüfung ob der Ball auf einen Bildschirmrand trifft ✅
	//Prüfung ob der Ball auf einen Schläger trifft ✅
	//Wenn Bildschirmrand hinter dem Spieler ist, dann wird die score_update-Queue gesetzt, die ein Signal an den score-Task setzt ✅
	//draw_screen Semaphore wird nach jedem Update gesetzt, die neu Zeichnen des Displays auslöst ✅

	for(;;){
		//Erst Prüfungen, dann setzten
		if(fPositionX < 1){
			//Linker Rand. Spieler 2 bekommt Punkt
			if(xSemaphoreTake(xScore_Update_Mutex, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE))==pdTRUE){
				xQueueSend(xScore_Update, &player2, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE));
				xSemaphoreGive(xScore_Update_Mutex);
			}
			else{
				//Fehler beim Score.
			}
		}
		else if(fPositionX > DISP_W-1){
			//Rechter Rand. Spieler 1 bekommt Punkt 
			if(xSemaphoreTake(xScore_Update_Mutex, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE))==pdTRUE){ //Code hier ist extra dupliziert um Kritische Sektion möglichst klein zu halten.
				xQueueSend(xScore_Update, &player1, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE));
				xSemaphoreGive(xScore_Update_Mutex);
			}
			else{
				//Fehler beim Score.
			}
		}
		if(fPositionY > DISP_H-1 || fPositionY < 1){ //Trifft auf Rand oben oder unten 
			//Nur Y Velocity Umkehren 
			fVelocityY = fVelocityY*(-1);
		}
		if((fPositionX > fPositionPlayer2X+fWidthPlayer2X && fPositionX < fPositionPlayer2X+fWidthPlayer2X) && (fPositionY > fPositionPlayer2Y-fWidthPlayer2Y && fPositionY < fPositionPlayer2Y+fWidthPlayer2Y)){ //Trifft auf Spieler2
			if(fVelocityX>0){ //Nur wenn der Ball nach Rechts fliegt soll seine Postion umgekehrt werden.
				fVelocityX = fVelocityX*(-1);
			}
		}
		else if((fPositionX > fPositionPlayer1X+fWidthPlayer1X && fPositionX < fPositionPlayer1X+fWidthPlayer1X) && (fPositionY > fPositionPlayer1Y-fWidthPlayer1Y && fPositionY < fPositionPlayer1Y+fWidthPlayer1Y)){ //Trifft auf Spieler1
			if(fVelocityX<0){ //Nur wenn der Ball nach Links fliegt soll seine Position umgekehrt werden. 
				fVelocityX = fVelocityX*(-1);
			}
		}
		//Ball position updaten
		fPositionX = fPositionX+fVelocityX;
		fPositionY = fPositionY+fVelocityY;

		//Tell draw_screen Task to do its Job
		if(xSemaphoreTake(xDraw_screen, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE)==pdTRUE)){//Warten bis Ressource freigegeben wird
			//Ressource genommen -> draw_screen kann Ressource jetzt freigeben
		} 
		else {
			//Ressource konnte nicht enommeng werden. Fehler, aber nicht schlimm, wenn der Bildschirm nicht aktualisiert wurde.
		}
		vTaskDelay( pdMS_TO_TICKS( UPDATE_RATE_BALL ) );
	}	
}

void player_logic(void *pvParameters){
	//Internes speichern der Button-Zustände
	int green_pressed;
	int blue_pressed;
	int yellow_pressed;
	int red_pressed;
	
	for(;;){
		if(xSemaphoreTake(xKey_Queue_Mutex, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE)==pdTRUE)){
			if(xQueueReceive(xKey_Queue, &key_press_player, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE))==pdPASS){
				xSemaphoreGive(xKey_Queue_Mutex);
				//Button handeln
				switch (key_press_player.button_num)
				{
				case GREEN_BUTTON:
					green_pressed = key_press_player.button_pressed;
					break;
				case BLUE_BUTTON: 
					blue_pressed = key_press_player.button_pressed;
					break;
				case YELLOW_BUTTON:
					yellow_pressed = key_press_player.button_pressed;
					break;
				case RED_BUTTON:
					red_pressed = key_press_player.button_pressed;
					break;
				default:
					break;
				}
			}
			else{
				xSemaphoreGive(xKey_Queue_Mutex);
				//Queue ist leer. Nicht schlimm. Einfach weitermachen.
			}
		}

		//Spieler1 bewegen: 
		if(green_pressed){ //hoch
			if(fPositionPlayer1Y+fWidthPlayer1Y < DISP_H){ //Out of Bounds check
				fPositionPlayer1Y = fPositionPlayer1Y+PLAYER_STEP;
			}
		}
		if(blue_pressed){ //runter
			if(fPositionPlayer1Y-fWidthPlayer1Y > 0){
				fPositionPlayer1Y = fPositionPlayer1Y-PLAYER_STEP;
			}
		}

		//Spieler2 bewegen: 
		if(red_pressed){ //hoch
			if(fPositionPlayer2Y+fWidthPlayer2Y < DISP_H){
				fPositionPlayer2Y = fPositionPlayer2Y+PLAYER_STEP;
			}
		}
		if(yellow_pressed){ //runter
			if(fPositionPlayer2Y-fWidthPlayer2Y > 0){
				fPositionPlayer2Y = fPositionPlayer2Y+PLAYER_STEP;
			}
		}


		//Tell draw_screen Task to do its Job
		if(xSemaphoreTake(xDraw_screen, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE)==pdTRUE)){//Warten bis Ressource freigegeben wird
			//Ressource genommen -> draw_screen kann Ressource jetzt freigeben
		} 
		else {
			//Ressource konnte nicht genommen werden. Fehler, aber nicht schlimm, wenn der Bildschirm nicht aktualisiert wurde.
		}
		vTaskDelay( pdMS_TO_TICKS( UPDATE_RATE_PLAYER ) );
	}

}

void score(void *pvParameters){
	//Update Score
	//Restart game if one of the Players has Score 3 
	int player_to_increase;
	for(;;){
		if(xSemaphoreTake(xScore_Update_Mutex, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE))==pdTRUE){
			if(xQueueReceive(xScore_Update,&player_to_increase, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE))==pdTRUE){
				xSemaphoreGive(xScore_Update_Mutex);
				if(player_to_increase==player1){
					iScorePlayer1++;
				}
				else if(player_to_increase==player2){
					iScorePlayer2++;
				}
			}
			else{
				xSemaphoreGive(xScore_Update_Mutex);
			}
		}
		//RESTART
		if(iScorePlayer1 == 3 || iScorePlayer2 == 3){
			//Muss ich hier verhindern, dass die anderen Tasks drauf zugreifen können? ja
			//BALL
			fVelocityX = 1; //Später random wählen
			fVelocityY = 1;
			fPositionX = 7;
			fPositionY = DISP_H/2;

			//PLAYERS

			//Links ist Spieler 1
			fPositionPlayer1Y = DISP_H/2;
			iScorePlayer1 = 0;

			//Rechts ist Spieler 2
			fPositionPlayer2Y = DISP_H/2;
			iScorePlayer2 = 0;
		}
		//Tell draw_screen Task to do its Job
		if(xSemaphoreTake(xDraw_screen, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE)==pdTRUE)){//Warten bis Ressource freigegeben wird
			//Ressource genommen -> draw_screen kann Ressource jetzt freigeben
		} 
		else {
			//Ressource konnte nicht genommen werden. Fehler, aber nicht schlimm, wenn der Bildschirm nicht aktualisiert wurde.
		}
		vTaskDelay( pdMS_TO_TICKS( UPDATE_RATE_BALL ) );
	}
}


void keyboard(void *pvParameters){
	int current;
	//Arrays um buttons zu überwachen
	int button_last_states[4] = {0,0,0,0};
	int button_last_pressed[4] = {0,0,0,0};
	int button_count[4] = {0,0,0,0};

	for(;;){
		//Einmal durch jeden Knopf gehen
		for(int i=0; i<4; i++){
			current = REG(GPIO_BASE + GPIO_INPUT_VAL) & (1 << buttons[i]);//button lesen
			//Hochzählen für das debouncen
			if(current && button_last_states[i]){
				button_count[i]++;
			}
			else if (!current && !button_last_states[i]){
				button_count[i]++;
			}
			else{
				button_count[i] = 0;
			}

			if(button_count[i]>DEBOUNCE_THRESHOLD && button_last_pressed[i]!=current){ //Debounce check und check ob der Button state sich zum letzten Event geändert hat

				button_last_pressed[i]=current;
				
				//Struct fürs Abschicken beschreieben
				key_press_keyboard.button_num = buttons[i];
				key_press_keyboard.button_pressed = current;

				//Mutex anfragen und Queue bestücken
				if(xSemaphoreTake(xKey_Queue_Mutex, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE))==pdTRUE){
					xQueueSend(xKey_Queue, &key_press_keyboard, pdMS_TO_TICKS(STANDARD_WAIT_SEMAPHORE));
					xSemaphoreGive(xKey_Queue_Mutex);
				}
			}
			button_last_states[i] = current;
		}

		key_press_keyboard.button_num=0;
		key_press_keyboard.button_pressed=0;
		//Doppelt so schnell wie die Spieleraktualisierung, damit beide Spieler Ihre knöpfe drücken können ohne, dass die Queue voll läuft.
		vTaskDelay(pdMS_TO_TICKS(UPDATE_RATE_PLAYER/2)); 
	}
}

void display(void *pvParameters){

	for(;;){
		//Score mit printChar() malen
		//Ball und Spieler mit fb_set_pixel() malen
		//TODO Score, Ball und Spielerposition mit MUTEX absichern
		
		//Hier display malen

		fb_flush();
		vTaskDelay(pdMS_TO_TICKS(1/GAME_REFRESHRATE));
	}
}