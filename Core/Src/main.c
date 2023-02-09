/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LiquidCrystal.h"
#include "event_groups.h"
#include <time.h>
#include "stdio.h"
#include <string.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc2;

I2C_HandleTypeDef hi2c1;

RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

PCD_HandleTypeDef hpcd_USB_FS;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = { .name = "defaultTask",
		.stack_size = 128 * 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for LCDTasl */
osThreadId_t LCDTaslHandle;
const osThreadAttr_t LCDTasl_attributes = { .name = "LCDTasl", .stack_size = 128
		* 4, .priority = (osPriority_t) osPriorityNormal, };
/* Definitions for PlayerTask */
osThreadId_t PlayerTaskHandle;
const osThreadAttr_t PlayerTask_attributes =
		{ .name = "PlayerTask", .stack_size = 128 * 4, .priority =
				(osPriority_t) osPriorityBelowNormal6, };
/* Definitions for EnemyTask */
osThreadId_t EnemyTaskHandle;
const osThreadAttr_t EnemyTask_attributes = { .name = "EnemyTask", .stack_size =
		128 * 4, .priority = (osPriority_t) osPriorityBelowNormal7, };
/* Definitions for BulletPTask */
osThreadId_t BulletPTaskHandle;
const osThreadAttr_t BulletPTask_attributes = { .name = "BulletPTask",
		.stack_size = 128 * 4, .priority = (osPriority_t) osPriorityLow, };
/* Definitions for BulletETask */
osThreadId_t BulletETaskHandle;
const osThreadAttr_t BulletETask_attributes = { .name = "BulletETask",
		.stack_size = 128 * 4, .priority = (osPriority_t) osPriorityLow, };
/* Definitions for AboutTask */
osThreadId_t AboutTaskHandle;
const osThreadAttr_t AboutTask_attributes = { .name = "AboutTask", .stack_size =
		128 * 4, .priority = (osPriority_t) osPriorityLow, };
/* Definitions for Buzzer */
osThreadId_t BuzzerHandle;
const osThreadAttr_t Buzzer_attributes = { .name = "Buzzer", .stack_size = 128
		* 4, .priority = (osPriority_t) osPriorityLow, };
/* Definitions for myEvent01 */
osEventFlagsId_t myEvent01Handle;
const osEventFlagsAttr_t myEvent01_attributes = { .name = "myEvent01" };
/* USER CODE BEGIN PV */
#define y80_max 3
#define x80_max 16
#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof((arr)[0]))
#define FlaginLanding (1U << 0U)
#define FlaginMenu (1U << 1U)
#define FlaginPlaying (1U << 2U)
#define FlaginAbout (1U << 3U)
#define FlagPlayerMoveLeft (1U << 4U)
#define FlagPlayerMoveRight (1U << 5U)
#define FlagPlayerShoot (1U << 6U)
#define FlagEnemiesMoveDown (1U << 7U)
#define FlagEnemiesMovelr (1U << 8U)
//#define FlagPlayerShoot (1U << 9U)
#define FlagEnemiesShoot (1U << 10U)
#define FlaginGetName (1U << 11U)
#define FlaginGetDifficulty (1U << 12U)
//osSemaphoreId_t semaphoreLcd;
osMutexId_t mutexLCD;
const osMutexAttr_t Thread_Mutex_attr = { "myThreadMutex", // human readable mutex name
		osMutexPrioInherit,  // attr_bits
		NULL,                // memory for control block
		0U                   // size for control block
		};
EventGroupHandle_t xEventGroup;
osTimerId_t timerPlayer;
osTimerId_t timerEnemies;
osTimerId_t timerEnemieslr;
osTimerId_t timerRandomBullet;
uint32_t timeDown;
RTC_TimeTypeDef mytime;
int x80_last = 16;
int mobkey1 = 0;
int mobkey2 = 0;
int mobkey3 = 0;
int sw = 0;
int locChar = 0;
char a[3] = { 'a', 'b', 'c' };
char b[3] = { 'd', 'e', 'f' };
char c[3] = { 'g', 'h', 'i' };
extern unsigned int normalizedVolume;
TIM_HandleTypeDef *pwm_timer = &htim2; // Point to PWM Timer configured in CubeMX
uint32_t pwm_channel = TIM_CHANNEL_2;   // Select configured PWM channel number
typedef struct {
	uint16_t frequency;
	uint16_t duration;
} Tone;
const Tone *volatile melody_ptr;
volatile uint16_t melody_tone_count;
volatile uint16_t current_tone_number;
volatile uint32_t current_tone_end;
volatile uint16_t volume = 50;          // (0 - 1000)
typedef unsigned char byte;
unsigned char data[1];
struct game mygame;
struct bullet playerBullet;
struct bullet enemiesBullet;
int score = 0;
byte enemy_bitmap[8] = { 0x00, 0x00, 0x04, 0x0E, 0x1B, 0x0E, 0x04, 0x00 };
byte enemy2_bitmap = { 0x01, 0x19, 0x0D, 0x07, 0x03, 0x07, 0x0D, 0x19 };
byte player_bitmap[8] = { 0x00, 0x10, 0x18, 0x1C, 0x18, 0x10, 0x00, 0x00 };

byte bulletp_bitmap[8] = { 0x00, 0x00, 0x00, 0x02, 0x0F, 0x02, 0x00, 0x00 };
byte bullete_bitmap[8] = { 0x00, 0x00, 0x00, 0x0E, 0x0E, 0x0E, 0x00, 0x00 };
byte white_bitmap[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
byte hp_bitmap[8] = { 0x00, 0x00, 0x0C, 0x1E, 0x0F, 0x1E, 0x0C, 0x00 };
byte ship_first_display_bitmap[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x04, 0x0E,
		0x1F };
byte boss4[8] = { 0x0A, 0x0A, 0x08, 0x0F, 0x18, 0x10, 0x00, 0x00 };

byte boss3[8] = { 0x00, 0x00, 0x10, 0x18, 0x0F, 0x08, 0x0A, 0x0A };

byte boss1[8] = { 0x03, 0x0B, 0x02, 0x1F, 0x03, 0x01, 0x00, 0x00 };

byte boss2[8] = { 0x00, 0x00, 0x01, 0x03, 0x1F, 0x02, 0x0B, 0x03 };
volatile uint32_t current_tone_end;
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0
int game_of_throne[] = {

NOTE_E5, 8, NOTE_E5, 8, REST, 8, NOTE_E5, 8, REST, 8, NOTE_C5, 8, NOTE_E5,
		8, //1
		NOTE_G5, 4, REST, 4, NOTE_G4, 8, REST, 4,
		NOTE_C5, -4, NOTE_G4, 8, REST, 4, NOTE_E4,
		-4, // 3
		NOTE_A4, 4, NOTE_B4, 4, NOTE_AS4, 8, NOTE_A4, 4,
		NOTE_G4, -8, NOTE_E5, -8, NOTE_G5, -8, NOTE_A5, 4, NOTE_F5, 8, NOTE_G5,
		8,
		REST, 8, NOTE_E5, 4, NOTE_C5, 8, NOTE_D5, 8, NOTE_B4, -4,
		NOTE_C5, -4, NOTE_G4, 8, REST, 4, NOTE_E4,
		-4, // repeats from 3
		NOTE_A4, 4, NOTE_B4, 4, NOTE_AS4, 8, NOTE_A4, 4,
		NOTE_G4, -8, NOTE_E5, -8, NOTE_G5, -8, NOTE_A5, 4, NOTE_F5, 8, NOTE_G5,
		8,
		REST, 8, NOTE_E5, 4, NOTE_C5, 8, NOTE_D5, 8, NOTE_B4, -4, };

int fire_note[] = {
NOTE_G4, 8, NOTE_G5, 4, NOTE_G6, 8, NOTE_G4, -4, NOTE_G5, 8, NOTE_G6, -4 };
int killed_note[] = {

NOTE_B5, 8, NOTE_E6, 4, RESET, 8,
NOTE_B5, 4, NOTE_E6, -4, RESET, 8 };

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_SPI1_Init(void);
static void MX_USB_PCD_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC2_Init(void);
static void MX_TIM2_Init(void);
static void MX_RTC_Init(void);
void StartDefaultTask(void *argument);
void StartTaskLCD(void *argument);
void StartTaskPlayer(void *argument);
void StartTaskEnemy(void *argument);
void StartTaskBulletP(void *argument);
void StartTaskBulletE(void *argument);
void StartTaskAbout(void *argument);
void StartTaskBuzzer(void *argument);

/* USER CODE BEGIN PFP */
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int _write(int file, char *ptr, int len) {
	/* Implement your write code here, this is used by puts and printf for example */
	int i = 0;
	for (i = 0; i < len; i++)
		ITM_SendChar((*ptr++));
	return len;
}

// ye x80 y80 vase khoone haye too boarde 20*4
//va ye x40 y40 baraye pixel haye dakhele har khoone
enum enemyType {
	normalEnemy = 1, advanceEnemy = 2, boss = 3
};
enum playerType {
	normalPlayer = 1, advancePlayer = 2
};
enum shootType {
	normalShoot = 1, taghe = 2
};
enum difficulty {
	easy = 0, normal = 1, hard = 2
};
enum buzzerState {
	first, shoot, killed
};
enum buzzerState buzzState = first;
enum gameState {
	inMenu = 0,
	inAbout = 1,
	inPlaying = 3,
	inBossFight = 4,
	inWin = 5,
	inLoose = 6,
	inGetName = 7,
	inGetDifficulty = 8,
	inLanding = 9
};

struct enemy {
	int index;
	enum enemyType type;
	int hp;
	int isAlive;
	int x80; // 0- 19
	int y80; // 0 - 3
};

struct enemy temp[3];
void createEnemies() {

	for (int var = 0; var < y80_max; ++var) {
		temp[var].hp = 1;
		temp[var].index = var;
		temp[var].isAlive = 1;
		temp[var].type = normalEnemy;
		temp[var].x80 = 15;
		temp[var].y80 = var;
	}
//	return &temp[0];
}
void decreaseEnemyHp(struct enemy *e, int i) {
	if (e[i].hp > 1) {
		e[i].hp--;
		sentMSG3();
	} else {
		e[i].isAlive = 0;
		buzzState = killed;
//		byte boom2[8] = { 0x00, 0x00, 0x1F, 0x11, 0x15, 0x11, 0x1F, 0x00 };
//		byte boom1[8] = { 0x00, 0x00, 0x00, 0x0E, 0x0E, 0x0E, 0x00, 0x00 };
//		byte boom3[8] = { 0x00, 0x00, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x00 };
////		createCharSem(3, white_bitmap);
//		createCharSem(10, boom1);
//		createCharSem(11, boom2);
//		createCharSem(12, boom3);
//
//		setCursorSem(e[i].x80, e[i].y80);
//		writeSem(3);
//		osDelay(30);
//		setCursorSem(e[i].x80, e[i].y80);
//		writeSem(9);
//		osDelay(20);
//		setCursorSem(e[i].x80, e[i].y80);
//		writeSem(10);
//		setCursorSem(e[i].x80, e[i].y80);
//		writeSem(9);
//		osDelay(20);
//		setCursorSem(e[i].x80, e[i].y80);
//		writeSem(11);
//		osDelay(20);
//		setCursorSem(e[i].x80, e[i].y80);
//		writeSem(9);
//		osDelay(20);
//		setCursorSem(e[i].x80, e[i].y80);
//		writeSem(12);
//		osDelay(20);
//		setCursorSem(e[i].x80, e[i].y80);
//		writeSem(9);
//		osDelay(20);
	}
}
void EnemiesMoveRight(struct enemy *e) {
	for (int var = 0; var < y80_max; ++var) {
		if (e[var].y80 != y80_max)
			e[var].y80++;
		else
			e[var].y80 = 0;
	}
}
void enemiesMoveLeft(struct enemy *e) {
	for (int var = 0; var < y80_max; ++var) {
		if (e[var].y80 != 0)
			e[var].y80--;
		else
			e[var].y80 = y80_max;
	}
}
void enemiesMovelr(struct enemy *e) {
	if (e[0].y80 == 0) {
		EnemiesMoveRight(e);
	} else {
		enemiesMoveLeft(e);
	}
}

void enemiesMoveDown(struct enemy *e) {
	for (int var = 0; var < y80_max; ++var) {
		e[var].x80--;
		x80_last = e[var].x80;
	}
}

void enemyShootRandom() {
	//todo
}

int addScore(enum enemyType t) {
	switch (t) {
	case normalEnemy:
		return 10;
		break;
	case advanceEnemy:
		return 50;
		break;
	case boss:
		return 100;
		break;
	default:
		return 0;
		break;
	}

}
// - -- - - - -- - -
int counter = 0;
struct player {
	enum playerType type;
	enum shootType stype;
	unsigned char name[10];
	int hp;
	int isAlive;
	int x80;
	int y80;
//	enum difficulty diff;
};
void decreasePlyaerHP(struct player *p) {
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, 1);
	if (p->hp > 1) {
		p->hp--;
		showHP();
		if (p->hp < 2) {
			sentMSG2();
		}
	} else {
		buzzState = killed;
		p->isAlive = 0;
	}
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, 0);
}

void printPlayer(struct player p) {
	if (p.isAlive) {
		createCharSem(1, player_bitmap);
		setCursorSem(p.x80, p.y80);
		writeSem(1);
	}
}
void clearPlayer(struct player p) {
	createCharSem(3, white_bitmap);
	setCursorSem(p.x80, p.y80);
	writeSem(3);
}

void playerMoveLeft(struct player *p) {

	if (p->y80 != 0)
		p->y80--;
	else
		p->y80 = y80_max;
}

void playerMoveRight(struct player *p) {
	if (p->y80 != y80_max)
		p->y80++;
	else
		p->y80 = 0;
}

// --------------------- - - - -- - - -

struct bullet {
	byte bullet_bitmap[8];
	int x80;
	int y80;
	int isAlive;
};

void clearBullet(struct bullet *b) {
	createCharSem(3, white_bitmap);
	setCursorSem(b->x80, b->y80);
	writeSem(3);
}
void printBulletP() {
	createCharSem(4, bulletp_bitmap);
	setCursorSem(playerBullet.x80, playerBullet.y80);
	writeSem(4);
}
void printBulletE() {
	createCharSem(5, bullete_bitmap);
	setCursorSem(enemiesBullet.x80, enemiesBullet.y80);
	writeSem(5);

}
void moveBulletP(struct bullet *bp) {
	osStatus_t val;
	val = osMutexAcquire(mutexLCD, 20U); // wait for max. 10 ticks for semaphore token to get available
	switch (val) {
	case osOK:
		bp->x80++;
		osMutexRelease(mutexLCD); // return a token back to a semaphore
		break;
	default:
		break;
	}

}
void moveBulletE(struct bullet *be) {
	osStatus_t val;
	val = osMutexAcquire(mutexLCD, 20U); // wait for max. 10 ticks for semaphore token to get available
	switch (val) {
	case osOK:
		be->x80--;
		osMutexRelease(mutexLCD); // return a token back to a semaphore
		break;
	default:
		break;
	}
}

int checkEnemiesPlayerCollision(struct enemy *e, struct player *p) {
	for (int i = 0; i < 3; i++) {
		if (e[i].x80 == 0) {

			decreasePlyaerHP(p);
			e[i].isAlive = 0;

			return 1;
		}
	}
	return 0;

}
int checkbulletsCollision(struct bullet *be, struct bullet *bp) {
	if (bp->x80 == be->x80 && bp->y80 == be->y80) {
		bp->isAlive = 0;
		be->isAlive = 0;

		return 1;
	}
	return 0;
}
int checkbulletPCollision(struct enemy *e, struct bullet *b) {
	for (int i = 0; i < 3; i++) {
		if (e[i].x80 == b->x80 && e[i].y80 == b->y80) {

			decreaseEnemyHp(e, i);
			b->isAlive = 0;
			score += addScore(e[i].type);
			return 1;
		}
	}
	return 0;
}
int checkbulletECollision(struct player *p, struct bullet *b) {
	if (p->x80 == b->x80 && p->y80 == b->y80) {
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, 1);
		decreasePlyaerHP(p);
		b->isAlive = 0;
//		sentMSG5();
		return 1;
	}
	HAL_GPIO_WritePin(GPIOE, GPIO_PIN_15, 0);
	return 0;
}
//- - - - -- - - - - -
struct game {
	struct enemy enemies[3];
	struct player player;
	enum gameState state;
	int killsToWin;
	enum difficulty diff;
};

void initGame(enum difficulty d, struct player p, int killsToWin,
		enum gameState s, struct enemy e[3]) {
	mygame.enemies[0] = e[0];
	mygame.enemies[1] = e[1];
	mygame.enemies[2] = e[2]; // in bayad dynamic she
	mygame.player = p;
	mygame.state = s;
	mygame.killsToWin = killsToWin;
	mygame.diff = d;
}

// -- - - -- - - -- -  general functions   -- - - - - --

void writeSem(uint8_t value) {
	osStatus_t val;
	val = osMutexAcquire(mutexLCD, 30U); // wait for max. 10 ticks for semaphore token to get available
	switch (val) {
	case osOK:
		write(value);
		osMutexRelease(mutexLCD); // return a token back to a semaphore
		break;
	case osErrorResource:
		break;
	case osErrorParameter:
		break;
	default:
		break;
	}
}
void createCharSem(uint8_t location, uint8_t charmap[]) {
	osStatus_t val;
	val = osMutexAcquire(mutexLCD, 30U); // wait for max. 10 ticks for semaphore token to get available
	switch (val) {
	case osOK:
		createChar(location, charmap);
		osMutexRelease(mutexLCD); // return a token back to a semaphore
		break;
	case osErrorResource:
		break;
	case osErrorParameter:
		break;
	default:
		break;
	}

}
void printSem(const char str[]) {
	osStatus_t val;
	val = osMutexAcquire(mutexLCD, 30U); // wait for max. 10 ticks for semaphore token to get available
	switch (val) {
	case osOK:
		print(str);
		osMutexRelease(mutexLCD); // return a token back to a semaphore
		break;
	case osErrorResource:
		break;
	case osErrorParameter:
		break;
	default:
		break;
	}
}
void setCursorSem(uint8_t col, uint8_t row) {
	osStatus_t val;
	val = osMutexAcquire(mutexLCD, 30U); // wait for max. 10 ticks for semaphore token to get available
	switch (val) {
	case osOK:
		setCursor(col, row);
		osMutexRelease(mutexLCD); // return a token back to a semaphore
		break;
	case osErrorResource:
		break;
	case osErrorParameter:
		break;
	default:
		break;
	}

}
void clearSem() {
	osStatus_t val;
	val = osMutexAcquire(mutexLCD, 20U); // wait for max. 10 ticks for semaphore token to get available
	switch (val) {
	case osOK:
		clear(); // it takes 2sec minimum to be completed
		osMutexRelease(mutexLCD); // return a token back to a semaphore
		break;
	case osErrorResource:
		break;
	case osErrorParameter:
		break;
	default:
		break;
	}
}

void printEnemies(struct game *g) {
	for (int j = 0; j < 3; ++j) {
		if (g->enemies[j].isAlive) {
			createCharSem(0, enemy_bitmap);
			setCursor(g->enemies[j].x80, g->enemies[j].y80);
			write(0);
		}

	}
}
void clearEnemies(struct game *g) {
	for (int j = 0; j < 3; ++j) {
		createCharSem(3, white_bitmap);
		setCursor(g->enemies[j].x80, g->enemies[j].y80);
		write(3);
	}
}
void showHP() {
	setCursorSem(18, 3);
	printSem("hp");
	for (int i = 0; i < mygame.player.hp; ++i) {
		createCharSem(3, white_bitmap);
		setCursorSem(18, i);
		writeSem(3);
		createCharSem(6, hp_bitmap);
		setCursorSem(18, i);
		writeSem(6);
	}
}
void showLanding() {
	createCharSem(0, enemy_bitmap);
	createCharSem(1, player_bitmap);
	createCharSem(2, ship_first_display_bitmap);

	setCursorSem(3, 0);
	printSem("space invaders !");

	setCursorSem(1, 1);
	writeSem(0);
	setCursorSem(3, 1);
	writeSem(0);
	setCursorSem(5, 1);
	writeSem(0);
	setCursorSem(7, 1);
	writeSem(0);
	setCursorSem(9, 1);
	writeSem(0);
	setCursorSem(11, 1);
	writeSem(0);
	setCursorSem(13, 1);
	writeSem(0);
	setCursorSem(15, 1);
	writeSem(0);
	setCursorSem(17, 1);
	writeSem(0);

	setCursorSem(10, 3);
	writeSem(2);
}

void showMenu() {
	clearSem();
	setCursorSem(5, 0);
	printSem("8.start");
	setCursorSem(5, 2);
	printSem("5.about");
}

void showGetName() {

	unsigned char data[10] = "";
//	HAL_UART_Receive_IT(&huart2, data, sizeof(data));
	HAL_UART_Transmit(&huart2, data, sizeof(data), 100);
//	mygame.player.name = data;
	strcpy(mygame.player.name, data);
	clearSem();
	setCursorSem(2, 0);
	printSem("enter your name : ");
	setCursorSem(3, 1);
	printSem(data);

}

void startGame() {
//	osThreadSuspend(BuzzerHandle);
	PWM_Change_Tone(4000, 0);
	initGame(mygame.diff, mygame.player, mygame.killsToWin, mygame.state,
			mygame.enemies);
//	uint8_t message[] = "a";
//	sprintf(message,"name= %s killsToWin= %d , difficulty= %d",mygame.player.name,mygame.killsToWin,mygame.diff);
//	HAL_UART_Transmit(&huart2, message, sizeof(message), 80);
	clearSem();
	printPlayer(mygame.player);
	printEnemies(&mygame);
	showHP();
//	osTimerStart(timerPlayer, 1000U);
	osTimerStart(timerEnemies, timeDown);
	osTimerStart(timerEnemieslr, 3000U);
	osTimerStart(timerRandomBullet, 4000U);
}

void stopGame() {
	osTimerStop(timerPlayer);
	osTimerStop(timerEnemies);
	osTimerStop(timerEnemieslr);
	osTimerStop(timerRandomBullet);
//	osThreadSuspend(BulletPTaskHandle);

}

void showAbout() {
	clearSem();
	setCursorSem(2, 0);
	printSem("Soroush--Behnam");
	showCurrentTime();
}

int checkWin() {
	for (int i = 0; i < 3; ++i) {
		if (mygame.enemies[i].isAlive) {
			return 0;
		}
	}
	return 1;
}
void showWin() {
	clearSem();
	setCursorSem(3, 3);
	printSem("you win!!");
	setCursorSem(3, 2);
	printSem(mygame.player.name);
	setCursorSem(3, 1);
	char msg[10];
	sprintf(msg, "%02d", score);
	printSem("score: ");
	setCursorSem(3, 0);
	printSem(msg);

}
int checkLoose() {
	return !mygame.player.isAlive;
}
void showLoose() {
	clearSem();
	setCursorSem(3, 3);
	printSem("game over!!");
}

void showCurrentTime() {
//	HAL_RTC_GetTime(&hrtc, &mytime, RTC_FORMAT_BIN);
	uint32_t currentTime = HAL_GetTick();
	// Convert to readable format
	struct tm *localTime = localtime(&currentTime); // or gmtime(&currentTime) for GMT/UTC time
	setCursorSem(5, 2);
	char *msg = (char*) malloc(20 * sizeof(char));
	// Prints "Hello world!" on hello_world
	sprintf(msg, "%02d:%02d:%02d\n", localTime->tm_hour, localTime->tm_min,
			localTime->tm_sec);
//	printf("Current Time: %02d:%02d:%02d\n", localTime->tm_hour, localTime->tm_min, localTime->tm_sec);
	printSem(msg);
}

enum difficulty getDifficulty() {
	enum difficulty d = normal;
	return d;
}
void showGetDifficulty() {
	clearSem();
	setCursor(4, 0);
	printSem("6. easy");
	setCursor(4, 1);
	printSem("9. normal");
	setCursor(4, 2);
	printSem("#. hard");
}
void sentMSG1() {
	uint8_t start_message[] = "HARD mode \n TARGET: destroy 25 enemy";
	HAL_UART_Transmit(&huart2, start_message, sizeof(start_message), 80);
//	osDelay(1000);
}
void sentMSG2() {
	uint8_t player_warning_message[] = "WARNING! \n your hp is low";
	HAL_UART_Transmit(&huart2, player_warning_message,
			sizeof(player_warning_message), 80);
//	osDelay(1000);
}
void sentMSG3() {
	uint8_t close_win_message[] = "LETS GO \n 3 enemies remaining";
	HAL_UART_Transmit(&huart2, close_win_message, sizeof(close_win_message),
			80);
//	osDelay(1000);
}
void sentMSG4() {
	uint8_t win_message[] = "VICTORY! \n ";
	HAL_UART_Transmit(&huart2, win_message, sizeof(win_message), 80);
//	osDelay(1000);
}
void sentMSG5() {
	uint8_t loose_message[] = "GAME OVER! \n ";
	HAL_UART_Transmit(&huart2, loose_message, sizeof(loose_message), 80);
//	osDelay(1000);
}

static void playerTimer_Callback(void *argument) {
//	srand(time(NULL)); // seed the random number generator
	int r = rand() % 4; // generate a random number between 0 and 3
	switch (r) {
	case 0:
		xEventGroupSetBits(xEventGroup, FlagPlayerMoveLeft);
		break;
	case 1:
		xEventGroupSetBits(xEventGroup, FlagPlayerMoveRight);
		break;
	case 2:
		if (!playerBullet.isAlive) {
			xEventGroupSetBits(xEventGroup, FlagPlayerShoot);
		}

		break;
	default:
		xEventGroupSetBits(xEventGroup, FlagPlayerMoveRight);
		break;
	}

}
static void enemiestimer_Callback(void *argument) {
	xEventGroupSetBits(xEventGroup, FlagEnemiesMoveDown);
}

static void enemiestimerlr_Callback(void *argument) {
	xEventGroupSetBits(xEventGroup, FlagEnemiesMovelr);
}

static void randomBulletTimer_Callback(void *argument) {
	if (enemiesBullet.isAlive == 0) {
		srand(time(NULL)); // seed the random number generator
		int r = rand() % 5; // generate a random number between 0 and 4
		enemiesBullet.x80 = mygame.enemies[0].x80 - 2;
		enemiesBullet.y80 = r;
		xEventGroupSetBits(xEventGroup, FlagEnemiesShoot);
	}

}

void PWM_Start() {
	HAL_TIM_PWM_Start(pwm_timer, pwm_channel);
}

void PWM_Change_Tone(uint16_t pwm_freq, uint16_t volume) // pwm_freq (1 - 20000), volume (0 - 1000)
{
	if (pwm_freq == 0 || pwm_freq > 20000) {
		__HAL_TIM_SET_COMPARE(pwm_timer, pwm_channel, 0);
	} else {
		const uint32_t internal_clock_freq = HAL_RCC_GetSysClockFreq();
		const uint16_t prescaler = 1 + internal_clock_freq / pwm_freq / 60000;
		const uint32_t timer_clock = internal_clock_freq / prescaler;
		const uint32_t period_cycles = timer_clock / pwm_freq;
		const uint32_t pulse_width = normalizedVolume * period_cycles / 1000
				/ 2;

		pwm_timer->Instance->PSC = prescaler - 1;
		pwm_timer->Instance->ARR = period_cycles - 1;
		pwm_timer->Instance->EGR = TIM_EGR_UG;
		__HAL_TIM_SET_COMPARE(pwm_timer, pwm_channel, pulse_width); // pwm_timer->Instance->CCR2 = pulse_width;
	}
}

void Change_Melody(const Tone *melody, uint16_t tone_count) {
	melody_ptr = melody;
	melody_tone_count = tone_count;
	current_tone_number = 0;
}
void setup_melody(int melody[], int size_arr) {
	int tempo = 180;
	int notes = size_arr / sizeof(melody[0]) / 2;
	int wholenote = (60000 * 4) / tempo;
	int divider = 0, noteDuration = 0;

	PWM_Start();
	for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
		divider = melody[thisNote + 1];
		if (divider > 0) {
			noteDuration = (wholenote) / divider;
		} else if (divider < 0) {
			noteDuration = (wholenote) / abs(divider);
			noteDuration *= 1.5;
		}

		PWM_Change_Tone(melody[thisNote], normalizedVolume);

		osDelay(noteDuration);

		PWM_Change_Tone(melody[thisNote], 0);
	}
}
void Update_Melody() {
	if ((HAL_GetTick() > current_tone_end)
			&& (current_tone_number < melody_tone_count)) {
		const Tone active_tone = *(melody_ptr + current_tone_number);
		PWM_Change_Tone(active_tone.frequency, normalizedVolume);
		current_tone_end = HAL_GetTick() + active_tone.duration;
		current_tone_number++;
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	if (GPIO_Pin == GPIO_PIN_10) {
		createCharSem(0, enemy_bitmap);
		createCharSem(1, player_bitmap);
//		createCharSem(2, shot_bitmap);
		int j = 0;
		for (int i = 0; i < 4; i++) {
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);

			if (i == 0) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)) {

					// 1 move left
					if (mygame.state == inPlaying) {

						xEventGroupSetBitsFromISR(xEventGroup,
								FlagPlayerMoveLeft, pdFALSE);
					}

				}
			}
			if (i == 1) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)) {

					if (playerBullet.isAlive == 0) {
						xEventGroupSetBitsFromISR(xEventGroup, FlagPlayerShoot,
								pdFALSE);
					}

				}
			}
			if (i == 2) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)) {
					//7 move right
//					playerMoveRight(&mygame.player);
					xEventGroupSetBitsFromISR(xEventGroup, FlagPlayerMoveRight,
							pdFALSE);

				}
			}
			if (i == 3) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10)) {

					mygame.state = inMenu;
					xEventGroupSetBitsFromISR(xEventGroup, FlaginMenu, pdFALSE);
				}
			}
			osDelay(10);

		}

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
		while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10))
			;
	}

	if (GPIO_Pin == GPIO_PIN_11) {
		createCharSem(0, enemy_bitmap);
		createCharSem(1, player_bitmap);
//		createCharSem(2, shot_bitmap);
		int j = 0;
		for (int i = 0; i < 4; i++) {
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);

			if (i == 0) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)) {
					//2
				}
			}
			if (i == 1) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)) {
//5

					mygame.state = inAbout;
					xEventGroupSetBitsFromISR(xEventGroup, FlaginAbout,
							pdFALSE);

				}
			}
			if (i == 2) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)) {
					//8

					mygame.state = inGetName;

					xEventGroupSetBitsFromISR(xEventGroup, FlaginGetName,
							pdFALSE);
				}
			}
			if (i == 3) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11)) {
//0
				}
			}
			osDelay(10);

		}

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
		while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11))
			;
	}

	if (GPIO_Pin == GPIO_PIN_12) {
		createCharSem(0, enemy_bitmap);
		createCharSem(1, player_bitmap);
//		createCharSem(2, shot_bitmap);
		int j = 0;
		for (int i = 0; i < 4; i++) {
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);

			if (i == 0) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)) {
					// 3 ok name
					mygame.state = inGetDifficulty;
					xEventGroupSetBitsFromISR(xEventGroup, FlaginGetDifficulty,
							pdFALSE);

				}
			}
			if (i == 1) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)) {
					//6 ez
					if (mygame.state == inGetDifficulty) {
						mygame.state = inPlaying;
						mygame.diff = easy;
						mygame.killsToWin = 3;
						mygame.player.hp = 7;
						timeDown = 12000U;
						xEventGroupSetBitsFromISR(xEventGroup, FlaginPlaying,
								pdFALSE);
					}

				}
			}
			if (i == 2) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)) {
					//9  normal
					if (mygame.state == inGetDifficulty) {
						mygame.state = inPlaying;
						mygame.diff = normal;
						mygame.killsToWin = 6;
						mygame.player.hp = 5;
						timeDown = 8000U;
						xEventGroupSetBitsFromISR(xEventGroup, FlaginPlaying,
								pdFALSE);
					}
				}
			}
			if (i == 3) {

				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12)) {
					//# hard
					if (mygame.state == inGetDifficulty) {
						mygame.diff = hard;
						mygame.state = inPlaying;
						mygame.killsToWin = 9;
						mygame.player.hp = 3;
						timeDown = 4000U;
						xEventGroupSetBitsFromISR(xEventGroup, FlaginPlaying,
								pdFALSE);
					}
				}
			}
			osDelay(10);

		}

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
		while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12))
			;
	}
	if (GPIO_Pin == GPIO_PIN_13) {

		int j = 0;
		for (int i = 0; i < 4; i++) {
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);

			if (i == 0) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)) {
					// A
					if (mobkey1 < 2)
						mobkey1++;
					else
						mobkey1 = 0;
					sw = 1;
				}
			}
			if (i == 1) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)) {
					//B ez
					if (mobkey2 < 2)
						mobkey2++;
					else
						mobkey2 = 0;
					sw = 2;
				}
			}
			if (i == 2) {
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)) {
					//C  normal
					if (mobkey3 < 2)
						mobkey3++;
					else
						mobkey3 = 0;
					sw = 3;
				}
			}
			if (i == 3) {

				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
				if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13)) {
					//# D
					setCursor(4 + locChar, 2);
					char msg[10];
					locChar++;
					switch (sw) {
					case 1:
						sprintf(msg, "%c", a[mobkey1]);
						print(msg);
						strcat(mygame.player.name, msg);
						break;
					case 2:
						sprintf(msg, "%c", b[mobkey2]);
						print(msg);
						strcat(mygame.player.name, msg);
						break;
					case 3:
						sprintf(msg, "%c", c[mobkey3]);
						print(msg);
						strcat(mygame.player.name, msg);
						break;
					default:
						break;
					}
				}
			}
			osDelay(10);

		}

		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
		HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
		while (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13))
			;
	}
}

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_I2C1_Init();
	MX_SPI1_Init();
	MX_USB_PCD_Init();
	MX_USART2_UART_Init();
	MX_ADC2_Init();
	MX_TIM2_Init();
	MX_RTC_Init();
	/* USER CODE BEGIN 2 */

	HAL_ADC_Start_IT(&hadc2);
	//HAL_TIM_Base_Start_IT(&htim2);

	HAL_UART_Receive_IT(&huart2, data, sizeof(data));
	uint8_t message[] = "start logging ... \n";
	HAL_UART_Transmit(&huart2, message, sizeof(message), 80);

	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);

	mytime.Hours = 10;
	mytime.Minutes = 20;
	mytime.Seconds = 30;

	HAL_RTC_SetTime(&hrtc, &mytime, RTC_FORMAT_BIN);

	struct player myplayer;
	myplayer.hp = 2;
	myplayer.isAlive = 1;
	myplayer.stype = taghe;
	myplayer.x80 = 0;
	myplayer.y80 = 0;
	myplayer.type = normalPlayer;

//	myplayer.name = "soroush";
	xEventGroup = xEventGroupCreate();
	createEnemies();
	playerBullet.bullet_bitmap[0] = &bulletp_bitmap;
	playerBullet.x80 = 0;
	playerBullet.y80 = 0;
	playerBullet.isAlive = 0;
	enemiesBullet.bullet_bitmap[0] = &bullete_bitmap;
	enemiesBullet.x80 = x80_max;
	enemiesBullet.y80 = y80_max;
	enemiesBullet.isAlive = 0;
	initGame(normal, myplayer, 4, inLanding, temp);
	xEventGroupSetBits(xEventGroup, FlaginLanding);
	/* USER CODE END 2 */

	/* Init scheduler */
	osKernelInitialize();

	/* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
	/* USER CODE END RTOS_MUTEX */

	/* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
	/* USER CODE END RTOS_SEMAPHORES */

	/* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
	/* USER CODE END RTOS_TIMERS */

	/* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
	/* USER CODE END RTOS_QUEUES */

	/* Create the thread(s) */
	/* creation of defaultTask */
	defaultTaskHandle = osThreadNew(StartDefaultTask, NULL,
			&defaultTask_attributes);

	/* creation of LCDTasl */
	LCDTaslHandle = osThreadNew(StartTaskLCD, NULL, &LCDTasl_attributes);

	/* creation of PlayerTask */
	PlayerTaskHandle = osThreadNew(StartTaskPlayer, NULL,
			&PlayerTask_attributes);

	/* creation of EnemyTask */
	EnemyTaskHandle = osThreadNew(StartTaskEnemy, NULL, &EnemyTask_attributes);

	/* creation of BulletPTask */
	BulletPTaskHandle = osThreadNew(StartTaskBulletP, NULL,
			&BulletPTask_attributes);

	/* creation of BulletETask */
	BulletETaskHandle = osThreadNew(StartTaskBulletE, NULL,
			&BulletETask_attributes);

	/* creation of AboutTask */
	AboutTaskHandle = osThreadNew(StartTaskAbout, NULL, &AboutTask_attributes);

	/* creation of Buzzer */
	BuzzerHandle = osThreadNew(StartTaskBuzzer, NULL, &Buzzer_attributes);

	/* USER CODE BEGIN RTOS_THREADS */
	// creates a periodic timer:
	timerPlayer = osTimerNew(playerTimer_Callback, osTimerPeriodic, (void*) 5,
	NULL); // (void*)5 is passed as an argument
	timerEnemies = osTimerNew(enemiestimer_Callback, osTimerPeriodic, (void*) 5,
	NULL); // (void*)5 is passed as an argument
	timerEnemieslr = osTimerNew(enemiestimerlr_Callback, osTimerPeriodic,
			(void*) 5,
			NULL); // (void*)5 is passed as an argument
	timerRandomBullet = osTimerNew(randomBulletTimer_Callback, osTimerPeriodic,
			(void*) 5, NULL); // (void*)5 is passed as an argument

//	semaphoreLcd = osSemaphoreNew(1U, 1U, NULL);
//	if (semaphoreLcd == NULL) {
//		; // Semaphore object not created, handle failure
//	}
	mutexLCD = osMutexNew(&Thread_Mutex_attr);
	if (mutexLCD != NULL) {
		// Mutex object created
	}
	/* add threads, ... */
	/* USER CODE END RTOS_THREADS */

	/* Create the event(s) */
	/* creation of myEvent01 */
	myEvent01Handle = osEventFlagsNew(&myEvent01_attributes);

	/* USER CODE BEGIN RTOS_EVENTS */
	/* add events, ... */

	PWM_Start();
//	Change_Melody(super_mario_bros, ARRAY_LENGTH(super_mario_bros));
	/* USER CODE END RTOS_EVENTS */

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */
	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
	RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
	RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };
	RCC_PeriphCLKInitTypeDef PeriphClkInit = { 0 };

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI
			| RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.LSIState = RCC_LSI_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
		Error_Handler();
	}
	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
			| RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
		Error_Handler();
	}
	PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB
			| RCC_PERIPHCLK_USART2 | RCC_PERIPHCLK_I2C1 | RCC_PERIPHCLK_RTC
			| RCC_PERIPHCLK_ADC12;
	PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
	PeriphClkInit.Adc12ClockSelection = RCC_ADC12PLLCLK_DIV1;
	PeriphClkInit.I2c1ClockSelection = RCC_I2C1CLKSOURCE_HSI;
	PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
	PeriphClkInit.USBClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
	if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
		Error_Handler();
	}
}

/**
 * @brief ADC2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_ADC2_Init(void) {

	/* USER CODE BEGIN ADC2_Init 0 */

	/* USER CODE END ADC2_Init 0 */

	ADC_ChannelConfTypeDef sConfig = { 0 };

	/* USER CODE BEGIN ADC2_Init 1 */

	/* USER CODE END ADC2_Init 1 */
	/** Common config
	 */
	hadc2.Instance = ADC2;
	hadc2.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
	hadc2.Init.Resolution = ADC_RESOLUTION_12B;
	hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc2.Init.ContinuousConvMode = DISABLE;
	hadc2.Init.DiscontinuousConvMode = DISABLE;
	hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc2.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc2.Init.NbrOfConversion = 1;
	hadc2.Init.DMAContinuousRequests = DISABLE;
	hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	hadc2.Init.LowPowerAutoWait = DISABLE;
	hadc2.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
	if (HAL_ADC_Init(&hadc2) != HAL_OK) {
		Error_Handler();
	}
	/** Configure Regular Channel
	 */
	sConfig.Channel = ADC_CHANNEL_5;
	sConfig.Rank = ADC_REGULAR_RANK_1;
	sConfig.SingleDiff = ADC_SINGLE_ENDED;
	sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	sConfig.OffsetNumber = ADC_OFFSET_NONE;
	sConfig.Offset = 0;
	if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN ADC2_Init 2 */

	/* USER CODE END ADC2_Init 2 */

}

/**
 * @brief I2C1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_I2C1_Init(void) {

	/* USER CODE BEGIN I2C1_Init 0 */

	/* USER CODE END I2C1_Init 0 */

	/* USER CODE BEGIN I2C1_Init 1 */

	/* USER CODE END I2C1_Init 1 */
	hi2c1.Instance = I2C1;
	hi2c1.Init.Timing = 0x2000090E;
	hi2c1.Init.OwnAddress1 = 0;
	hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
	hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
	hi2c1.Init.OwnAddress2 = 0;
	hi2c1.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
	hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
	hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
	if (HAL_I2C_Init(&hi2c1) != HAL_OK) {
		Error_Handler();
	}
	/** Configure Analogue filter
	 */
	if (HAL_I2CEx_ConfigAnalogFilter(&hi2c1, I2C_ANALOGFILTER_ENABLE)
			!= HAL_OK) {
		Error_Handler();
	}
	/** Configure Digital filter
	 */
	if (HAL_I2CEx_ConfigDigitalFilter(&hi2c1, 0) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN I2C1_Init 2 */

	/* USER CODE END I2C1_Init 2 */

}

/**
 * @brief RTC Initialization Function
 * @param None
 * @retval None
 */
static void MX_RTC_Init(void) {

	/* USER CODE BEGIN RTC_Init 0 */

	/* USER CODE END RTC_Init 0 */

	/* USER CODE BEGIN RTC_Init 1 */

	/* USER CODE END RTC_Init 1 */
	/** Initialize RTC Only
	 */
	hrtc.Instance = RTC;
	hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
	hrtc.Init.AsynchPrediv = 39;
	hrtc.Init.SynchPrediv = 999;
	hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
	hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
	hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
	if (HAL_RTC_Init(&hrtc) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN RTC_Init 2 */

	/* USER CODE END RTC_Init 2 */

}

/**
 * @brief SPI1 Initialization Function
 * @param None
 * @retval None
 */
static void MX_SPI1_Init(void) {

	/* USER CODE BEGIN SPI1_Init 0 */

	/* USER CODE END SPI1_Init 0 */

	/* USER CODE BEGIN SPI1_Init 1 */

	/* USER CODE END SPI1_Init 1 */
	/* SPI1 parameter configuration*/
	hspi1.Instance = SPI1;
	hspi1.Init.Mode = SPI_MODE_MASTER;
	hspi1.Init.Direction = SPI_DIRECTION_2LINES;
	hspi1.Init.DataSize = SPI_DATASIZE_4BIT;
	hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
	hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
	hspi1.Init.NSS = SPI_NSS_SOFT;
	hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
	hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
	hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
	hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	hspi1.Init.CRCPolynomial = 7;
	hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
	hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
	if (HAL_SPI_Init(&hspi1) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN SPI1_Init 2 */

	/* USER CODE END SPI1_Init 2 */

}

/**
 * @brief TIM2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_TIM2_Init(void) {

	/* USER CODE BEGIN TIM2_Init 0 */

	/* USER CODE END TIM2_Init 0 */

	TIM_ClockConfigTypeDef sClockSourceConfig = { 0 };
	TIM_MasterConfigTypeDef sMasterConfig = { 0 };
	TIM_OC_InitTypeDef sConfigOC = { 0 };

	/* USER CODE BEGIN TIM2_Init 1 */

	/* USER CODE END TIM2_Init 1 */
	htim2.Instance = TIM2;
	htim2.Init.Prescaler = 144;
	htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim2.Init.Period = 9999;
	htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
	htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
	if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
		Error_Handler();
	}
	if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig)
			!= HAL_OK) {
		Error_Handler();
	}
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = 0;
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2)
			!= HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN TIM2_Init 2 */

	/* USER CODE END TIM2_Init 2 */
	HAL_TIM_MspPostInit(&htim2);

}

/**
 * @brief USART2 Initialization Function
 * @param None
 * @retval None
 */
static void MX_USART2_UART_Init(void) {

	/* USER CODE BEGIN USART2_Init 0 */

	/* USER CODE END USART2_Init 0 */

	/* USER CODE BEGIN USART2_Init 1 */

	/* USER CODE END USART2_Init 1 */
	huart2.Instance = USART2;
	huart2.Init.BaudRate = 9600;
	huart2.Init.WordLength = UART_WORDLENGTH_8B;
	huart2.Init.StopBits = UART_STOPBITS_1;
	huart2.Init.Parity = UART_PARITY_NONE;
	huart2.Init.Mode = UART_MODE_TX_RX;
	huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart2.Init.OverSampling = UART_OVERSAMPLING_16;
	huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart2) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USART2_Init 2 */

	/* USER CODE END USART2_Init 2 */

}

/**
 * @brief USB Initialization Function
 * @param None
 * @retval None
 */
static void MX_USB_PCD_Init(void) {

	/* USER CODE BEGIN USB_Init 0 */

	/* USER CODE END USB_Init 0 */

	/* USER CODE BEGIN USB_Init 1 */

	/* USER CODE END USB_Init 1 */
	hpcd_USB_FS.Instance = USB;
	hpcd_USB_FS.Init.dev_endpoints = 8;
	hpcd_USB_FS.Init.speed = PCD_SPEED_FULL;
	hpcd_USB_FS.Init.phy_itface = PCD_PHY_EMBEDDED;
	hpcd_USB_FS.Init.low_power_enable = DISABLE;
	hpcd_USB_FS.Init.battery_charging_enable = DISABLE;
	if (HAL_PCD_Init(&hpcd_USB_FS) != HAL_OK) {
		Error_Handler();
	}
	/* USER CODE BEGIN USB_Init 2 */

	/* USER CODE END USB_Init 2 */

}

/**
 * @brief GPIO Initialization Function
 * @param None
 * @retval None
 */
static void MX_GPIO_Init(void) {
	GPIO_InitTypeDef GPIO_InitStruct = { 0 };

	/* GPIO Ports Clock Enable */
	__HAL_RCC_GPIOE_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOF_CLK_ENABLE();
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOE,
			CS_I2C_SPI_Pin | LD4_Pin | LD3_Pin | LD5_Pin | LD7_Pin | LD9_Pin
					| LD10_Pin | LD8_Pin | LD6_Pin, GPIO_PIN_RESET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
			GPIO_PIN_RESET);

	/*Configure GPIO pins : DRDY_Pin MEMS_INT3_Pin MEMS_INT4_Pin MEMS_INT1_Pin
	 MEMS_INT2_Pin */
	GPIO_InitStruct.Pin = DRDY_Pin | MEMS_INT3_Pin | MEMS_INT4_Pin
			| MEMS_INT1_Pin | MEMS_INT2_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : CS_I2C_SPI_Pin LD4_Pin LD3_Pin LD5_Pin
	 LD7_Pin LD9_Pin LD10_Pin LD8_Pin
	 LD6_Pin */
	GPIO_InitStruct.Pin = CS_I2C_SPI_Pin | LD4_Pin | LD3_Pin | LD5_Pin | LD7_Pin
			| LD9_Pin | LD10_Pin | LD8_Pin | LD6_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

	/*Configure GPIO pins : PC0 PC1 PC2 PC3 */
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	/*Configure GPIO pin : B1_Pin */
	GPIO_InitStruct.Pin = B1_Pin;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

	/*Configure GPIO pins : PB10 PB11 PB12 PB13 */
	GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	/* EXTI interrupt init*/
	HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
	HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument) {
	/* USER CODE BEGIN 5 */
	/* Infinite loop */
	osThreadTerminate(defaultTaskHandle);
	for (;;) {
		printf("default task running \n");
		osDelay(1000);
	}
	/* USER CODE END 5 */
}

/* USER CODE BEGIN Header_StartTaskLCD */
/**
 * @brief Function implementing the LCDTasl thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskLCD */
void StartTaskLCD(void *argument) {
	/* USER CODE BEGIN StartTaskLCD */
	/* Infinite loop */
	LiquidCrystal(GPIOD, GPIO_PIN_8, GPIO_PIN_9, GPIO_PIN_10, GPIO_PIN_11,
	GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14);
	begin(20, 4);
	EventBits_t xEventGroupValue;
	const EventBits_t uxBitsToWaitFor = (FlaginLanding | FlaginMenu
			| FlaginPlaying | FlaginGetName | FlaginGetDifficulty | FlaginAbout);

	for (;;) {
		printf("lcd task running \n");
		xEventGroupValue = xEventGroupWaitBits(xEventGroup, uxBitsToWaitFor,
		pdTRUE, pdFALSE, portMAX_DELAY);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, 1);
		if ((xEventGroupValue) != 0) {
			//suspension
			osThreadSuspend(AboutTaskHandle);
//			stopGame();
		}
		if ((xEventGroupValue & FlaginLanding) != 0) {
			showLanding();
//			osThreadSuspend(KeyTaskHandle);
//			osThreadSuspend(BuzzerTaskHandle);
		} else if ((xEventGroupValue & FlaginMenu) != 0) {

			showMenu();
//			osThreadSuspend(KeyTaskHandle);
//			osThreadSuspend(BuzzerTaskHandle);
		} else if ((xEventGroupValue & FlaginGetName) != 0) {
			showGetName();
		} else if ((xEventGroupValue & FlaginGetDifficulty) != 0) {
			showGetDifficulty();
		} else if ((xEventGroupValue & FlaginPlaying) != 0) {
			startGame();
//			osThreadResume(BuzzerTaskHandle);
		} else if ((xEventGroupValue & FlaginAbout) != 0) {
			osThreadResume(AboutTaskHandle);
		}
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, 0);
		osDelay(1000);
	}
	/* USER CODE END StartTaskLCD */
}

/* USER CODE BEGIN Header_StartTaskPlayer */
/**
 * @brief Function implementing the PlayerTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskPlayer */
void StartTaskPlayer(void *argument) {
	/* USER CODE BEGIN StartTaskPlayer */
	/* Infinite loop */

	EventBits_t xEventGroupValue;
	const EventBits_t uxBitsToWaitFor = (FlagPlayerMoveLeft
			| FlagPlayerMoveRight | FlagPlayerShoot);

	for (;;) {

		xEventGroupValue = xEventGroupWaitBits(xEventGroup, uxBitsToWaitFor,
		pdTRUE, pdFALSE, portMAX_DELAY);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, 1);
		if ((xEventGroupValue & FlagPlayerMoveLeft) != 0
				&& mygame.player.isAlive) {
			clearPlayer(mygame.player);
			playerMoveLeft(&mygame.player);
			printPlayer(mygame.player);
		} else if ((xEventGroupValue & FlagPlayerMoveRight) != 0
				&& mygame.player.isAlive) {
			clearPlayer(mygame.player);
			playerMoveRight(&mygame.player);
			printPlayer(mygame.player);
		} else if ((xEventGroupValue & FlagPlayerShoot) != 0
				&& mygame.player.isAlive) {
			buzzState = shoot;
			playerBullet.x80 = mygame.player.x80 + 1;
			playerBullet.y80 = mygame.player.y80;
			playerBullet.isAlive = 1;
			osThreadResume(BulletPTaskHandle);
		}
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, 0);
//		osDelay(100);
	}
	/* USER CODE END StartTaskPlayer */
}

/* USER CODE BEGIN Header_StartTaskEnemy */
/**
 * @brief Function implementing the EnemyTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskEnemy */
void StartTaskEnemy(void *argument) {
	/* USER CODE BEGIN StartTaskEnemy */
	/* Infinite loop */
	EventBits_t xEventGroupValue;
	const EventBits_t uxBitsToWaitFor = (FlagEnemiesMoveDown | FlagEnemiesMovelr
			| FlagEnemiesShoot);
	for (;;) {
		xEventGroupValue = xEventGroupWaitBits(xEventGroup, uxBitsToWaitFor,
		pdTRUE, pdFALSE, portMAX_DELAY);
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, 1);
		if ((xEventGroupValue & FlagEnemiesMoveDown) != 0) {

			clearEnemies(&mygame);
			enemiesMoveDown(&mygame.enemies);
			printEnemies(&mygame);
			if (checkEnemiesPlayerCollision(&mygame.enemies, &mygame.player)) {
				showLoose();
				stopGame();
			}

		} else if ((xEventGroupValue & FlagEnemiesMovelr) != 0) {
			clearEnemies(&mygame);
			enemiesMovelr(&mygame.enemies);
			printEnemies(&mygame);
		} else if ((xEventGroupValue & FlagEnemiesShoot) != 0) {
			//fargh
			enemiesBullet.x80 = mygame.enemies[0].x80 + 1;
			srand(time(NULL)); // seed the random number generator
			int r = rand() % 5; // generate a random number between 0 and 4
			enemiesBullet.y80 = r;
			enemiesBullet.isAlive = 1;
			osThreadResume(BulletETaskHandle);
		}
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_12, 0);
//		osDelay(200);
	}
	/* USER CODE END StartTaskEnemy */
}

/* USER CODE BEGIN Header_StartTaskBulletP */
/**
 * @brief Function implementing the BulletPTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskBulletP */
void StartTaskBulletP(void *argument) {
	/* USER CODE BEGIN StartTaskBulletP */
	/* Infinite loop */
	osThreadSuspend(BulletPTaskHandle);
	for (;;) {
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, 1);
		//if not collided

		clearBullet(&playerBullet);
		moveBulletP(&playerBullet);
		printBulletP();

		if (checkbulletPCollision(&mygame.enemies, &playerBullet)
				|| checkbulletsCollision(&enemiesBullet, &playerBullet)
				|| playerBullet.x80 == x80_last) {
//			clearBullet(&playerBullet);

			playerBullet.isAlive = 0;
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, 0);
			if (checkWin()) {
				sentMSG4();
				stopGame();
				showWin();

//				osThreadTerminate(BulletETaskHandle);
//				osThreadTerminate(BulletPTaskHandle);
			}
			osThreadSuspend(BulletPTaskHandle);

		}
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, 0);
		osDelay(40);
	}
	/* USER CODE END StartTaskBulletP */
}

/* USER CODE BEGIN Header_StartTaskBulletE */
/**
 * @brief Function implementing the BulletETask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskBulletE */
void StartTaskBulletE(void *argument) {
	/* USER CODE BEGIN StartTaskBulletE */
	/* Infinite loop */
	osThreadSuspend(BulletETaskHandle);
	for (;;) {
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, 1);
		clearBullet(&enemiesBullet);
		moveBulletE(&enemiesBullet);
		printBulletE();
		if (checkbulletECollision(&mygame.player, &enemiesBullet)
				|| checkbulletsCollision(&enemiesBullet, &playerBullet)
				|| enemiesBullet.x80 == 0) {
			clearBullet(&enemiesBullet);
			enemiesBullet.isAlive = 0;
			HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, 0);
			if (mygame.player.isAlive == 0) {
				sentMSG5();
				stopGame();
				showLoose();
			}
			osThreadSuspend(BulletETaskHandle);
		}
		HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14, 0);
		osDelay(40);
	}
	/* USER CODE END StartTaskBulletE */
}

/* USER CODE BEGIN Header_StartTaskAbout */
/**
 * @brief Function implementing the AboutTask thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskAbout */
void StartTaskAbout(void *argument) {
	/* USER CODE BEGIN StartTaskAbout */
	/* Infinite loop */
	osThreadSuspend(AboutTaskHandle);
	for (;;) {
		showAbout();
		osDelay(1000);
	}
	/* USER CODE END StartTaskAbout */
}

/* USER CODE BEGIN Header_StartTaskBuzzer */
/**
 * @brief Function implementing the Buzzer thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_StartTaskBuzzer */
void StartTaskBuzzer(void *argument) {
	/* USER CODE BEGIN StartTaskBuzzer */
	/* Infinite loop */

	for (;;) {
//		PWM_Start();
//		Change_Melody(super_mario_bros, ARRAY_LENGTH(super_mario_bros));
		switch (buzzState) {
		case first:
			setup_melody(game_of_throne, sizeof(game_of_throne));
			break;
		case shoot:
			setup_melody(fire_note, sizeof(fire_note));
			break;
		case killed:
			setup_melody(killed_note, sizeof(killed_note));
			break;
		default:
			break;
		}

		osDelay(1000);
	}
	/* USER CODE END StartTaskBuzzer */
}

/**
 * @brief  Period elapsed callback in non blocking mode
 * @note   This function is called  when TIM1 interrupt took place, inside
 * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
 * a global variable "uwTick" used as application time base.
 * @param  htim : TIM handle
 * @retval None
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	/* USER CODE BEGIN Callback 0 */

	/* USER CODE END Callback 0 */
	if (htim->Instance == TIM1) {
		HAL_IncTick();
	}
	/* USER CODE BEGIN Callback 1 */

	/* USER CODE END Callback 1 */
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
