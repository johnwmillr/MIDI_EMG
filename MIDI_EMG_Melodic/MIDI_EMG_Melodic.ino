#include <math.h>
#define LED 13

// Sensor pin
const int n_sensors = 2;
const int sensorPins[n_sensors] = {A5, A4};
const int pin_pitch = 0;
const int pin_velocity = 1;
double sensorVals[n_sensors] = {0,0};
bool   sensor_is_ready = true;

// Threshold
double thresholds[n_sensors] = {0,0};

// MIDI Stuff
bool note_is_on = false;
int pitch;
int vel;
int prev_pitch;

// Timing stuff
double t_last_note = 0;
int t_between_notes = 100; // Min. time until next trigger from sensor (ms)

void setup() {    
  pinMode(LED, OUTPUT);
  Serial.begin(31250); // Talk MIDI
  // Serial.begin(9600);

  // Measure baseline level w/ no muscle activity
  // Flash the LED to indicate the start of baseline recording
  flashLED(6);  
  digitalWrite(LED, HIGH);
  for (int n=0; n<n_sensors; n++){
    thresholds[n] = measureBaseline(n); // Trial & error for the multiplier    
  }

  thresholds[pin_pitch]    = 0.9*thresholds[pin_pitch];
  thresholds[pin_velocity] = 1.2*thresholds[pin_velocity];

  flashLED(6); // Indicate end of recording

}

// ----------------------------------------------------------------
// Start loopin'
void loop() {
  // Read and average some values from the pins
  // for (int n=0; n<n_sensors; n++){
  //   sensorVals[n] = sampleValues(3.0,n);
  // }
  sensorVals[pin_pitch] = sampleValues(6.0,pin_pitch);
  sensorVals[pin_velocity] = sampleValues(3.0,pin_velocity);

  // Check if you've crossed a threshold on the intensity sensor
  if (sensorVals[pin_velocity] > thresholds[pin_velocity]){  
    if (note_is_on == false){
      pitch =  convertSignalToPitch(sensorVals[pin_pitch]);
    }
    vel = convertSignalToVelocity(sensorVals[pin_velocity]);    
    // vel = 127;
    // pitch = 60;

    if (pitch != prev_pitch){
      sendNoteOff(prev_pitch);
    }
    if (note_is_on == false){
      sendNoteOn(pitch,vel);
      digitalWrite(LED, HIGH);
      t_last_note = millis();    
      note_is_on = true;    
      prev_pitch = pitch;
    }
    else{
      t_last_note = millis();
    }  
  }
  else if (note_is_on && (sensorVals[pin_velocity] < 0.90*thresholds[pin_velocity])){// || (millis() - t_last_note > 1000)){
    sendNoteOff(prev_pitch);
    note_is_on = false;
    digitalWrite(LED, LOW);
  }  

  delay(1);  
}

// ----------------------------------------------------------------
// Define the internal functions
void sendNoteOn(int note, int velocity) {
  int command  = 144; //144 = 10010000 in binary, note on command
  Serial.write(command); // Send note on command
  Serial.write(note);
  Serial.write(velocity); // Velocity value of 0 (turn the note off)
}

void sendNoteOff(int note) {
  int command = 128; // Note off, 128 = 10000000 in binary, note off command
  Serial.write(command); // Send note on command
  Serial.write(note);
  Serial.write(0); // Velocity value of 0 (turn the note off)
}

int convertSignalToPitch(double sig){
  // const int n_notes = 8;
  // const int notes[n_notes] = {60, 62, 64, 65, 67, 69, 71, 72};  
  // const int note_min = 60; // Set min and max corresponding to notes array
  // const int note_max = 72;
  // const int notes[n_notes] = {36, 38, 40, 41, 43, 45, 46, 48};  
  const int n_notes = 6;
  const int notes[n_notes] = {36, 38, 39, 41, 43, 48};
  const int note_min = 36; // Set min and max corresponding to notes array
  const int note_max = 48;

  
  const double sig_min = thresholds[pin_pitch];
  const double sig_max = 1.25*thresholds[pin_pitch]; // Emperical value

  // sig = log(sig);
  if (sig < sig_min){
    sig = sig_min;
  }
  if (sig > sig_max){
    sig = sig_max;
  }

  // Normalize to the desired MIDI velocity range  
  int note_index = round((((sig - sig_min) * (n_notes-1)) / (sig_max-sig_min)));

  return notes[note_index]+12;
}

int convertSignalToVelocity(double sig){    
  const int vel_min = 20;
  const int vel_max = 130;

  // Take the log of the raw EMG signal  
  const double sig_min = log(thresholds[pin_velocity]);
  const double sig_max = log(1.25*thresholds[pin_velocity]); // Guess  
  
  sig = log(sig);
  if (sig < sig_min){
    sig = sig_min;
  }
  if (sig > sig_max){
    sig = sig_max;
  }

  // Normalize to the desired MIDI velocity range
  int value = round((((sig - sig_min) * (vel_max - vel_min)) / (sig_max-sig_min)) + vel_min);

  if (value > 127){
    value = 127;
  }

  return value;
}

double sampleValues(const double n_samples, const int n_pin){  
  double val   = 0.0;
  double total = 0.0;

  // Take samples
  for (int i = 0; i < n_samples; i++){
    val = abs(analogRead(sensorPins[n_pin]));
    total = total + val;
    delay(1);
  }

  // Determine average
  return total / n_samples;

}

void flashLED(int t) {
  for (int i = 0; i < t; i++) {
    digitalWrite(LED, HIGH);
    delay(50);
    digitalWrite(LED, LOW);
    delay(50);
  }
}

// Measures the average signal for a preset amount of time at start of trial
double measureBaseline(const int n_sensor) {  

  // Take samples to record the average activity level of your EMG signal        
  double average = sampleValues(500,n_sensor);

  return average;
}
