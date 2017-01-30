// #include <FilterOnePole.h>
#include <Filters.h>

#define LED 13

// Sensor pin
const int sensorPin = A5;
double sensorVal = 0;
double volts = 0;

// Threshold
double noiseFloor = 0;
double threshold = 0;
bool over_thresh = false;

// MIDI Stuff
const int n_sensors = 2;
int pitches[n_sensors] = {50, 37};
int noteVel = 0;
bool note_is_on = false;

// Timing stuff
double t_prev_note[n_sensors] = {0, 0};
int t_between_notes = 200; // Min. time until next trigger from sensor (ms)

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

// ----------------------------------------------------------------
void setup() {  
  pinMode(LED, OUTPUT);
  Serial.begin(31250);
  // Serial.begin(9600);

  noiseFloor = measureBaseline();
  threshold = 1.3 * noiseFloor;
    
}

// ----------------------------------------------------------------
void loop() {        
    sensorVal = sampleValues(3.0);
    // Serial.println(sensorVal);

    if (sensorVal > threshold && (millis() - t_prev_note[0]) > t_between_notes)
    {
      digitalWrite(LED, HIGH);
      sendNoteOn(pitches[0], 100);
      t_prev_note[0] = millis();
      note_is_on = true;
    }
    else if (note_is_on)
    {
      digitalWrite(LED, LOW);
      sendNoteOff(pitches[0]);
      note_is_on = false;
    }

    while (sensorVal > threshold) {
      sensorVal = sampleValues(3.0);
      delay(1);
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

double convertToVoltage(double analogVal)
{
  return 5.0 * (analogVal / 1023.0);
}

int convertToVel(double raw, int threshold) {
  double outputVel;
  double min = 60;
  double max = 127;
  // outputVel = floor(raw/8.0+10);
  outputVel = floor(max * (raw - threshold) / (max - min));
  if (outputVel >= 115) {
    outputVel = max;
  }

  return outputVel;
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
