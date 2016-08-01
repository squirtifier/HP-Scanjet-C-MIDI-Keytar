// July 2016
//90% of this code for the MIDI Simple Synth was taken directly from the Arduino's web site "MIDI_SimpleSynth.ino"
//The Arduino NANO listens to MIDI channel one and outputs the corresponding note to the stepper motor board's CLK input
//This program was compiled with Arduino version 1.6.7 for windows
// Don't for get to install the arduino_midi_library-dev.zip (just search the internet for this file name) or the code won't compile
//HP Scanjet MIDI Keytar was built by James Cochrane, Toronto, Ontario, Canada - https://www.YouTube.com/BD594
//Hardware used Arduino Nano or UNO (both tested), ITEADStudio Motor Controller shield v1.1 and any MIDI shield
//Don't worry about the error when compiling the code // WARNING: Category 'audio' in library Arduino-MIDI is not valid. Setting to 'Uncategorized'

#include <MIDI.h> //don't forget to install the arduino_midi_library-dev.zip
#include "noteList.h" 
#include "pitches.h"

// These constants won't change and are used to set pin numbers
//Attach limitswitches comman to +5VDC and N.O to pin 10 with a 10K pull down resistor to ground
const int LimitSwitchesPin = 10;    // the input pin number for the limit switches
const int StepperMotorDirectionPin = 3;      // direction output pin for Stepper Motor controller

// Variables will change:
int StepperMotorDirectionState = HIGH;  // the current state of the output pin
int LimitSwitchesState;             // the current reading from the input pin
int lastLimitSwitchesState = LOW;   // the previous reading from the input pin

// the following variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
long lastDebounceTime = 0;  // the last time the output pin was toggled
long debounceDelay = 50;    // the debounce time; increase if the output flickers


MIDI_CREATE_DEFAULT_INSTANCE();

#ifdef ARDUINO_SAM_DUE // Due has no tone function (yet), overriden to prevent build errors.
#define tone(...)
#define noTone(...)
#endif

// This example shows how to make a simple synth out of an Arduino, using the
// tone() function. It also outputs a gate signal for controlling external
// analog synth components (like envelopes).

static const unsigned sGatePin     = 13; //  pin 13 MIDI RX LED/Power Indicator
static const unsigned sAudioOutPin = 2; // Step input to Stepper Motor controller
static const unsigned sMaxNumNotes = 16;
MidiNoteList<sMaxNumNotes> midiNotes;

// -----------------------------------------------------------------------------

inline void handleGateChanged(bool inGateActive)
{
   digitalWrite(sGatePin, inGateActive ? LOW : HIGH); // MIDI RX LED on-off-on //usefull because the LED light acts as a power indicator
   // digitalWrite(sGatePin, inGateActive ? HIGH : LOW); // MIDI RX LED off-on-off // Blinks only when MIDI is received on channel 1
}

inline void pulseGate()
{
    handleGateChanged(false);
    delay(1);
    handleGateChanged(true);
}

// -----------------------------------------------------------------------------

void handleNotesChanged(bool isFirstNote = false)
{
    if (midiNotes.empty())
    {
        handleGateChanged(false);
        noTone(sAudioOutPin);
    }
    else
    {
        // Possible playing modes:
        // Mono Low:  use midiNotes.getLow
        // Mono High: use midiNotes.getHigh
        // Mono Last: use midiNotes.getLast

        byte currentNote = 0;
        if (midiNotes.getLast(currentNote))
        {
            tone(sAudioOutPin, sNotePitches[currentNote]);

            if (isFirstNote)
            {
                handleGateChanged(true);
            }
            else
            {
                pulseGate(); // Retrigger envelopes. Remove for legato effect.
            }
        }
    }
}

// -----------------------------------------------------------------------------

void handleNoteOn(byte inChannel, byte inNote, byte inVelocity)
{
    const bool firstNote = midiNotes.empty();
    midiNotes.add(MidiNote(inNote, inVelocity));
    handleNotesChanged(firstNote);
}

void handleNoteOff(byte inChannel, byte inNote, byte inVelocity)
{
    midiNotes.remove(inNote);
    handleNotesChanged();
}

// -----------------------------------------------------------------------------

void setup() {
  pinMode(LimitSwitchesPin, INPUT);
  pinMode(StepperMotorDirectionPin, OUTPUT);

  // set initial Stepper Motor Direction
  digitalWrite(StepperMotorDirectionPin, StepperMotorDirectionState);
 
    pinMode(sGatePin,     OUTPUT);
    pinMode(sAudioOutPin, OUTPUT);
    MIDI.setHandleNoteOn(handleNoteOn);
    MIDI.setHandleNoteOff(handleNoteOff);
    MIDI.begin();
}

void loop() {

  // read the state of the limit switch into a local variable:
  int reading = digitalRead(LimitSwitchesPin);

  // check to see if the Hall Effect switch sensed the magnet or the manual limit switch was pressed
  // (i.e. the input went from HIGH to LOW),  and you've waited
  // long enough since the last press to ignore any noise:

  // If the Hall Effect or limit switch changed, due to noise or pressing:
  if (reading != lastLimitSwitchesState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the limit button state has changed:
    if (reading != LimitSwitchesState) {
      LimitSwitchesState = reading;

      // only toggle Stepper Motor directon if the new button state is LOW
      if (LimitSwitchesState == LOW) {
        StepperMotorDirectionState = !StepperMotorDirectionState;
      }
    }
  }

  // set the Stepper Motor Direction:
  digitalWrite(StepperMotorDirectionPin, StepperMotorDirectionState);

  // save the reading.  Next time through the loop,
  // it'll be the lastLimitSwitchesState:
  lastLimitSwitchesState = reading;
   
    MIDI.read(); //read the MIDI In port
}
