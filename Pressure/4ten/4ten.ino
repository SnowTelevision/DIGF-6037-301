/*
Creation & Computation
Experiment 1: Pressure

4ten

The code follows a loop (I will address each loop as a "session") that will go through different states through a switch case. 
Prior to each input state, there will always be a calibration state. 
At the beginning of the session, the initialization state will count how many users are joining this session by taking tap input from a user. 
Next, the session will loop through a "counting" state for each user. When a user starts pressing down on the cardboard button, 
the counting will start. If the user holds the button for less than 10 seconds, the duration will be saved for the user. 
If the user holds for more than 10 seconds, the program will display a "bust" signal through the LEDs, 
and save 0 seconds as the duration for the user. 
After every user in the session is finished, the LED corresponding to the user with the longest duration will light up, 
indicating the "winner" for this session. Then it will loop back to the initialization state and be ready for a new session.

Credits:

  "Arrays" example

  created 2006
  by David A. Mellis
  modified 30 Aug 2011
  by Tom Igoe

  This example code is in the public domain.

  https://www.arduino.cc/en/Tutorial/BuiltInExamples/Arrays

*/

int mode = 0;  // Which mode is the program currently at
// 0: initializing
// 1: initializing - end
// 2: calibrating - wait
// 3: calibrating
// 4: calibrating - end
// 5: counting - wait
// 6: counting
// 7: counting - end
// 8: display result
int savedTime = 0;                 // Save a timestamp
bool sensorDown = false;           // Is the sensor pressed
int userCount = 0;                 // How many user is using (max 4)
int countings[] = { 0, 0, 0, 0 };  // Counter for each user
int waitInputLedPin = 2;           // Pin# for the LED that indicating waiting for input
int confirmInputLedPin = 3;        // Pin# for the LED that confirm a user input
int userLED[] = { 4, 5, 6, 7 };    // Pin#s for LED displaying result for the users
bool initialized = false;          // Is it initialized
int idleSensor = 0;                // Sensor's reading while idle
int currentUser = 0;               // Which user is currently counting

int sensorPin = A0;  // pin that the sensor is attached to
int sensorMin = 1023;
int sensorMax = 0;
int sensorValue = 0;  // raw analog sensor reading
int ledPins[] = {
  2, 3, 4, 5, 6, 7
};                 // an array of pin numbers to which LEDs are attached
int pinCount = 6;  // the number of pins (i.e. the length of the array)

void setup() {
  // put your setup code here, to run once:
  // the array elements are numbered from 0 to (pinCount - 1).
  // use a for loop to initialize each pin as an output:
  for (int thisPin = 0; thisPin < pinCount; thisPin++) {
    pinMode(ledPins[thisPin], OUTPUT);
  }

  mode = 2;  // Start with calibration mode
}

void loop() {
  // put your main code here, to run repeatedly:
  switch (mode) {
    case 0:
      Initializing();
      break;
    case 1:
      Halt(3000, 2);
      break;
    case 2:
      CalibrationWaitInput();
      break;
    case 3:
      Calibration();
      break;
    case 4:
      Serial.println("Halt for 3 sec");
      // If not initialized
      if (!initialized) {
        Halt(3000, 0);
      } else {
        Halt(3000, 5);
      }
      break;
    case 5:
      CountWaitInput();
      break;
    case 6:
      Counting();
      break;
    case 7:
      EndCounting();
      break;
    case 8:
      Result();
      break;
  }
}

// 8 Display result
void Result() {
  int winner = 0;   // who counted the closest
  int highest = 0;  // highest user "score"
  for (int i = 0; i < userCount - 1; i++) {
    if (countings[i] > highest) {
      highest = countings[i];
      winner = i;
    }
  }

  // If there is a winner
  if (highest > 0) {
    digitalWrite(userLED[winner], HIGH);
  }

  // Wait 5 sec to cheer for the winner
  savedTime = millis();
  while (millis() < savedTime + 5000) {
  }

  SwitchLEDs(userLED, false);
  initialized = false;  // Reset game
  mode = 2;
}

// 5 Wait for user input to start counting
void CountWaitInput() {
  digitalWrite(waitInputLedPin, HIGH);

  // If detect sudden pressure jump
  if (analogRead(sensorPin) > sensorMin + 200) {
    digitalWrite(confirmInputLedPin, HIGH);
    savedTime = millis();  // Save start counting time
    mode = 6;              // Move to counting
  }
}

// 6 Counting
void Counting() {
  // If user still pressing
  if (analogRead(sensorPin) > sensorMin + 200) {

    // If user hold more than 10 sec
    if (millis() > savedTime + 10000) {
      savedTime = millis();
      SwitchLEDs(userLED, true);
      while (millis() < savedTime + 2000) {
      }
      SwitchLEDs(userLED, false);
      mode = 7;
    }
  } else {
    digitalWrite(confirmInputLedPin, LOW);
    countings[currentUser] = millis() - savedTime;
    mode = 7;
  }
}

// 7 Counting end
void EndCounting() {
  digitalWrite(waitInputLedPin, LOW);
  digitalWrite(confirmInputLedPin, LOW);
  currentUser++;  // Move to next user
  // If all user have finished
  if (currentUser > userCount - 1) {
    Halt(3000, 8);
  } else {
    Halt(3000, 2);
  }
}

// 2 Wait for user input while recording the idle read
void CalibrationWaitInput() {
  digitalWrite(waitInputLedPin, HIGH);
  //Serial.print(analogRead(sensorPin));  // Debug
  //Serial.print("\n ");                  // Debug

  // If detect sudden pressure jump
  if (analogRead(sensorPin) > 200) {
    sensorMax = 0;
    sensorMin = idleSensor;  // Reset sensor calibration
    Serial.println(sensorMin);
    mode = 3;  // Move to calibration
    Serial.println("Calibration Start");
  }

  // record the maximum idle sensor value
  if (analogRead(sensorPin) > idleSensor) {
    idleSensor = analogRead(sensorPin);
  }
}

// 3 Calibrating process
void Calibration() {
  savedTime = millis();

  while (millis() < savedTime + 3000) {
    sensorValue = analogRead(sensorPin);
    //Serial.println(sensorValue);  // Debug
    // record the maximum sensor value
    if (sensorValue > sensorMax) {
      sensorMax = sensorValue;
      digitalWrite(confirmInputLedPin, HIGH);
    }

    // record the minimum sensor value
    if (sensorValue < sensorMin) {
      sensorMin = sensorValue;
      digitalWrite(confirmInputLedPin, HIGH);
    }

    digitalWrite(confirmInputLedPin, LOW);
  }

  //Serial.println(savedTime + ", " + millis());  // Debug
  idleSensor = 0;  // Reset pressure sensor idle reading
  mode = 4;        // Move to calibrating finish
  Serial.println("Calibration Finish");
}

// 0 Set up how many users are joining (max 4)
void Initializing() {
  digitalWrite(waitInputLedPin, HIGH);

  // If it is not initialized
  if (!initialized) {
    Serial.println("start initializaion");
    userCount = 0;
    //Serial.println(userCount);  // debug
    // Reset user countings
    for (int i = 0; i < sizeof(countings); i++) {
      countings[i] = 0;
    }
    currentUser = 0;
    initialized = true;
  }

  // If the sensor is pressed
  if (analogRead(sensorPin) > sensorMin + 200) {
    // Serial.println("sensor pressing");
    // If the sensor is just pressed
    if (!sensorDown) {
      sensorDown = true;
      digitalWrite(confirmInputLedPin, HIGH);  // Confirm user input
      userCount++;                             //
      Serial.println(sensorMin);
      Serial.println(analogRead(sensorPin));
      Serial.print("player count: ");
      Serial.println(userCount);
    }
  } else {
    // If the sensor is just released
    if (sensorDown) {
      Serial.println(sensorMin);
      Serial.println(analogRead(sensorPin));
      Serial.println("sensor lift");
      sensorDown = false;
      digitalWrite(confirmInputLedPin, LOW);  // Confirm user input
      savedTime = millis();
    }

    // If at least one user is joined
    if (userCount >= 1) {
      // If input timeout or reached max user count
      if (millis() - savedTime >= 3000 || userCount >= 4) {
        Serial.print("total player count: ");
        Serial.println(userCount);
        Serial.println("initialization done");
        mode = 1;  // Progress the mode
      }
    }
  }
}

// Halt for the specified duration, then enter the specified mode
void Halt(int duration, int nextMode) {
  savedTime = millis();
  digitalWrite(waitInputLedPin, LOW);  // Indicate user need to wait
  while (millis() - savedTime < duration || analogRead(sensorPin) > sensorMin + 200) {
    if (analogRead(sensorPin) > sensorMin + 200) {
      digitalWrite(confirmInputLedPin, HIGH);
    } else {
      digitalWrite(confirmInputLedPin, LOW);
    }
  }
  digitalWrite(confirmInputLedPin, LOW);
  mode = nextMode;
}

// Switch on/off specified LEDs
void SwitchLEDs(int pins[], bool on) {
  for (int i = 0; i < sizeof(pins); i++) {
    if (on) {
      digitalWrite(pins[i], HIGH);
    } else {
      digitalWrite(pins[i], LOW);
    }
  }
}
