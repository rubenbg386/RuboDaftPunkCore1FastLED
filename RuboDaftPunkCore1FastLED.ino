#include "FastLED.h"
#include <SPI.h>
#include <SD.h>
#include <LiquidCrystal_I2C.h>

//Variables referents a las luces
#define DATA_PIN 3
#define LED_TYPE WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS 256
#define BRIGHTNESS 96

#define CONTROLLER_PIN 0
#define CS_PIN 53 //10 o 4 en Nano

#define BUTTON_NEXT_PIN 22 //el boton NEXT cambia de modo
#define BUTTON_PLAY_PIN 24 //el boton PLAY inicializa cosas dentro del modo
#define BUTTON_AUX_PIN 26

#define INITIAL_MODE 1001
#define LAST_MODE 1004
#define MODE_GLEDI 1001
#define MODE_DEMO1 1002
#define MODE_DEMO2 1003
#define MODE_DEMO3 1004
#define MODE_PROGRAM 1005
#define MODE_TETRIS 1006
#define MODE_SNAKE 1007
#define MODE_PONG 1008
#define MODE_INTERACTIVE 1009
#define MODE_WAIT 1010
#define MODE_LIGTHTING 1011
#define MODE_PARTY 1012

#define I2C_ADDR    0x27

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

File fxdata;
CRGB leds[NUM_LEDS];
CRGB ledsFromFile[NUM_LEDS];

int controller_value = 0;

int button_next_status = 0; //estado de pulsacion del boton NEXT
int button_play_status = 0; //estado de pulsacion del boton PLAY
int button_aux_status = 0; //estado de pulsacion del boton AUX

int mode = INITIAL_MODE; //estado de programa establecido al modo inicial

int exitoSD = 0;

//const dataType variableName[] PROGMEM = {}; // use this form
const uint8_t TraslatedIndex[] = {
 255, 254, 253, 252, 251, 250, 249, 248,
 240, 241, 242, 243, 244, 245, 246, 247,
 239, 238, 237, 236, 235, 234, 233, 232,
 224, 225, 226, 227, 228, 229, 230, 231,
 223, 222, 221, 220, 219, 218, 217, 216,
 208, 209, 210, 211, 212, 213, 214, 215,
 207, 206, 205, 204, 203, 202, 201, 200,
 192, 193, 194, 195, 196, 197, 198, 199,
 191, 190, 189, 188, 187, 186, 185, 184,
 176, 177, 178, 179, 180, 181, 182, 183,
 175, 174, 173, 172, 171, 170, 169, 168,
 160, 161, 162, 163, 164, 165, 166, 167,
 159, 158, 157, 156, 155, 154, 153, 152,
 144, 145, 146, 147, 148, 149, 150, 151,
 143, 142, 141, 140, 139, 138, 137, 136,
 128, 129, 130, 131, 132, 133, 134, 135,
 127, 126, 125, 124, 123, 122, 121, 120,
 112, 113, 114, 115, 116, 117, 118, 119,
 111, 110, 109, 108, 107, 106, 105, 104,
  96,  97,  98,  99, 100, 101, 102, 103,
  95,  94,  93,  92,  91,  90,  89,  88,
  80,  81,  82,  83,  84,  85,  86,  87,
  79,  78,  77,  76,  75,  74,  73,  72,
  64,  65,  66,  67,  68,  69,  70,  71,
  63,  62,  61,  60,  59,  58,  57,  56,
  48,  49,  50,  51,  52,  53,  54,  55,
  47,  46,  45,  44,  43,  42,  41,  40,
  32,  33,  34,  35,  36,  37,  38,  39,
  31,  30,  29,  28,  27,  26,  25,  24,
  16,  17,  18,  19,  20,  21,  22,  23,
  15,  14,  13,  12,  11,  10,   9,   8,
   0,   1,   2,   3,   4,   5,   6,   7
};



void setup() {

	lcd.begin (16,2);    // Inicializar el display con 16 caraceres 2 lineas

	lcd.setBacklight(HIGH);

	lcd.home ();                   // go home
	lcd.print("Core1.MEGA v1.02");
	lcd.setCursor ( 0, 1 );        // go to the 2nd line
	lcd.print("Start");

	delay(1000);

	pinMode(BUTTON_NEXT_PIN,INPUT); //Pin donde está conectado el pulsador
	pinMode(BUTTON_PLAY_PIN,INPUT); //Pin donde está conectado el pulsador
	pinMode(BUTTON_AUX_PIN,INPUT); //Pin donde está conectado el pulsador

	//inicializacion de los leds
	lcd.clear();lcd.setCursor ( 0, 0 );lcd.print("Init LEDs");
	delay(1000); // initial delay of a few seconds is recommended
	FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip); // initializes LED strip
	//FastLED.setBrightness(BRIGHTNESS);// global brightness
	FastLED.setMaxPowerInVoltsAndMilliamps(5, 900);

	for(int i = 0 ; i < NUM_LEDS ; i++){
		if(i%2 == 0){
			leds[i] = CRGB::Crimson; // set all leds to black during setup
		}
	}
	FastLED.show();
	delay(1000);
	for(int i = 0 ; i < NUM_LEDS ; i++){
		leds[i] = CRGB::Black; // set all leds to black during setup
	}
	FastLED.show();

	lcd.clear();lcd.setCursor ( 0, 0 );lcd.print("Init SD");
	delay(1000);

	pinMode(CS_PIN, OUTPUT); // CS/SS pin as output for SD library to work.
	digitalWrite(CS_PIN, HIGH); // workaround for sdcard failed error...

	if (!SD.begin(CS_PIN))//4 en uno 53 en mega
	{
		lcd.clear();lcd.setCursor ( 0, 0 );lcd.print("SD Error");
		exitoSD = 0;
	}else{
		lcd.clear();lcd.setCursor ( 0, 0 );lcd.print("SD OK");
		exitoSD = 1;
	}


	mode = INITIAL_MODE;
}

//retorna 1 si se cambia de modo
int compruebaNEXT(){
	button_next_status=digitalRead(BUTTON_NEXT_PIN);
	if(button_next_status == 1) {
		mode++;
		if(mode>LAST_MODE){
			mode = INITIAL_MODE;
		}
		delay(500);
		return 1;
	}
	return 0;
}

//retorna 1 si el play está pulsado
int compruebaPLAY(){
	button_play_status=digitalRead(BUTTON_PLAY_PIN);
	if(button_play_status == 1) {
		delay(500);
		return 1;
	}
	return 0;
}

// switches off all LEDs
int showProgramCleanUp(long delayTime) {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[TraslatedIndex[i]] = CRGB::Black;
  }
  if(compruebaNEXT()>0) return 1; // si es mayor que cero es que se ha cambiado de modo
  FastLED.show();
  delay(delayTime);
  return 0;
}

// switches on all LEDs. Each LED is shown in random color.
// numIterations: indicates how often LEDs are switched on in random colors
// delayTime: indicates for how long LEDs are switched on.
int showProgramRandom(int numIterations, long delayTime) {
  for (int iteration = 0; iteration < numIterations; ++iteration) {
    for (int i = 0; i < NUM_LEDS; ++i) {
      leds[TraslatedIndex[i]] = CHSV(random8(),255,255); // hue, saturation, value
    }
    if(compruebaNEXT()>0) return 1; // si es mayor que cero es que se ha cambiado de modo
    FastLED.show();
    delay(delayTime);
  }
  return 0;
}

// Shifts a single pixel from the start of strip to the end.
// crgb: color of shifted pixel
// delayTime: indicates how long the pixel is shown on each LED
int showProgramShiftSinglePixel(CRGB crgb, long delayTime) {
  for (int i = 0; i < NUM_LEDS; ++i) {
    leds[TraslatedIndex[i]] = crgb;
    if(compruebaNEXT()>0) return 1; // si es mayor que cero es que se ha cambiado de modo
    FastLED.show();
    delay(delayTime);
    leds[TraslatedIndex[i]] = CRGB::Black;
  }
  return 0;
}

// Shifts multiple pixel from the start of strip to the end. The color of each pixel is randomized.
// delayTime: indicates how long the pixels are shown on each LED
int showProgramShiftMultiPixel(long delayTime) {
  for (int i = 0; i < NUM_LEDS; ++i) {
    for (int j = i; j > 0; --j) {
      leds[TraslatedIndex[j]] = leds[TraslatedIndex[j-1]];
    }
    CRGB newPixel = CHSV(random8(), 255, 255);
    leds[TraslatedIndex[0]] = newPixel;
    if(compruebaNEXT()>0) return 1; // si es mayor que cero es que se ha cambiado de modo
    FastLED.show();
    delay(delayTime);
  }
  return 0;
}

int showGlediatorAnim(){
	  fxdata = SD.open("all.dat");  // read only
	  if (fxdata)
	    {
	      //Serial.println("file open ok");
	    }

	  while (fxdata.available())
	  {
	    fxdata.readBytes((char*)ledsFromFile, NUM_LEDS*3);
	    for(int i = 0 ; i < NUM_LEDS ; i++)
	    {
	  	  leds[TraslatedIndex[i]] = ledsFromFile[i];
	    }

	    if(compruebaNEXT()>0){
	    		//Cerramos el fichero antes de salir
	    		fxdata.close();
	    		return 1; // si es mayor que cero es que se ha cambiado de modo
	    }

	    FastLED.show();
	    delay(10); // set the speed of the animation. 20 is appx ~ 500k bauds
	  }

	  // close the file in order to prevent hanging IO or similar throughout time
	  fxdata.close();
}

// main program
void loop() {


	switch (mode) {
	case MODE_GLEDI:
		lcd.clear();lcd.setCursor ( 0, 0 );lcd.print("Mode: ");lcd.print(mode);
		showProgramCleanUp(2500); // clean up
		showGlediatorAnim();
		break;
	case MODE_DEMO1:
		lcd.clear();lcd.setCursor ( 0, 0 );lcd.print("Mode: ");lcd.print(mode);
		showProgramCleanUp(250); // clean up
		showProgramRandom(100, 100); // show "random" program
		break;
	case MODE_DEMO2:
		lcd.clear();lcd.setCursor ( 0, 0 );lcd.print("Mode: ");lcd.print(mode);
		showProgramCleanUp(250); // clean up
		showProgramShiftSinglePixel(CHSV(random8(), 255, 255), 10); // show "shift single pixel program" with maroon pixel
		break;
	case MODE_DEMO3:
		lcd.clear();lcd.setCursor ( 0, 0 );lcd.print("Mode: ");lcd.print(mode);
		showProgramCleanUp(2500); // clean up
		showProgramShiftMultiPixel(50); // show "shift multi pixel" program
		break;
	default:
		lcd.clear();lcd.setCursor ( 0, 0 );lcd.print("DEFAULT CASE. Mode: ");lcd.print(mode);
		// default code
		showProgramCleanUp(2500); // clean up
	}

}
