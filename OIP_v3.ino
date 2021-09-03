#include <AccelStepper.h>
#include <Servo.h>

#define motorPin1  8                // IN1 pin on the ULN2003A driver
#define motorPin2  9                // IN2 pin on the ULN2003A driver
#define motorPin3  10               // IN3 pin on the ULN2003A driver
#define motorPin4  11               // IN4 pin on the ULN2003A driver

Servo servo; 

int stepsPerRevolution = 64;        // steps per revolution
int degreePerRevolution = 5.625;    // degree per revolution
int angle = 0;

AccelStepper stepper(AccelStepper::HALF4WIRE, motorPin1, motorPin3, motorPin2, motorPin4);

unsigned long start_time = 0;
unsigned long duration = 0;
String process;
String data;
int high = 750;
int low = 300;
     
void setup() {
  Serial.begin(9600);  // Initialize serial

  pinMode(A0,INPUT); // water level sensor
  pinMode(12,OUTPUT); // USB fan
  pinMode(13,OUTPUT); // water pump
  pinMode(22,OUTPUT); // simulate on/off of ultrasonic cleaner
  pinMode(23,OUTPUT); // simulate on/off of heating element
  pinMode(24,OUTPUT); // simulate heating coil for fans
  pinMode(25,OUTPUT); // simulate standby mode/process is done

  digitalWrite(12,LOW);
  digitalWrite(13,LOW);
  digitalWrite(22,LOW);
  digitalWrite(23,LOW);
    
  stepper.setMaxSpeed(1000.0);      // set the max motor speed
  stepper.setAcceleration(200.0);   // set the acceleration
  stepper.setSpeed(200);            // set the current speed

  servo.attach(2);

}

void loop() {
  digitalWrite(25,HIGH); // indicates device is ready for use

  if (Serial.available()){
    process = Serial.readStringUntil('\n'); // read incoming serial data
  }

  if (process == "process1"){
    digitalWrite(25,LOW); // indicate device is in use
    Serial.println("ok"); // send acknowledgement to raspberry pi

    bool temp_status = false;
    duration = receive_duration(); // read duration of washing process 
    
    while (temp_status == false){
    
      pump_water(high); //pump water in
      delay(3000);
      ultrasonic_cleaner(duration); // simulate ultrasonic cleaner   
      delay(3000);
      drain_valve(high); // simulate drain valve

      delay(2000);
    
      pump_water(high); //pump water in
      delay(3000);
      drain_valve(high); // simulate drain valve

      Serial.println("washing completed"); // send message to raspberry pi

      while(!Serial.available());  
      data = Serial.readStringUntil('\n');
      if (data == "True"){
        temp_status = true;
        stepper.runToNewPosition(degToSteps(180));
        while(!Serial.available());
        data = Serial.readStringUntil('\n');
        if (data == "False"){
          temp_status = false;
        }        
      }
    }

    
    pump_water(low); //pump water in
    delay(3000);
    heating(); // simulate heating of water
    delay(3000);
    drain_valve(low); // simulate drain valve

    Serial.println("sterilization completed"); // send message to raspberry pi
    delay(3000);
    
    duration = receive_duration(); // read duration of washing process 

    while (duration != 0){
      delay(3000);
      digitalWrite(25,LOW);
      USB_fan(duration); // use USB fans for drying
      Serial.println("drying completed");
      digitalWrite(25,HIGH);
      
      duration = receive_duration();            
    }
    
    process = " ";    
  }

  if (process == "process2"){
    Serial.println("ok");
    digitalWrite(25,LOW);

    pump_water(low); //pump water in
    delay(3000);
    heating(); // simulate heating of water
    delay(3000);
    drain_valve(low); // simulate drain valve

    Serial.println("sterilization completed"); // send message to raspberry pi
    delay(3000);
    
    duration = receive_duration(); // read duration of washing process 

    while (duration != 0){
      delay(3000);
      digitalWrite(25,LOW);
      USB_fan(duration); // use USB fans for drying
      Serial.println("drying completed");
      digitalWrite(25,HIGH);
      
      duration = receive_duration();            
    }
    
    process = " ";  
  }
}


float degToSteps(float deg) // function to convert deg to steps
{
  return (stepsPerRevolution / degreePerRevolution) * deg;
}

int receive_duration(){
  while(!Serial.available());  
  data = Serial.readStringUntil('\n');
  duration = 1000 * data.toInt();
  return duration;
}

void pump_water(int level){
    digitalWrite(13,HIGH);
    while (analogRead(A0)<level);
    digitalWrite(13,LOW);  
}

void ultrasonic_cleaner(int duration){
    Serial.println("start");
    digitalWrite(22,HIGH);
    start_time = millis();
    while (millis() - start_time < duration);
    digitalWrite(22,LOW);  
}

void drain_valve(int level){
    for(angle = 0; angle < 90; angle += 1)     
    {                                  
      servo.write(angle);                
      delay(15);                       
    } 
    
    if (level == 750){
      delay(5000);
    }
    else if (level == 300){
      delay(3000);
    }
    
    for(angle = 90; angle>=0; angle-=1)     
    {                                
      servo.write(angle);              
      delay(15);                       
    } 
}

void heating(){
    duration = 10000;
    Serial.println("start");
    digitalWrite(23,HIGH);
    start_time = millis();
    while (millis() - start_time < duration);
    digitalWrite(23,LOW);  
}

void USB_fan(int duration){
    Serial.println("start");
    start_time = millis();
    digitalWrite(24,HIGH);
    digitalWrite(12,HIGH);
    while (millis() - start_time < duration);
    digitalWrite(12,LOW);  
    digitalWrite(24,LOW);
}
