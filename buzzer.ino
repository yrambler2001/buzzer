const int pinDoor = 4;
const int pinPower = 5;
const int pinBuzzer = 6;

int doorClosedPrevious = 2;                  // 0 = door open, 1 = door closed, 2 = undefined
unsigned int doorClosedLastSecondTimes = 0;

int powerOnPrevious = 2;  // 0 = power off, 1 = power on, 2 = undefined
unsigned int powerOnLastSecondTimes = 0;

unsigned int washingSeconds = 0;

int currentlyBeeping = 0;                      // 0 = silence (currently washing), 1 = performing beeping procedure (washing completed)
int currentlySecondsAfterFinishedWashing = 0;  // counter of seconds after washing completion

int beepIntervalSeconds[] = {
  0,
  0 + 0.5 * 60,
  0 + 0.5 * 60 + 1 * 60,
  0 + 0.5 * 60 + 1 * 60 + 2 * 60,
  0 + 0.5 * 60 + 1 * 60 + 2 * 60 + 3 * 60,
  0 + 0.5 * 60 + 1 * 60 + 2 * 60 + 3 * 60 + 5 * 60,
  0 + 0.5 * 60 + 1 * 60 + 2 * 60 + 3 * 60 + 5 * 60 + 8 * 60,
};  // modified fibonacci


void setup() {
  pinMode(pinDoor, INPUT_PULLUP);
  pinMode(pinPower, INPUT_PULLUP);
  pinMode(pinBuzzer, OUTPUT);
  Serial.begin(250000);  // https://arduino.stackexchange.com/a/299
  Serial.println("Init. Idea: 22.12.2024. Firmware: 23.12.2024.");
}

void beep() {
  Serial.println("beep()");
  tone(pinBuzzer, 4000);
  Serial.println("beep() end");
}
void stopBeeping() {
  Serial.println("stopBeeping()");
  noTone(pinBuzzer);
  Serial.println("stopBeeping() end");
}

void loop() {
  Serial.println("---------");
  Serial.println("Loop start");
  unsigned long previousMillis = millis();
  Serial.println("Millis: ");
  Serial.println(previousMillis);
  doorClosedLastSecondTimes = 0;
  powerOnLastSecondTimes = 0;
  while (millis() - previousMillis < 1000) {       // https://arduino.stackexchange.com/a/12591
    int pinDoorStateRead1 = digitalRead(pinDoor);  // 5us
    int pinDoorStateRead2 = digitalRead(pinDoor);  // 5us
    int pinDoorStateRead3 = digitalRead(pinDoor);  // 5us
    if ((pinDoorStateRead1 == pinDoorStateRead2) && (pinDoorStateRead2 == pinDoorStateRead3)) {
      if (pinDoorStateRead1 == 0) doorClosedLastSecondTimes++;
    }

    int pinPowerStateRead1 = digitalRead(pinPower);  // 5us
    int pinPowerStateRead2 = digitalRead(pinPower);  // 5us
    int pinPowerStateRead3 = digitalRead(pinPower);  // 5us
    if ((pinPowerStateRead1 == pinPowerStateRead2) && (pinPowerStateRead2 == pinPowerStateRead3)) {
      if (pinPowerStateRead1 == 0) powerOnLastSecondTimes++;
    }
  }
  Serial.println("powerOnLastSecondTimes:");
  Serial.println(powerOnLastSecondTimes);
  Serial.println("doorClosedLastSecondTimes:");
  Serial.println(doorClosedLastSecondTimes);


  // value is either 0 or near 2300 (when 125Hz) and 9300 (when 500Hz). 34000 is [invalid] maximum (when every reading is matched).
  // 200 is guaranteed to be within acceptable bounds 0 < 200 < 2300.
  int doorClosedCurrent = doorClosedLastSecondTimes > 200;
  int powerOnCurrent = powerOnLastSecondTimes > 200;
  Serial.println("doorClosedCurrent (>200):");
  Serial.println(doorClosedCurrent);
  Serial.println("powerOnCurrent (>200):");
  Serial.println(powerOnCurrent);



  if (doorClosedPrevious == 2 || powerOnPrevious == 2) {  // initialize state
    Serial.println("Initializing state");
    doorClosedPrevious = doorClosedCurrent;
    powerOnPrevious = powerOnCurrent;
  } else {
    // check if something changed:

    // finished washing
    if (doorClosedPrevious == 1 && doorClosedCurrent == 0) {
      Serial.println("doorClosedPrevious == 1 && doorClosedCurrent == 0");
      // start beeping
      if (washingSeconds > 5) {  // debounced
        Serial.println("washingSeconds > 5. Started beeping");
        currentlyBeeping = 1;
        currentlySecondsAfterFinishedWashing = 0;
      }
    }

    // new washing is started
    // this code assumes "door closed" initial state is not a real scenario.
    if (doorClosedPrevious == 0 && doorClosedCurrent == 1) {
      Serial.println("doorClosedPrevious == 0 && doorClosedCurrent == 1");
      Serial.println("Stopped beeping");
      // stop beeping
      currentlyBeeping = 0;
      washingSeconds = 0;
      stopBeeping();
    }

    // power off
    if (powerOnPrevious == 1 && powerOnCurrent == 0) {
      Serial.println("powerOnPrevious == 1 && powerOnCurrent == 0");
      Serial.println("Stopped beeping");
      // stop beeping
      currentlyBeeping = 0;
      stopBeeping();
    }

    // increase washing counter
    if (doorClosedCurrent) washingSeconds += 1;
    Serial.println("washingSeconds");
    Serial.println(washingSeconds);

    // power on
    // if (powerOnCurrent == 0 && powerOnCurrent == 1) {
    //   do nothing
    // }

    // beeping when finished washing
    if (currentlyBeeping) {
      Serial.println("currentlySecondsAfterFinishedWashing:");
      Serial.println(currentlySecondsAfterFinishedWashing);

      // if out of acceptable fibonacci bounds:
      if (currentlySecondsAfterFinishedWashing > 1200) {
        Serial.println("More than 1200 seconds after finished washing");

        // beep once every 10 minutes:
        if ((currentlySecondsAfterFinishedWashing % 600) == 0) {
          beep();
        }
        // stop the beep on the next second
        if (((currentlySecondsAfterFinishedWashing % 600) - 1) == 0) {
          stopBeeping();
        }
      } else {
        // within fibonacci bounds, try to match fibonacci second
        for (int i = 0; i < 7; i += 1) {
          // if first iteration, beep three times: on 0, 2, 4 second
          // if next iterations, beep once
          int fibonacciMatchedBeep = (currentlySecondsAfterFinishedWashing == (beepIntervalSeconds[i] + 0));
          int firstBeep = currentlySecondsAfterFinishedWashing == 2 || currentlySecondsAfterFinishedWashing == 4;
          if (fibonacciMatchedBeep || firstBeep) {
            Serial.println("modified fibonacci index:");
            Serial.println(i);
            beep();
            break;
          }
          // if first iteration, stop the beep three times: on 1, 3, 5 second
          // if next iterations, stop once (1 second after after beeping)
          int fibonacciMatchedBeepStop = (currentlySecondsAfterFinishedWashing == (beepIntervalSeconds[i] + 1));
          int firstBeepStop = currentlySecondsAfterFinishedWashing == 3 || currentlySecondsAfterFinishedWashing == 5;
          if (fibonacciMatchedBeepStop || firstBeepStop) {
            Serial.println("modified fibonacci index:");
            Serial.println(i);
            stopBeeping();
            break;
          }
        }
      }
      currentlySecondsAfterFinishedWashing += 1;
    }


    Serial.println("Setting doorClosedPrevious and powerOnPrevious");
    doorClosedPrevious = doorClosedCurrent;
    powerOnPrevious = powerOnCurrent;
  }
  Serial.println("Loop end");
}