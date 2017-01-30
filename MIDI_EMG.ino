#include <math.h>
#define LED 13

// Sensor pin
const int sensorPin = A5;
double sensorVal = 0;

// Threshold
double noiseFloor = 0;
double threshold = 0;
bool over_thresh = false;

// MIDI Stuff
bool note_is_on = false;

// Timing stuff
double t_prev_note = 0;
int t_between_notes = 100; // Min. time until next trigger from sensor (ms)

// ----------------------------------------------------------------

double sampleValues(const double n_samples = 5.0){  
  double val   = 0.0;
  double total = 0.0;

  // Take samples
  for (int i = 0; i < n_samples; i++){
    val = abs(analogRead(sensorPin));
    total = total + val;
    delay(1);
  }

  // Determine average
  return total / n_samples;

}

int convertSignalToMIDI(double sig, const int vel_min, const int vel_max){    
  const double sig_min = log(threshold);
  const double sig_max = log(1.35*threshold); // Guess  

  sig = log(sig);

  // Normalize to the desired MIDI velocity range
  int midi_val = round((((sig - sig_min) * (vel_max - vel_min)) / (sig_max-sig_min)) + vel_min);
  

  if (midi_val > 127){
    midi_val = 127;
  }

  // Serial.println(midi_val);

  return midi_val;
}

int convertSignalToPitch(double sig){
  const int n_notes = 8;
  int notes[n_notes] = {60, 62, 64, 65, 67, 69, 71, 72};  
  const int note_min = 60;
  const int note_max = 72;

  const double sig_min = log(threshold);
  const double sig_max = log(1.3*threshold); // Guess  

  sig = log(sig);

  // Normalize to the desired MIDI velocity range  
  int value = round((((sig - sig_min) * (n_notes-1 - 0)) / (sig_max-sig_min)) + 0);
  

  // if (value > 127){    
  //   value = 127;
  // }

  // Serial.println(vel);

  return notes[value];



}

// ----------------------------------------------------------------
void setup() {  
  pinMode(LED, OUTPUT);
  Serial.begin(31250);
  // Serial.begin(9600);

  noiseFloor = measureBaseline();
  threshold = 1.21 * noiseFloor; // Trial and error
    
}

// ----------------------------------------------------------------
void loop() {        
    sensorVal = sampleValues(3.0);    
    int pitch = 0;

    if (sensorVal > threshold && (millis() - t_prev_note) > t_between_notes)
    {    
      pitch = convertSignalToPitch(sensorVal);
      double vel   = convertSignalToVel(sensorVal, 30, 130);
      sendNoteOn(pitch, vel);
      t_prev_note = millis();
      note_is_on = true;
      digitalWrite(LED, HIGH);
    }
    else if (note_is_on)
    {      
      sendNoteOff(pitch);
      note_is_on = false;
      digitalWrite(LED, LOW);
    }

    delay(1);  
}

// -------------------------------------------------------------------
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

int convertSignalToVel(double sig, const int vel_min, const int vel_max){    
  const double sig_min = log(threshold);
  const double sig_max = log(1.3*threshold); // Guess  

  sig = log(sig);

  // Normalize to the desired MIDI velocity range
  int vel = round((((sig - sig_min) * (vel_max - vel_min)) / (sig_max-sig_min)) + vel_min);

  

  if (vel > 127){
    vel = 127;
  }

  // Serial.println(vel);

  return vel;
}


double convertToVoltage(double analogVal)
{
  return 5.0 * (analogVal / 1023.0);
}

void flashLED(int t = 8) {
  for (int i = 0; i < t; i++) {
    digitalWrite(LED, HIGH);
    delay(50);
    digitalWrite(LED, LOW);
    delay(50);
  }
}

// Measures the average signal for a preset amount of time at start of trial
double measureBaseline() {
  // Flash the LED to indicate the start of baseline recording
  flashLED();
  digitalWrite(LED, HIGH);

  // Take samples to record the average activity level of your EMG signal      
  double average;
  average = sampleValues(1000);  

  // Indicate end of recording
  flashLED();

  return average;
}
