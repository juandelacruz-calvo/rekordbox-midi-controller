

#define ARRAY_SIZE(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define FIRST_NOTE 60
#define LINE_GAP 1
#define MIDI_OUT_CHANNEL 1
#define MIDI_IN_CHANNEL 1
#define NUMBER_OF_KEYS 12
#define SECTION_GAP 3
#define NUMBER_OF_NOTES 128
#define UNDERLINE_HEIGHT 2
#define PIN_SWITCH 11
#define PIN_ROTA 10
#define PIN_ROTB 8

#include <Adafruit_TinyUSB.h>
#include <Arduino.h>
#include <MIDI.h>
#include <Keypad.h>

// Input/Output declarations #define outputA 6
#define rows 4
#define columns 3

int counter = 0;
int aState;
int aLastState;


char keys[rows][columns] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { 'a', 'b', 'c' }
};

byte rowPins[rows] = { 13, 29, 28, 27 };
// byte colPins[columns] = {27};

byte colPins[columns] = { 26, 15, 14 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, rows, columns);

String msg;
char key;
int status = 0;
int newStatus = 0;

// MIDI
Adafruit_USBD_MIDI usbdMidi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usbdMidi, MIDI);


int encoderPosition = 0;
bool buttonState;

struct Option {
  String name;
  byte min;
  byte max;
  byte value;
  byte range;
};

Option options[3] = {
  {
    "Range",
    0,
    NUMBER_OF_NOTES - NUMBER_OF_KEYS,
    FIRST_NOTE,
    NUMBER_OF_KEYS,
  },
  {
    "Attack",
    0,
    127,
    64,
    1,
  },
  {
    "Decay",
    0,
    127,
    0,
    1,
  },
};

byte selectedOption = 0;

const String noteNames[12] = {
  "C",
  "C#",
  "D",
  "D#",
  "E",
  "F",
  "F#",
  "G",
  "G#",
  "A",
  "A#",
  "B",
};

uint32_t keyColours[NUMBER_OF_KEYS] = {};
boolean externalNoteStates[NUMBER_OF_NOTES] = {};

void setup() {
  // Delay recommended for RP2040
  delay(100);

  // Set rotary encoder inputs and interrupts
  pinMode(PIN_ROTA, INPUT_PULLUP);
  pinMode(PIN_ROTB, INPUT_PULLUP);
  pinMode(PIN_SWITCH, INPUT_PULLUP);

  // Initialise MIDI
  MIDI.begin(MIDI_IN_CHANNEL);

  // Wait until the device is mounted
  while (!TinyUSBDevice.mounted()) {
    delay(1);
  }
  aLastState = digitalRead(PIN_ROTA);
}

void loop() {
  processEncoder();
  processKeys();
  MIDI.read();
}

void processKeys() {
  if (keypad.getKeys()) {
    for (int i = 0; i < LIST_MAX; i++)  // Scan the whole key list.
    {
      if (keypad.key[i].stateChanged)  // Only find keys that have changed state.
      {
        switch (keypad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:
            msg = " PRESSED.";
            MIDI.sendNoteOn(keypad.key[i].kchar, 127, MIDI_OUT_CHANNEL);
            break;
          case HOLD:
            msg = " HOLD.";
            break;
          case RELEASED:
            msg = " RELEASED.";
            MIDI.sendNoteOff(keypad.key[i].kchar, 0, MIDI_OUT_CHANNEL);
            break;
          case IDLE:
            msg = " IDLE.";
        }

        Serial.print("Key ");
        Serial.print(keypad.key[i].kchar);
        Serial.println(msg);
      }
    }
  }
}

void processEncoder() {
  // Check whether encoder is pressed
  bool newState = digitalRead(PIN_SWITCH);
  if (newState != buttonState) {
    if (!newState) {
      Serial.println("Button on");
      MIDI.sendNoteOn(100, 127, MIDI_OUT_CHANNEL);
    } else {
      Serial.println("Button off");
      MIDI.sendNoteOff(100, 0, MIDI_OUT_CHANNEL);
    }

    buttonState = newState;
  }

  aState = digitalRead(PIN_ROTA);  // Reads the "current" state of the outputA
  // If the previous and the current state of the outputA are different, that means a Pulse has occured
  if (aState != aLastState) {
    // If the outputB state is different to the outputA state, that means the encoder is rotating clockwise
    if (digitalRead(PIN_ROTB) != aState) {
      counter++;
      MIDI.sendNoteOn(101, 127, MIDI_OUT_CHANNEL);
      MIDI.sendNoteOff(101, 0, MIDI_OUT_CHANNEL);
    } else {
      MIDI.sendNoteOn(101, 127, MIDI_OUT_CHANNEL);
      MIDI.sendNoteOff(101, 0, MIDI_OUT_CHANNEL);
      counter--;
    }
    Serial.print("Position: ");
    Serial.println(counter);
  }
  aLastState = aState;  // Updates the previous state of the outputA with the current state
}

int mod(int x, int m) {
  return (x % (m + m)) % m;
}
