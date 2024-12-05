#include "HX711.h"
#include "pitches.h"

HX711 scale;

const int loadcell_clk = 2;
const int loadcell_dout = 3;
const int red_led = 5;
const int yellow_led = 4;
const int green_led = 6;
const int buzzer = 9;

// hx711 & water data setup
float scale_factor = 442;  //rawvalue/knownweight
float last_weight = 0.0;
float total_filled = 0.0;
float total_consumed = 0.0;

// goal and thresholds for leds
const float daily_goal = 510.0;
const float green_threshold = daily_goal;
const float yellow_threshold = daily_goal*0.5;

// reminder interval (rtclib millis () returns milisecond)
const unsigned long reminder_interval = 3600000; //1 hour. 1 min = 60000
unsigned long previous_time = 0;

// buzzer setup. Current melody: A section of Pirate of the Caribeans ost
int melody[] = {NOTE_E4, NOTE_G4, NOTE_A4, NOTE_A4, REST,
  NOTE_A4, NOTE_B4, NOTE_C5, NOTE_C5, REST,
  NOTE_C5, NOTE_D5, NOTE_B4, NOTE_B4, REST,
  NOTE_A4, NOTE_G4, NOTE_A4, REST
};

int durations[] = {8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8, 8,
  8, 8, 4, 8
};

int size = sizeof(durations)/sizeof(int); ////amount of memory allocated to int type (= 4 bytes)


void setup() {
  Serial.begin(9600);
 
  // leds & buzzer
  pinMode(red_led, OUTPUT);
  pinMode(yellow_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(buzzer, OUTPUT);

  // Start with red on, others off
  digitalWrite(red_led, HIGH);
  digitalWrite(yellow_led, LOW);
  digitalWrite(green_led, LOW);

  // hx711
  scale.begin(loadcell_dout, loadcell_clk);
  scale.set_scale(scale_factor);
  scale.tare(); //reset scale to 0
}

void loop() {
  // get current weight
  float current_weight = scale.get_units(10); //avg of 10 times
  if (current_weight < 0) {current_weight = 0;} //ignore negative values
  
  // if (current_weight > last_weight) {
  //   total_filled = current_weight;
  //   }

  // detect if water consumed
  if (current_weight < last_weight) {
    total_consumed += last_weight - current_weight;
  }

  last_weight = current_weight;

  turn_led();
  
  // play reminder
  if (millis() - previous_time >= reminder_interval && total_consumed < green_threshold) {
    play_reminder();
  }

}

void turn_led() {
  // turn on green
  if (total_consumed >= green_threshold) {
    digitalWrite(red_led, LOW);
    digitalWrite(yellow_led, LOW);
    digitalWrite(green_led, HIGH);
  }
  // turn on yellow
  else if (total_consumed < green_threshold && total_consumed >= yellow_threshold) {
    digitalWrite(red_led, LOW);
    digitalWrite(green_led, LOW);
    digitalWrite(yellow_led, HIGH);
  }
}

void play_reminder() {
  // calculate note duration: 1 second/note
  // e.g: quarter note = 1000/4
  for (int note = 0; note < size; note++) {
    int note_duration = 1000/durations[note];
    tone(buzzer, melody[note], note_duration);

  // set a minimum time between the note to distinguish them
  // current setting: note's duration + 30%
  int pause = note_duration * 1.30;
  delay(pause);

  // stop sound
  noTone(buzzer); 
  }

}
