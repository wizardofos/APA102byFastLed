//
// APA102 by FastLED on Arduino UNO
//
// Wizardofos, 2017
//

#include <FastLED.h>

#define NUM_LEDS           90

#define DATA_PIN           1     // SPI MOSI pin (data out)
#define CLOCK_PIN          2     // SPI clock pin
#define BUTTON_PIN         4     // Default pin for button operation
#define DEFAULT_SPIFREQ    10    // Set to 10kHz for operation over long distances
#define DEFAULT_DELAY      15    // Default ms wait in the 'loop' function
#define DEFAULT_BRIGHTNESS 255   // Default brightness of the leds

#define MIN_BRIGHTNESS     0     // The minimum brightness of the ledstrip
#define MAX_BRIGHTNESS     255   // The maximum brightness of the ledstrip
#define DIM_STEP           1     // Steps for increasing/decreasing the brightness
#define DIM_DELAY          500   // 

CRGB leds[NUM_LEDS];

bool reading          = LOW;            // Actual state of the button
bool lastReading      = LOW;            // Start with button not pressed
byte dimValue         = MIN_BRIGHTNESS; // Start with minimal brightness
byte prevDimValue     = MAX_BRIGHTNESS; // Return value when toggled
bool dimState         = LOW;            // default increase brightness when input hold HIGH
bool buttonState      = LOW;            // the current reading from the input pin
bool prevButtonState  = LOW;            // the previous reading from the input pin
byte stateChanged     = 0;


// The following variables are unsigned long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.

unsigned long lastDebounceTime = 0;     // the last time the input pin was toggled
unsigned long debounceDelay = 50;       // the debounce time; increase if the output flickers
unsigned long startPressed = 0;         // the last time the input whas HIGH
unsigned long buttonPressed = 0;        // the duration the input was kept HIGH


void setup() {

  // initialize the digital pin as an output.
  pinMode(1, OUTPUT); //LED on Model A  or Pro
  showWake();
  
  pinMode(DATA_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, RGB, DATA_RATE_KHZ(DEFAULT_SPIFREQ)>(leds, NUM_LEDS);
  FastLED.setBrightness(MAX_BRIGHTNESS);

  setLed(MIN_BRIGHTNESS);
  FastLED.show();
}

void showWake() {
  for (int i = 0; i < 10; i++) {
    digitalWrite(0, HIGH);   // turn the LED on (HIGH is the voltage level)
    digitalWrite(1, HIGH);
    delay(50);               // wait for a second
    digitalWrite(0, LOW);    // turn the LED off by making the voltage LOW
    digitalWrite(1, LOW); 
    delay(50);               // wait for a second    
  }
}

/*
 * If the pushbutton is pressed (consider debouncing) for less then 500ms, swap the ledstate
 * If pressed longer then 500ms, when dimstate > 0, increase luminosity,
 * else decrease luminosity each 50ms until button released
 */

void loop() {
  
  // read the state of the switch into a local variable:
  reading = digitalRead(BUTTON_PIN);

  // check to see if button is pressed
  // (i.e. the input went from LOW to HIGH), and we've waited
  // long enough since the last press to ignore any noise:

  // If the switch changed, due to noise or pressing:
  if (reading != lastReading) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    buttonState = reading;
  }

  if (buttonState == HIGH && prevButtonState == LOW) {
    startPressed = millis();
  }

  // Measure time input is kept HIGH
  if (buttonState == HIGH) {
    // Ignore when startup with input already being HIGH
    if (startPressed != 0) {
      buttonPressed = millis() - startPressed;
    }
  }
  
  if (buttonState == LOW && prevButtonState == HIGH) {
    if (buttonPressed < DIM_DELAY) {
      toggleLed();
      stateChanged = 2;
    }

    dimState = !dimState;
  }

  if (buttonState == HIGH && buttonPressed > 500) {
    dimLed();
    stateChanged = 2;
  }

  // save the reading.  Next time through the loop,
  // it'll be the lastReading:
  lastReading = reading;

  prevButtonState = buttonState;

  // Instead of calling FastLED.show repeatedly, we
  // only need to call it when something has changed
  // Strangly, we need to call it twice?
  // TODO: investigate why we need to call it twice
  
  if (stateChanged > 0) {
    FastLED.show();
    stateChanged--;
  }
  
  delay(DEFAULT_DELAY);
}

void toggleLed() {
  if (dimValue > MIN_BRIGHTNESS) {
    // We need to switch to minimal brightness
    // Retain the previous brightness setting
    prevDimValue = dimValue;
    dimValue = MIN_BRIGHTNESS;
  } else {
    // Switch back to the retained value
    dimValue = prevDimValue;
  }

  setLed(dimValue);
}

void dimLed() {
  if (dimState == HIGH && dimValue > MIN_BRIGHTNESS)
    decreaseBrightness();
  else if (dimState == LOW && dimValue < MAX_BRIGHTNESS)
    increaseBrightness();  
}

void increaseBrightness() {
  if (dimValue < MAX_BRIGHTNESS) {
    dimValue += DIM_STEP;
    setLed(dimValue);
  }
}

void decreaseBrightness() {
  if (dimValue > (MIN_BRIGHTNESS + 1)) {
    dimValue -= DIM_STEP;
    setLed(dimValue);
  }
}

void setLed(byte value) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(value, value, value);
  }
}
