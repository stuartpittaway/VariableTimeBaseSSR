/*
Latest version found at https://github.com/stuartpittaway/VariableTimeBaseSSR

 
Burst Fire Variable Time Base SSR controller (for Arduino)


Note that this code ignores the zero cross delay/miss fire of the SSR !!!


A SSR power controller with a "variable time base" changes the time base according to the power requirement. Burst ﬁring with a variable time base usesthe
smallest possible number of AC cycles to deliver the required percentage power to the heater. 


Why is variable time base preferred over a ﬁxed time base? 
The ON/ OFF switching of the heater happens much more quickly with variable time base. The more quickly the heater is switched, the less temperature variations 
the resistance element has. The nearly constant load current to the heater keeps the heater’s resistance element temperature nearly constant. 
This provides a longer heater life.



LICENCE
 Attribution-NonCommercial-ShareAlike 3.0 Unported (CC BY-NC-SA 3.0)
 http://creativecommons.org/licenses/by-nc-sa/3.0/
 You are free:
 to Share — to copy, distribute and transmit the work
 to Remix — to adapt the work
 Under the following conditions:
 Attribution — You must attribute the work in the manner specified by the author or licensor (but not in any way that suggests that they endorse you or your use of the work).
 Noncommercial — You may not use this work for commercial purposes.
 Share Alike — If you alter, transform, or build upon this work, you may distribute the resulting work only under the same or similar license to this one.
 
 All code is copyright Stuart Pittaway, (c)2012.
*/


#define relay_pin A1
#define led_pin 6

#define halfcyclems 1000/50/2    //1000ms / 50hz / half

#define ssr_on digitalWrite(relay_pin,HIGH);currentssrstate=true;
#define ssr_off digitalWrite(relay_pin,LOW);currentssrstate=false;

unsigned long current_time;
char c;
int n=0;
bool currentssrstate=false;

int max_load_power = 3300;  //Maximum watts of power for the load (eg. 3kw water heater)

//This table is an attempt to provide a variable time base to fire the SSR device rather than a straight on/off delay (fixed time base)
//Cycle over 16 bits/mains half cycles (we hope!)
//Can we create this table dynamically ??
//1=SSR on
unsigned int switchingtable[] = {
  (B00000000 * 256) + B00000000,  //0%
  (B10000000 * 256) + B10000000,  //10% = 12.5%
  (B10000001 * 256) + B00001000,  //20% = 20.00%
  (B10010001 * 256) + B00100010,  //30% = 31.25%
  (B10101100 * 256) + B00110010,  //40% = 43.75%
  (B10101010 * 256) + B10101010,  //50% = 50.00%
  (B11101010 * 256) + B11101010,  //60% = 62.50%
  (B11101010 * 256) + B11101011,  //70% = 68.75%
  (B11101110 * 256) + B11101111,  //80% = 81.25%
  (B11111110 * 256) + B11111110,  //90% = 87.50%
  (B11111111 * 256) + B11111111  //100%
};


void setup()
{
  Serial.begin(115200);          //Serial port for debugging output

  pinMode(relay_pin, OUTPUT);
  pinMode(led_pin, OUTPUT);

  ssr_off;

  digitalWrite(led_pin,HIGH);  //Nanode, switch LED off!  

  Serial.println(F("Press letters a-k to control power level (a=0%, k=100%)"));
}

//JUst for debugging
bool oneloopdone=false;

void loop()
{ 
  current_time = millis();

  //Reading input from keyboard a-z a=0 (off) bcdefghij k = 100%
  if (Serial.available())
  {
    c = Serial.read()- 'a';
    if (c>=0 && c<=10) {
      n = c;
      //Serial.println(switchingtable[n],BIN);      
      Serial.print(F("Using value "));
      Serial.print(n*10);
      Serial.print('%');
      oneloopdone=false;
    }
  }

  //Loop around the selected frequency switching SSR on/off as needed
  unsigned int c=32768;
  
  byte onperiods=0;
  
  for(byte bitNumber=16;bitNumber>0;bitNumber--) {     
    bool value = (switchingtable[n] & c)>0;

    if (value) {
      ssr_on;
      onperiods++;
    } 
    else {
      ssr_off;
    }

    c=c >> 1;  //Divide c by 2

    //We should use the millis timer here instead of hogging the CPU
    delay(halfcyclems);
  }
  
  if (!oneloopdone) {
    Serial.print('=');
    Serial.print(100*((float)onperiods/(float)16));
    Serial.println('%'); 
     oneloopdone=true;
  } 
}



