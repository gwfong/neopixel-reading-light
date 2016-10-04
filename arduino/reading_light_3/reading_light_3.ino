#include <Adafruit_NeoPixel.h>
#include "NeoPatterns.h"
#include <Bounce2.h>
#include <Encoder.h>
#include "PirSleepDetection.h"

const int PIN_LED = 6;
const int PIN_BTN = 3;
const int PIN_ENCODER_A = 2;
const int PIN_ENCODER_B = 8;
const int PIN_PIR = 9;
const int NUM_PIXELS = 16;
const int INTERVAL_SLOW = 700;
const int INTERVAL_FAST = 200;
const int BRIGHTNESS_INTERVAL = 5;
const int RAINBOW_SPEED = 5;
const int BRIGHTNESS_MAX = 255;
const int BRIGHTNESS_MIN = 1;

enum LightState {
	OFF = 0,
	READING_WHITE = 1,
	READING_RED = 2,
	MOOD_SLOW = 3,
	MOOD_FAST = 4
};

void ringComplete();
NeoPatterns ring(NUM_PIXELS, PIN_LED, NEO_GRB + NEO_KHZ800, &ringComplete);

Bounce bounce = Bounce();
bool ledOn = false;
LightState lightState = LightState::OFF;

Encoder knob(PIN_ENCODER_A, PIN_ENCODER_B);
long knobPos = 0;
int rainbowDirection = FORWARD;

PirSleepDetection pirSleep(15, 5, 5);

void setup() {
	Serial.begin(9600);

	pinMode(PIN_BTN, INPUT);
	bounce.attach(PIN_BTN);
	bounce.interval(5);

	ring.begin();
	lightsOff(); // turn off lights just in case if they're on

	pinMode(PIN_PIR, INPUT);
	pirSleep.attach(PIN_PIR);
}

void loop() {
  
	bounce.update();
	if (bounce.fell()) {

		Serial.println("fell");

		// If the lights are on, move to next light effect
		if (ledOn) {
			nextState();
			performState();
      
		}

		// If the lights are off, turn on the lights
		else {
			lightState = LightState::READING_WHITE;
			performState();
		}

		// If button pressed, then only the button activity will
		// be handled in this loop run.
		return;
	}

	// If the light is not on, then we are done.
	if (!ledOn) {
		return;
	}

	// Was the knob rotated? If so, which way?
	long newPos = knob.read();
	if (newPos < knobPos) {
		lowerLights();
		knobPos = newPos;
	} else if (newPos > knobPos) {
		raiseLights();
		knobPos = newPos;
	}

	// At this point, the lights are on...

	if (lightState == LightState::MOOD_SLOW
		|| lightState == LightState::MOOD_FAST) {
		ring.Update();
	}

	// Check to see if the person is asleep
	pirSleep.update();
	if (!pirSleep.isAwake()) {
		lightState = LightState::OFF;
		performState();
	}
}

void lowerLights() {
	int b = max(BRIGHTNESS_MIN, ring.getBrightness() - BRIGHTNESS_INTERVAL);
	for (int i = 0; i < NUM_PIXELS; i++) {
		ring.setBrightness(b);
	}
	ring.show();
	Serial.print("lowered lights to: ");
	Serial.println(b);
}

void raiseLights() {
	int b = min(BRIGHTNESS_MAX, ring.getBrightness() + BRIGHTNESS_INTERVAL);
	for (int i = 0; i < NUM_PIXELS; i++) {
		ring.setBrightness(b);
	}
	ring.show();
	Serial.print("raised lights to: ");
	Serial.println(b);
}

void nextState() {
	Serial.print("nextState: ");
	Serial.println(lightState);
  
	if (lightState == LightState::OFF) {
		lightState = LightState::READING_WHITE;
	} else if (lightState == LightState::READING_WHITE) {
		lightState = LightState::READING_RED;
	} else if (lightState == LightState::READING_RED) {
		lightState = LightState::MOOD_SLOW;
	} else if (lightState == LightState::MOOD_SLOW) {
		lightState = LightState::MOOD_FAST;
	} else if (lightState == LightState::MOOD_FAST) {
		lightState = LightState::OFF;
	}
  
	Serial.print("nextState: ");
	Serial.println(lightState);
}

void performState() {
	if (lightState == LightState::OFF) {
		lightsOff();
		ledOn = false;
		pirSleep.stopMonitoring();
	} else if (lightState == LightState::READING_WHITE) {
		lightsWhite();
		ledOn = true;
		pirSleep.startMonitoring();
	} else if (lightState == LightState::READING_RED) {
		lightsRed();
		ledOn = true;
		pirSleep.startMonitoring();
	} else if (lightState == LightState::MOOD_SLOW) {
		lightsMoodSlow();
		ledOn = true;
		pirSleep.startMonitoring();
	} else if (lightState == LightState::MOOD_FAST) {
		lightsMoodFast();
		ledOn = true;
		pirSleep.startMonitoring();
	}
}

void lightsWhite() {
	Serial.println("lights white");
	for (int i = 0; i < NUM_PIXELS; i++) {
		ring.setPixelColor(i, 255, 255, 255);
	}
	ring.show();
}

void lightsRed() {
	Serial.println("lights red");
	for (int i = 0; i < NUM_PIXELS; i++) {
		ring.setPixelColor(i, 255, 0, 0);
	}
	ring.show();
}

void lightsMoodSlow() {
	Serial.println("lights mood slow");
	ring.RainbowCycle(RAINBOW_SPEED);
}

void lightsMoodFast() {
	Serial.println("lights mood fast");
	ring.TheaterChase(
		ring.Color(random(255), random(255), random(255)),
		ring.Color(random(255), random(255), random(255)),
		100);
}

// Turn off the lighs
void lightsOff() {
	Serial.println("lights off");
	ring.clear();
	ring.show();
}

// Callback method called when the pattern has completed
void ringComplete() {
	Serial.println("ring complete");
	if (lightState == LightState::MOOD_SLOW) {
		lightsMoodSlow();
	} else if (lightState == LightState::MOOD_FAST) {
		lightsMoodFast();
	}
}

