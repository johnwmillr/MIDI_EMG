#define LED 13

// Sensor pin
const int sensorPin = A5;
double sensorVal = 0;
double volts = 0;

// Threshold
double threshold = 0.75; // Volts
bool over_thresh = false;

// MIDI Stuff
const int n_sensors = 2;
int pitches[n_sensors] = {50, 37};
int noteVel = 0;
bool note_is_on = false;

// Timing stuff
double t_prev_note[n_sensors] = {0, 0};
int t_between_notes = 250; // Min. time until next trigger from sensor (ms)

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  Serial.begin(31250);
  //  Serial.begin(9600);
  threshold = threshold * (1023 / 5.0);
}

// ----------------------------------------------------------------
void loop() {
  // put your main code here, to run repeatedly:

  sensorVal = analogRead(sensorPin);

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
    sensorVal = analogRead(sensorPin);
    delay(1);
  }
  delay(1);
}

// -------------------------------------------------------------------
// Define the internal functions for MIDI communication
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

double convertToVoltage(int analogVal)
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

