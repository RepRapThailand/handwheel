/*
  Internal Controler for Estlcam "CNC-Handrad"

  - copyright: Dario Carluccio 
               https://www.carluccio.de
  - history:   
    - 04.06.2018 initial release    
    - 19.06.2018 Add default Values for Relais 1-4
  - Hardware: Arduino Nano v3 
  - Purpose:
    - control LEDs of Estlcam controlled Buttons (except X,Y,Z)
      - select (cycle) modes using AUX4 Button
        - All LEDs ON (default)
        - only OK
        - PGM Start/Stop and Spindle Start/Stop        
        - All OFF
    - Toggle Relais states (only Relais 1-3)
      - press of button AUX1-3 changes State of Relais 1-3 
      - LEDs arround Buttons show Relais states
      - default all Relais off
  - Connection of GPIO:
     8 - LED OK
     9 - LED Program Start
    10 - LED Program Stop
    11 - LED Spindle Start
    12 - LED Spindle Stop
    14 - LED Aux 1
    15 - LED Aux 2
    16 - LED Aux 3
    17 - LED Aux 4
    18 - Button Aux1
    19 - Button Aux2
     2 - Button Aux3
     3 - Button Aux4
     4 - Relais 1 - Sub-D Pin 14
     5 - Relais 2 - Sub-D Pin 15
     6 - Relais 3 - Sub-D Pin 16
     7 - Relais 4 - Sub-D Pin 20 (not in use)
    13 - Aux      - Sub-D Pin 21 (not in use)
*/

/***************************
 * External Libs
 **************************/
#include <Bounce2.h>          // from: https://github.com/thomasfredericks/Bounce2

/***************************
 * GPIO Defines
 **************************/
#define LED_OK                8
#define LED_PROGRAM_START     9
#define LED_PROGRAM_STOP     10
#define LED_SPINDLE_START    11
#define LED_SPINDLE_STOP     12
#define LED_AUX_1            14
#define LED_AUX_2            15
#define LED_AUX_3            16
#define LED_AUX_4            17

#define BUTTON_AUX_1         18
#define BUTTON_AUX_2         19
#define BUTTON_AUX_3          2
#define BUTTON_AUX_4          3 

#define RELAIS_1              4
#define RELAIS_2              5
#define RELAIS_3              6
#define RELAIS_4              7

#define DEFAULT_RELAIS_1      false
#define DEFAULT_RELAIS_2      false
#define DEFAULT_RELAIS_3      false
#define DEFAULT_RELAIS_4      false

#define AUX_OUT              13 

/***************************
 * CNC LED Modi 
 * 
 * 0bsSpPO 
 *   s: Spindle Start 
 *   S: Spindle Stop  
 *   p: Program Start 
 *   P: Program Stop 
  *  O: OK
 **************************/
#define MODE_1               0b11111         // ALL
#define MODE_2               0b00001         // only OK
#define MODE_3               0b11110         // PGM Start/Stop + Spindle Start/Stop

#define SHORTDELAY           delay(50)       // Delay for starup Animation
#define LONGDELAY            delay(2000)     // Delay for starup Animation

#define NUM_RELAIS 4                         // number of relais
#define NUM_BUTTONS 4                        // number of buttons

// global vars
boolean gRelais[NUM_RELAIS];                 // state of relais
byte    gCncLedMode;                         // actual CncLedMode
Bounce  * gButton = new Bounce[NUM_BUTTONS]; // Buttons


/*************************** 
 * Set LED Status 
 * according to bitfield
 *  
 *    1234sSpPO
 *  0b000000000
 *  
 *  1-4: AUX 1 to 4      0x100, 0x080, 0x040, 0x020
 *    s: Spindle Start   0x010
 *    S: Spindle Stop    0x008
 *    p: Program Start   0x004
 *    P: Program Stop    0x002
 *    O: OK              0x001
 **************************/
void setAllLEDs(unsigned int bf) {
  byte cncleds;
  cncleds = bf & 0x1f;
  setCncLEDs(cncleds);
  digitalWrite(LED_AUX_1, (1 & (bf>>5)));    
  digitalWrite(LED_AUX_2, (1 & (bf>>6)));    
  digitalWrite(LED_AUX_3, (1 & (bf>>7)));   
  digitalWrite(LED_AUX_4, (1 & (bf>>8)));     
}

/*************************** 
 * Set LED Status 
 * of Estlcam controlled Buttons according to bitfield
 *  
 *  Bitfield:  0bsSpPO
 *    s: Spindle Start   0x010
 *    S: Spindle Stop    0x008
 *    p: Program Start   0x004
 *    P: Program Stop    0x002
 *    O: OK              0x001
 **************************/
void setCncLEDs(byte bf) {
  digitalWrite(LED_OK,            (1 & (bf)));   
  digitalWrite(LED_PROGRAM_START, (1 & (bf>>1)));   
  digitalWrite(LED_PROGRAM_STOP,  (1 & (bf>>2)));   
  digitalWrite(LED_SPINDLE_START, (1 & (bf>>3)));   
  digitalWrite(LED_SPINDLE_STOP,  (1 & (bf>>4)));   
}

/*********************************************************
 * Cycle next Mode 
 * for LEDs of Estlcam controlled Buttons 
 *********************************************************/
void setNextCncLedMode() {
  gCncLedMode ++;  
  if (gCncLedMode == 1) { 
     setCncLEDs (MODE_1);     
  } else if (gCncLedMode == 2) { 
     setCncLEDs (MODE_2);
  } else if (gCncLedMode == 3) { 
     setCncLEDs (MODE_3);
  }  else {
     gCncLedMode = 0;
     setCncLEDs (0);     
  }
}

/*************************** 
 *  Initialize Buttons
 **************************/
void initButtons() {
  gButton[0].attach(BUTTON_AUX_1, INPUT_PULLUP);
  gButton[1].attach(BUTTON_AUX_2, INPUT_PULLUP);
  gButton[2].attach(BUTTON_AUX_3, INPUT_PULLUP);
  gButton[3].attach(BUTTON_AUX_4, INPUT_PULLUP);
  for (int i = 0; i < NUM_BUTTONS; i++) {
    gButton[i].interval(100);              
  }
}

/*************************** 
 *  Initialize GPIO Ports
 **************************/
void initLEDs() {  
  for (int i = 0; i<3; i++){
    digitalWrite(LED_PROGRAM_STOP, 1);
    SHORTDELAY;
    digitalWrite(LED_AUX_3, 1);
    SHORTDELAY;
    digitalWrite(LED_SPINDLE_STOP, 1);    
    SHORTDELAY;
    digitalWrite(LED_PROGRAM_START, 1);    
    SHORTDELAY;
    digitalWrite(LED_AUX_2, 1);   
    SHORTDELAY;
    digitalWrite(LED_SPINDLE_START, 1);    
    SHORTDELAY;
    digitalWrite(LED_AUX_1, 1);    
    SHORTDELAY;
    digitalWrite(LED_AUX_4, 1);    
    SHORTDELAY;
    digitalWrite(LED_OK, 1);    
    SHORTDELAY;
    digitalWrite(LED_PROGRAM_STOP, 0);
    SHORTDELAY;
    digitalWrite(LED_AUX_3, 0);
    SHORTDELAY;
    digitalWrite(LED_SPINDLE_STOP, 0);
    SHORTDELAY;
    digitalWrite(LED_PROGRAM_START, 0);
    SHORTDELAY;
    digitalWrite(LED_AUX_2, 0); 
    SHORTDELAY;
    digitalWrite(LED_SPINDLE_START, 0);
    SHORTDELAY;
    digitalWrite(LED_AUX_1, 0);
    SHORTDELAY;
    digitalWrite(LED_AUX_4, 0);    
    SHORTDELAY;
    digitalWrite(LED_OK, 0);    
    SHORTDELAY;
  }
  SHORTDELAY;  
  setAllLEDs(0);
  
  // Start with Mode_1: All on
  gCncLedMode = 0;
  setNextCncLedMode();  
}

/*************************** 
 *  Initialize GPIO Ports
 **************************/
void initPorts() {
  // init Outputs  
  pinMode(LED_OK, OUTPUT);
  digitalWrite(LED_OK, 0);    
  pinMode(LED_PROGRAM_START, OUTPUT);
  digitalWrite(LED_PROGRAM_START, 0);    
  pinMode(LED_PROGRAM_STOP, OUTPUT);
  digitalWrite(LED_PROGRAM_STOP, 0);    
  pinMode(LED_SPINDLE_START, OUTPUT);
  digitalWrite(LED_SPINDLE_START, 0);    
  pinMode(LED_SPINDLE_STOP, OUTPUT);
  digitalWrite(LED_SPINDLE_STOP, 0);    
  pinMode(LED_AUX_1, OUTPUT);
  digitalWrite(LED_AUX_1, 0);    
  pinMode(LED_AUX_2, OUTPUT);
  digitalWrite(LED_AUX_2, 0);    
  pinMode(LED_AUX_3, OUTPUT);
  digitalWrite(LED_AUX_3, 0);    
  pinMode(LED_AUX_4, OUTPUT);
  digitalWrite(LED_AUX_4, 0);    
  pinMode(RELAIS_1, OUTPUT);
  digitalWrite(RELAIS_1, 0);    
  pinMode(RELAIS_2, OUTPUT);
  digitalWrite(RELAIS_2, 0);    
  pinMode(RELAIS_3, OUTPUT);
  digitalWrite(RELAIS_3, 0);    
  pinMode(RELAIS_4, OUTPUT);
  digitalWrite(RELAIS_4, 0);    
  pinMode(AUX_OUT, OUTPUT);
  digitalWrite(AUX_OUT, 0);    
  // init Inputs
  pinMode(BUTTON_AUX_1, INPUT_PULLUP);
  pinMode(BUTTON_AUX_2, INPUT_PULLUP);
  pinMode(BUTTON_AUX_3, INPUT_PULLUP);
  pinMode(BUTTON_AUX_4, INPUT_PULLUP);
}

/*************************** 
 *  Set Relay
 *  - set relay state
 *  - set AUX LED according state
 *  Relais 4 and 5 never used (reserved for further usage)
 **************************/
void setRelais(byte num, boolean state){
  if (num == 0) { 
     digitalWrite(RELAIS_1, state);
     digitalWrite(LED_AUX_1, state);
  } else if (num == 1) { 
     digitalWrite(RELAIS_2, state);
     digitalWrite(LED_AUX_2, state);
  } else if (num == 2) { 
     digitalWrite(RELAIS_3, !state);
     digitalWrite(LED_AUX_3, state);
  } else if (num == 3) { 
     digitalWrite(RELAIS_4, state);
     digitalWrite(LED_AUX_4, state);
  } else if (num == 4) { 
     digitalWrite(AUX_OUT, state);    
  }    
}

/*************************** 
 *  Initialize Relais
 *  - global status vars
 *  - portstates
 **************************/
void initRelais(){
  gRelais[0] = DEFAULT_RELAIS_1;
  gRelais[1] = DEFAULT_RELAIS_2;
  gRelais[2] = DEFAULT_RELAIS_3;
  gRelais[3] = DEFAULT_RELAIS_4;
  for (int i = 0; i < NUM_RELAIS; i++) {
    setRelais(i, gRelais[i]);    
  }
}

/*************************** 
 *  Setup
 **************************/
void setup() {
  // init Serial-Port
  Serial.begin(115200);  
  Serial.println("Init...");
  
  // Init GPIO
  Serial.print(" GPIO... ");  
  initPorts();
  Serial.println("done.");

  // Init LEDs
  Serial.print(" LEDs... ");  
  initLEDs();
  Serial.println("done.");

  // Init Buttons
  Serial.print(" Buttons... ");  
  initButtons();
  Serial.println("done.");

  // Init Relais
  Serial.print(" Relais... ");  
  initRelais();
  Serial.println("done.");

  // Init done.
  Serial.println("Init done.");
  Serial.println("---------------------");
  Serial.println("Starting Main Loop...");
}

/*************************** 
 *  Main Loop
 **************************/
void loop() {
  // get button states and handle changes
  for (int i = 0; i < NUM_BUTTONS; i++)  {
    gButton[i].update();    
    if (gButton[i].fell() ) {            
        // Buttons 1-4 toggle Relais 1-4
        gRelais[i] = !gRelais[i];
        setRelais(i, gRelais[i]);
    }
  }
}

