const int pinDoor = 4;
const int pinPower = 5;
const int pinBuzzer = 6;

int doorClosedPrevious = 2;  // 0 = door open, 1 = door closed, 2 = undefined
int doorClosedLastSecondTimes = 0;

int powerOnPrevious = 2;  // 0 = power off, 1 = power on, 2 = undefined
int powerOnLastSecondTimes = 0;

int currentlyBeeping = 0;                      // 0 = silence (currently washing), 1 = performing beeping procedure (washing completed)
int currentlySecondsAfterFinishedWashing = 0;  // counter of seconds after washing completion

int beepIntervalSeconds[] = {
  0,
  21,
  55,
  89,
  144,
  233,
  377,
  610,
};  // modified fibonacci


void setup() {
  pinMode(pinDoor, INPUT);
  pinMode(pinPower, INPUT);
  pinMode(pinBuzzer, OUTPUT);
  Serial.begin(115200);
  Serial.println("Init. Idea: 22.12.2024. Firmware: 22.12.2024.");
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
  Serial.println("Loop start");
  unsigned long previousMillis = millis();
  Serial.println("Millis: ");
  Serial.println(previousMillis);
  while (millis() - previousMillis < 1000) {                                                     // https://arduino.stackexchange.com/a/12591
    int pinDoorStateRead1 = digitalRead(pinDoor);                                                // 10us
    int pinDoorStateRead2 = digitalRead(pinDoor);                                                // 10us
    int pinDoorStateRead3 = digitalRead(pinDoor);                                                // 10us
    if ((pinDoorStateRead1 == pinDoorStateRead2) && (pinDoorStateRead2 == pinDoorStateRead3)) {  // 125Hz - 500Hz, 450us high
      if (pinDoorStateRead1 == 1) doorClosedLastSecondTimes++;
    }

    int pinPowerStateRead1 = digitalRead(pinPower);                                                  // 10us
    int pinPowerStateRead2 = digitalRead(pinPower);                                                  // 10us
    int pinPowerStateRead3 = digitalRead(pinPower);                                                  // 10us
    if ((pinPowerStateRead1 == pinPowerStateRead2) && (pinPowerStateRead2 == pinPowerStateRead3)) {  // 125Hz - 500Hz, 450us low
      if (pinPowerStateRead1 == 0) powerOnLastSecondTimes++;
    }
  }
  Serial.println("powerOnLastSecondTimes:");
  Serial.println(powerOnLastSecondTimes);
  Serial.println("doorClosedLastSecondTimes:");
  Serial.println(doorClosedLastSecondTimes);


  // 125Hz, 450us duty cycle, one Arduino loop check is 60us. It means that within one second, the signal became 125 times high, each time we have checked it 450us/60us=7 times, 7 times * 125 times = 875. 200 is guaranteed to be within acceptable bounds 0 < 200 < 875.
  int doorClosedCurrent = doorClosedLastSecondTimes > 200;
  int powerOnCurrent = powerOnLastSecondTimes > 200;
  Serial.println("doorClosedCurrent:");
  Serial.println(doorClosedCurrent);
  Serial.println("powerOnCurrent:");
  Serial.println(powerOnCurrent);



  if (doorClosedPrevious == 2 || powerOnPrevious == 2) {  // initialize state
    Serial.println("Initializing state");
    doorClosedPrevious = doorClosedCurrent;
    powerOnPrevious = powerOnCurrent;
  } else {
    // check if something changed:
    if (doorClosedPrevious == 1 && doorClosedCurrent == 0) {
      Serial.println("doorClosedPrevious == 1 && doorClosedCurrent == 0");
      Serial.println("Started beeping");
      // start beeping
      currentlyBeeping = 1;
      currentlySecondsAfterFinishedWashing = 0;
    }
    if (doorClosedPrevious == 0 && doorClosedCurrent == 1) {
      Serial.println("doorClosedPrevious == 0 && doorClosedCurrent == 1");
      Serial.println("Stopped beeping");
      // stop beeping (new washing is started)
      currentlyBeeping = 0;
      stopBeeping();
    }
    if (powerOnPrevious == 1 && powerOnCurrent == 0) {
      Serial.println("powerOnPrevious == 1 && powerOnCurrent == 0");
      Serial.println("Stopped beeping");
      // stop beeping (power off)
      currentlyBeeping = 0;
      stopBeeping();
    }
    // if (powerOnCurrent == 0 && powerOnCurrent == 1) {
    //   power on, do nothing
    // }

    if (currentlyBeeping) {
      // if out of acceptable fibonacci bounds:
      Serial.println("currentlySecondsAfterFinishedWashing:");
      Serial.println(currentlySecondsAfterFinishedWashing);

      if (currentlySecondsAfterFinishedWashing > 650) {
        Serial.println("More than 650 seconds after finished washing");

        // beep once every 10 minutes:
        if ((currentlySecondsAfterFinishedWashing % 600) == 0) {
          beep();
        }
        // stop the beep on the next second
        if (((currentlySecondsAfterFinishedWashing % 600) - 1) == 0) {
          stopBeeping();
        }
      } else {
        for (int i = 0; i < 8; i += 1) {
          Serial.println("modified fibonacci index:");
          Serial.println(i);
          // beep three times: on 0, 2, 4 second
          if ((currentlySecondsAfterFinishedWashing == (beepIntervalSeconds[i] + 0)) || (currentlySecondsAfterFinishedWashing == (beepIntervalSeconds[i] + 2)) || (currentlySecondsAfterFinishedWashing == (beepIntervalSeconds[i] + 4))) {
            beep();
            break;
          }
          // stop the beep three times: on 1, 3, 5 second
          if ((currentlySecondsAfterFinishedWashing == (beepIntervalSeconds[i] + 1)) || (currentlySecondsAfterFinishedWashing == (beepIntervalSeconds[i] + 3)) || (currentlySecondsAfterFinishedWashing == (beepIntervalSeconds[i] + 5))) {
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