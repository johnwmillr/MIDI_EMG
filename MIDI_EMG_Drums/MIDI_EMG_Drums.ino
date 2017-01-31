#include <math.h>
#define LED 13

// Sensor pin
const int n_sensors = 2;
const int sensorPins[n_sensors] = {A5, A4};
double sensorVals[n_sensors] = {0,0};
bool sensor_is_ready[n_sensors] = {true,true};

// Threshold
double thresholds[n_sensors] = {0,0};

// MIDI Stuff
bool note_is_on[n_sensors] = {false,false};
int pitches[n_sensors] = {36,38};
int vel;

// Timing stuff
double t_last_note[n_sensors];
int t_between_notes = 120; // Min. time until next trigger from sensor (ms)
int t_duration = 200;

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
  flashLED(6); // Indicate end of recording

  thresholds[0] = 1.32*thresholds[0]; // Empirical
  thresholds[1] = 1.33*thresholds[1];  

}

// ----------------------------------------------------------------
// Start loopin'
void loop() {
  // Read and average some values from the pins
  for (int n=0; n<n_sensors; n++){
    sensorVals[n] = sampleValues(1.0,n);
  }
  
  // Check for sensor hits (threshold crossing)
  // Also, turn any notes off if it's time to do so
  for (int n = 0; n < n_sensors; n++){                
    // Read from sensor and (maybe) trigger a note
    sensorVals[n] = analogRead(sensorPins[n]);     
    if (sensorVals[n] > thresholds[n] && sensor_is_ready[n] == true) {            
      if (note_is_on[n] == true){
        sendNoteOff(pitches[n]);
      }
      
      vel = convertSignalToVelocity(sensorVals[n],thresholds[n]);      
      sendNoteOn(pitches[n], vel); // Turn the MIDI note on
      t_last_note[n] = millis();
      note_is_on[n] = true;
      sensor_is_ready[n] = false;
    }    

    // Check if the current sensor is ready to be reactivated    
    if (millis() - t_last_note[n] > t_between_notes){
      sensor_is_ready[n] = true;      
      
      // Turn off previous note if it's rang out long enough
      if (note_is_on[n] == true && ((millis() - t_last_note[n]) > t_duration)){     
        sendNoteOff(pitches[n]); // Turn off note
        note_is_on[n] = false;
      }      
    }
  } // End looping through checking each sensor  

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

int convertSignalToVelocity(double sig, const double threshold){    
  const int vel_min = 100;
  const int vel_max = 135;

  // Take the log of the raw EMG signal  
  const double sig_min = log(threshold);
  const double sig_max = log(1.3*threshold); // Guess  
  
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
