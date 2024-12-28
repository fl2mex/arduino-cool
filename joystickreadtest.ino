// i dont actualy know how to program so dont expect this to be any good please
// 
// Date 28/12/24
// Commit by Kaiden
// - Added enum for direction, state does not need an enum because it is globally managed
// - Cleaned up joyconCalc, worst case goes from 9 if statements down to 6 by managing scope
// - Tabify'd it ;)
//
// Date 10/08/23
// Commit by Kaiden
// - Cleaned up Code
// TODO: Set up enums for direction and state, and clean switch statements
//
//Date: 9/08/23
//Wiring Instructions:
// - Step one: don't
//
// - I am using a joystick module with it's own Z axis but you can replace it
//   with a regular push button if you want
//
// - Wire Joycon Z or momentary push button between a Digital I/O pin.
// - Wire Joycon X and Y pins into seperate Analog pins.
// - Wire the joycon positive and ground pins into the arduino 5V and Ground pins
//

const bool DEBUG = true;  // Set to true to enable serial monitor debugging info

// --------CONSTANTS (won't change)---------------

//Joycon
const int Y_PIN = A0, X_PIN = A1; // Analog 
const byte Z_PIN = 13; // Digital
const byte DEADZONE = 150;

//Button
const byte BUTTON_PIN = 2; // Digital

//LED Strip
const byte LED_PIN = 12; // Digital
const byte NUM_LEDS = 30; // Number of lights in use

// ----------LIBRARIES--------------

#include "FastLED.h" // Weird place to put a library, but isn't wrong
CRGB leds[NUM_LEDS];

#include "JC_Button.h"

//------------ VARIABLES (will change)-----

enum Direction
{
	Unset = 9,
	Centre = 0,
	North = 1,
	NorthEast = 2,
	East = 3,
	SouthEast = 4,
	South = 5,
	SouthWest = 6,
	West = 7,
	NorthWest = 8
}
byte direction = Direction::Unset;
//(8)(1)(2)
//(7)(0)(3)
//(6)(5)(4)

// ----------FUNCTIONS--------------

bool inCenterRange(int val) { return ((500 - DEADZONE <= val) && (val <= 500 + DEADZONE)); }
bool inUpperRange(int val) { return ((501 + DEADZONE <= val) && (val <= 1024)); }
bool inLowerRange(int val) { return ((0 <= val) && (val <= 499 - DEADZONE)); }

void setup()
{
	// input/output pin setups
	pinMode(LED_PIN, OUTPUT);
	pinMode(Z_PIN, INPUT); 

	// FastLED library specific setup
	FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, NUM_LEDS);
	FastLED.setBrightness(50);
	FastLED.clear();

	// Debugging for Serial monitor
	if(DEBUG)
	{
		Serial.begin(115200); //enable serial monitor
		Serial.println("Debugging has been enabled.");
		Serial.println("Setup complete.");
	}
}

void loop()
{
	buttonCheck();
}

void buttonCheck()
{
	// To Char: Static means it persists over descope - every time buttonCheck() ends, all non-static items are descoped
	// For example, the read integer will be descoped and reinitialised with digitalRead()
	// Another way of working this is to have an integer that either takes an input and an output or a reference to an int so it is modifiable

	static unsigned long time = 0, t0 = 0;
	static const long debounceMs = 100; // ms
	static int state = 0;

	int read = digitalRead(Z_PIN);

	switch(state)
	{
	case 0: // Reset. 
		if(read == HIGH) state = 1; // Proceeds when reading 5V (a potential button press)
		break;
	case 1: // Capture first time, immeadiatly proceed
		t0 = millis();
		state = 2;
		break;
	case 2:
		time = millis(); // Capture second time to compare with the first.
		if (time - t0 > debounceMs) state = 3; // Proceeds only if the debounce delay time has elapsed
		if(read == LOW) state = 0; // Resets if signal goes back to 0V before the debounce delay
		break;
	case 3: // Button has now confirmed a 'true' press
		if(read == LOW) state = 4; // But it will wait until the button is lifted before triggering the click event
		break;
	case 4: // Click
		if(DEBUG) Serial.println("Click!");
		joyconCalc(); // Event trigger
		state = 0; // After running the events, resets
		break;
	default:
		if(DEBUG) Serial.println("ERROR - Default case in buttonCheck switchcase"); // If I end up getting this message than I should probably start praying
		break;
	}
}

// note: joyconCalc and directionTrigger could probably be merged...but I'm keeping some things seperate until I fully figure them out
void joyconCalc()
{
	int y_axis = analogRead(Y_PIN); // 0-1024
	int x_axis = analogRead(X_PIN); // 0-1024

	if(DEBUG)
	{
		Serial.print("Y Value: " + std::string(y_axis) + "\n");
		Serial.print("X Value: " + std::string(x_axis) + "\n");
	}

	// evaluate by comparing the axises and change the direction variable accordingly
	direction = Direction::Unset; // Start by setting to unset
	if (inUpperRange(y_axis))
	{
		if (inUpperRange(x_axis)) direction = Direction::NorthEast;
		else if (inCenterRange(x_axis)) direction = Direction::North;
		else if (inLowerRange(x_axis)) direction = Direction::NorthWest;
	}
	else if (inCenterRange(y_axis))
	{
		if (inUpperRange(x_axis)) direction = Direction::East;
		else if (inCenterRange(x_axis)) direction = Direction::Centre;
		else if (inLowerRange(x_axis)) direction = Direction::West;
	}
	else if (inLowerRange(y_axis))
	{
		if (inUpperRange(x_axis)) direction = Direction::SouthEast;
		else if (inCenterRange(x_axis)) direction = Direction::South;
		else if (inLowerRange(x_axis)) direction = Direction::SouthWest;
	}

	if (DEBUG) directionTrigger();
}

void directionTrigger()
{
	switch (direction)
	{
	case Direction::Centre:
		centerLED();
		Serial.println("Center");
		break;
	case Direction::North:
		Serial.println("North");
		break;
	case Direction::NorthEast:
		Serial.println("North East");
		break;
	case Direction::East:
		Serial.println("East");
		break;
	case Direction::SouthEast:
		Serial.println("South East");
		break;
	case Direction::South:
		Serial.println("South");
		break;
	case Direction::SouthWest:
		Serial.println("South West");
		break;
	case Direction::West:
		Serial.println("West");
		break;
	case Direction::NorthWest:
		Serial.println("North West");
		break;
	default:
		Serial.println("Unaccounted for direction");
		break;
	}
}

void centerLED()
{
	// ignore this it's also a sort of placeholder
	// FastLED.clear();
	// fill_solid(leds, NUM_LEDS, CRGB::Red);
	// FastLED.show();
}
