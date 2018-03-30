/*
 This uses some example code from J. Coliz <maniacbug@ymail.com> for the nRF24L01+ radios,
 to communicate between two arduinos.
 Used in the IEEE 2017 Hardware hackathon to create a bot using two Arduinos and the 
 nRF24L01+ chip to communicate back and forth the distance of objects using rangefinders.
 Makes sounds when objects get too close.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include <SevSeg.h>

RF24 radio(9,10);
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };
typedef enum { role_ping_out = 1, role_pong_back } role_e;
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};
role_e role = role_pong_back;
int counter = 0;
int toPlay = 0;
bool on = false;
SevSeg sevseg;
int toDisplay = 0;
bool sound = false;

void setup(void)
{
  pinMode(2, INPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  role = role_pong_back;
  
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("* PRESS 'T' to begin transmitting to the other node\n\r");
  radio.begin();
  radio.setRetries(15,15);
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
  radio.startListening();
  radio.printDetails();
  
  byte numDigits = 4;   
  byte digitPins[] = {A2, A3, A4, A5};
  byte segmentPins[] = {0, 1, 5, 6, 7, 8, A0, A1}; //Segments: A,B,C,D,E,F,G,Period
  sevseg.begin(COMMON_ANODE, numDigits, digitPins, segmentPins);
  sevseg.setBrightness(10);
  
}

void loop(void)
{
  
  static byte decPlace = 0;
  //code for the seven segment display
  sevseg.setNumber(toDisplay,1);
  decPlace++;
  decPlace %= 4; 
  sevseg.refreshDisplay();
  printf("\n");
  Serial.print(digitalRead(2));
  printf(" ");
  Serial.print(on);
  printf("\n");
  
  digitalWrite(4, HIGH);
  if (on == false && digitalRead(2) == HIGH)
  {
    sevseg.setNumber(0000,1);
    sevseg.refreshDisplay();
    return;
  }
  if (on == true && digitalRead(2) == LOW)
  {
    while (digitalRead(2) == LOW) {}
    on = false;
    return;
  } 
  if (on == false && digitalRead(2) == LOW)
  {
    while (digitalRead(2) == LOW) {}
    on = true;
  }
  
  printf("\n");
  Serial.print(counter);
  printf("\n");
  
  counter++;
  if (counter > toPlay && toPlay > 0)
  {
    int pitch = (sound)?1319:440;
    sound = !sound;
    //plays the sound with tone
    tone(3, pitch, 250);
    counter = 0;
  }
  else if (counter > 15)
  {
    noTone(3);
  }
  if (role == role_pong_back )
  {
    if ( radio.available() )
    {
      char distance[10];
      bool done = false;
      while (!done)
      {
        done = radio.read(&distance, sizeof(distance));
        printf(distance);
        delay(20);
      }
      toDisplay = (int)10*String(distance).toFloat();
      //this is the string form of the distance.
      toPlay = (String(distance).toFloat() > 100)?-1:String(distance).toFloat()*6;
      printf("\n");
      Serial.print(toPlay);
      printf("\n");
      radio.stopListening();
      radio.write( &distance, sizeof(distance));
      printf("Sent response.\n\r");
      radio.startListening();
    }
  }
}
// vim:cin:ai:sts=2 sw=2 ft=cpp

