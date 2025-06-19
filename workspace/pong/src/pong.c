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

#define PLIC_BASE 0x0C000000
#define PLIC_ENABLE 0x2000

//CONSTANTS
#define STANDARD_WAIT_SEMAPHORE 2 //ms
#define UPDATE_RATE_BALL 20 //ms
#define UPDATE_RATE_PLAYER 5 //ms
#define PLAYER_STEP 1

//Structs
struct key_pres
{
	int button_pressed;
	int button_num;
} key_press_size, key_press_player, key_press_keyboard; //Drei variablen, damit diese nicht Daten untereinadner austauschen, falls etwas schiefgeht.

//Tasks
void ball_logic(void *pvParameters);
void player_logic(void *pvParameters);

//Variables

//BALL
float fVelocityX;
float fVelocityY;
float fPositionX;
float fPositionY;

//PLAYERS

//Links ist Spieler 1
const int player1 = 1;
const int fPositionPlayer1X;
const int fWidthPlayer1X;
float fPositionPlayer1Y;
const float fWidthPlayer1Y;

//Rechts ist Spieler 2
const int player2 = 2;
const int fPositionPlayer2X;
const int fWidthPlayer2X;
float fPositionPlayer2Y;
const float fWidthPlayer2Y;

//INTERPROCESS COMMUNICATION
//WARNING: EVERY ACTION WITH THESE NEEDS TO BE SECURED
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
	oled_init();

	//Setup Semaphores etc.
	xDraw_screen = xSemaphoreCreateBinary();
	xScore_Update = xQueueCreate(2, sizeof(int)); //Könnte Speicheroptimierung vertragen, da man für Zwei Spieler nur einen Byte braucht, aber er hat mich nicht machen lassen
	xScore_Update_Mutex =xSemaphoreCreateMutex();
	xKey_Queue = xQueueCreate(10, sizeof(key_press_size));
	xKey_Queue_Mutex = xSemaphoreCreateMutex();
	
	if(xDraw_screen == NULL || xScore_Update == NULL || xScore_Update_Mutex == NULL || xKey_Queue == NULL || xKey_Queue_Mutex == NULL){ //Kritischer Fehler. Programm kann ohne nicht laufen
		setEntireDisplayOn(1);
	}

	//TODO Tasks initialisieren

	vTaskStartScheduler();

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
			//Ressource konnte nicht genommen werden. Fehler, aber nicht schlimm, wenn der Bildschirm nicht aktualisiert wurde.
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
				case 2:
					green_pressed = key_press_player.button_pressed;
					break;
				case 3: 
					blue_pressed = key_press_player.button_pressed;
					break;
				case 4:
					yellow_pressed = key_press_player.button_pressed;
					break;
				case 5:
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
