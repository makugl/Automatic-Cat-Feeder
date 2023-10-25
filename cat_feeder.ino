/*+++ DEFINIES +++*/
#define LED_GREEN 12
#define LED_RED 13
#define TASTER_PIN 11
#define PRESSURE_PIN A0
#define DISTANCE_TRIGGER 7
#define DISTANCE_ECHO 8

#define CAT_DISATNCE_TRIGGER_CM 10
#define SERVING_FOOD_DURATION_S 6
#define SERVING_TRIGGER_DURATION_S 3
#define UNLIMITED_WAITING 0
#define FEEDER_TIMEOUT_S 10

#define THRESHHOLD_FOOD_EMPTY 10
#define THRESHHOLD_FOOD_ALMOST_EMPTY 20
#define THRESHHOLD_FOOD_ALMOST_FULL 40
#define THRESHHOLD_FOOD_FULL 50

#define RGB_GREEN 10
#define RGB_RED 9
#define RGB_BLUE 6

#define DISTANCE_SENSOR_MAX_RANGE 300

int pressure_value = 150;
unsigned long duration;
unsigned long distance;

/* +++ STATES +++ */
enum FOODLEVEL {
  EMPTY,
  ALMOST_EMPTY,
  ALMOST_FULL,
  FULL
};
FOODLEVEL foodlevel = FOODLEVEL::EMPTY;

enum STATES {
  IDLE,
  FEEDER_READY,
  FEEDER_WAITING,
  FEEDER_EMPTY
};
STATES state = STATES::IDLE;

/*=======================================================================*/
/* +++ TEST-FUNCTIONS +++ */
void distance_test()
{
  digitalWrite(DISTANCE_TRIGGER, LOW); 
  delayMicroseconds(2);
  digitalWrite(DISTANCE_TRIGGER, HIGH);
  delayMicroseconds(10);

  duration = pulseIn (DISTANCE_ECHO, HIGH);
  distance = duration / 58;
  if (distance > DISTANCE_SENSOR_MAX_RANGE || distance <= 0){
    Serial.println("Ausserhalb der Reichweite!"); 
    }else{
    Serial.println("Entfernung: " + String(distance) + " cm");
  }
  delay (1000);
}

void pressure_test()
{
  //PRESSURE TEST
  pressure_value = analogRead(PRESSURE_PIN);
  Serial.print("Analoges Signal = ");
  Serial.print(pressure_value);

  if(pressure_value < 10){
    Serial.println(" − kein Druck"); 
  }
  else if (pressure_value < 400){ 
    Serial.println(" − leichter Druck"); 
  }
  else if (pressure_value < 500){ 
    Serial.println(" − mittlerer Druck"); 
    }
  else {
    Serial.println(" − grosser Druck");
  }
  delay(500);
}

void led_test()
{
  //LED TEST
  analogWrite(LED_GREEN, HIGH);
  delay(1000);

  analogWrite(LED_GREEN, LOW);
  delay(1000);
}

void taster_test()
{
  if(digitalRead(TASTER_PIN) == HIGH){
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.print("TASTER");
    }else{
      digitalWrite(LED_BUILTIN, LOW);
    }

}

void rgb_test()
{
  analogWrite(RGB_RED, 0);
  analogWrite(RGB_BLUE, 255);
  analogWrite(RGB_GREEN, 0);
}

/*=======================================================================*/
/*+++ HELPER FUNCTIONS +++*/
//this funktion controls the led while feeding the cat. 
//The led will blink until serving food is done
void serving_food(unsigned int duration_seconds, unsigned int LED_PIN)
{
  if(LED_PIN == LED_GREEN){
    digitalWrite(LED_RED, LOW);
  }
  else if(LED_PIN == LED_RED){
    digitalWrite(LED_GREEN, LOW);
  }else{
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);
  }

  long myTimer = millis();
  long myTimeout = duration_seconds * 1000; //seconds to milliseconds

  while(millis() < (myTimeout + myTimer)){
    if(millis() % 1000 > 500){
      digitalWrite(LED_PIN, HIGH);
    }else{
      digitalWrite(LED_PIN, LOW);
    }
  }
}

//wait until food got some refill
void food_is_empty()
{
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);

  while (foodlevel != FOODLEVEL::ALMOST_FULL)
  {
    //waiting for food refill. 
    //red led will blink until refill is done...
    if(millis() % 1000 > 500){
      digitalWrite(LED_RED, HIGH);
    }else{
      digitalWrite(LED_RED, LOW);
    }
    update_food_level();
  }
}

//function for blocked-waiting
void feeder_timeout(unsigned int timeout_seconds)
{
  long myTimer = millis();
  long myTimeout = timeout_seconds * 1000; //seconds to milliseconds

  while(millis() < (myTimeout + myTimer)){
    //waiting...
  }
}

//check if cat is waiting for food.
//to do that we will work with the distance sensor
bool cat_waiting_for_food(unsigned int timeout_seconds)
{
  digitalWrite(LED_RED, LOW);
  digitalWrite(LED_GREEN, LOW);   

  bool cat_was_waiting = true;
  long myTimer = millis();
  long myTimeout = timeout_seconds * 1000; //seconds to milliseconds

  while(millis() < (myTimeout + myTimer)){
    if(millis() % 1000 > 500){
      digitalWrite(LED_RED, HIGH);
      digitalWrite(LED_GREEN, LOW);
    }else{
      digitalWrite(LED_RED, LOW);
      digitalWrite(LED_GREEN, HIGH);
    }

    update_distance();
    if(distance > 10){
      cat_was_waiting = false;
      break;
    }
  }
  return cat_was_waiting;
}

//get current pressure on pressure sensor
void update_pressure()
{
  pressure_value = analogRead(PRESSURE_PIN);
    Serial.print("Pressure: ");
    Serial.print(pressure_value);
    Serial.print("\n");
}

//get current distance from distance sensor
void update_distance()
{
  digitalWrite(DISTANCE_TRIGGER, LOW); 
  delayMicroseconds(2);
  digitalWrite(DISTANCE_TRIGGER, HIGH);
  delayMicroseconds(10);
  duration = pulseIn (DISTANCE_ECHO, HIGH);
  distance = duration / 58;
}

//set values of our rgb led
void set_rgb_led()
{
  if(foodlevel == FOODLEVEL::FULL){ //green
    analogWrite(RGB_RED, 0);
    analogWrite(RGB_BLUE, 0);
    analogWrite(RGB_GREEN, 255);
  }
  else if(foodlevel == FOODLEVEL::ALMOST_FULL){ //blue
    analogWrite(RGB_RED, 0);
    analogWrite(RGB_BLUE, 255);
    analogWrite(RGB_GREEN, 0);
  }
  else if(foodlevel == FOODLEVEL::ALMOST_EMPTY){ //orange
    analogWrite(RGB_RED, 255);
    analogWrite(RGB_BLUE, 165);
    analogWrite(RGB_GREEN, 0);
  }
  else if(foodlevel == FOODLEVEL::EMPTY){ //red
    analogWrite(RGB_RED, 255);
    analogWrite(RGB_BLUE, 0);
    analogWrite(RGB_GREEN, 0);
  }
}

//check how much food is in our feeder.
//change state an color of rgb led if the amout of food is not enough
void update_food_level()
{
  update_pressure();
  if(pressure_value <= THRESHHOLD_FOOD_EMPTY){
    foodlevel = FOODLEVEL::EMPTY;
    state = STATES::FEEDER_EMPTY;
  }
  else if(pressure_value <= THRESHHOLD_FOOD_ALMOST_EMPTY){
    foodlevel = FOODLEVEL::ALMOST_EMPTY;
  }
  else if(pressure_value <= THRESHHOLD_FOOD_ALMOST_FULL){
    foodlevel = FOODLEVEL::ALMOST_FULL;
  }
  else if(pressure_value <= THRESHHOLD_FOOD_FULL){
    foodlevel = FOODLEVEL::FULL;
  }
  set_rgb_led();
}

/*=======================================================================*/

void setup() {
  Serial.begin (9600) ; 
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);

  pinMode(TASTER_PIN, INPUT);
  
  pinMode(DISTANCE_TRIGGER, OUTPUT);
  pinMode(DISTANCE_ECHO, INPUT);

  Serial.println("SERIAL TEST!");
}

void loop() {
  //test functions for every single component
  //led_test();
  //pressure_test();
  //distance_test();
  //taster_test();
  //rgb_test();

  /* +++ STATE-MACHINE +++ */
  switch (state)
  {
  case STATES::IDLE:
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, LOW);

    //CHECK FOR FOOD LEVEL
    update_food_level();
    if(foodlevel == FOODLEVEL::EMPTY){
        state = STATES::FEEDER_EMPTY;
      }
    else if(digitalRead(TASTER_PIN) == HIGH){
        state = STATES::FEEDER_READY;
    }
    break;


  case STATES::FEEDER_READY:
    digitalWrite(LED_RED, LOW);
    digitalWrite(LED_GREEN, HIGH);

    //check food status
    update_food_level();

    //check if cat is coming for some food
    update_distance();
    if (distance > DISTANCE_SENSOR_MAX_RANGE || distance <= 0){
      Serial.println("Ausserhalb der Reichweite!");
    }else if(distance <= CAT_DISATNCE_TRIGGER_CM){
      if(cat_waiting_for_food(/*duration in secoond*/ SERVING_TRIGGER_DURATION_S)){
        serving_food(/*duration in secoond*/ SERVING_FOOD_DURATION_S, /*LED_PIN*/ LED_GREEN);
        state = STATES::FEEDER_WAITING;
      }
    }else{
      Serial.println("Entfernung: " + String(distance) + " cm");
    }
    break;


  case STATES::FEEDER_WAITING:
    digitalWrite(LED_GREEN, LOW);
    digitalWrite(LED_RED, HIGH);
    update_food_level();

    //wait until defined time is over
    feeder_timeout(FEEDER_TIMEOUT_S);
    state = STATES::FEEDER_READY;
    break;


  case STATES::FEEDER_EMPTY:
    //state will not be exited until feeder got some refill
    food_is_empty();
    state = STATES::FEEDER_READY;
    break;
  

  default:
  //in case will go into this state --> write a huge error message because its undefined
    Serial.print("\n\n\nCASE DEFAULT ---> UNHANDLED CASE !!!!! \n\n\n");
    break;
  }
  delay(250);
}
