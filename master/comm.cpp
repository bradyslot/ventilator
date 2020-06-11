// ! Implementation of comm ! ==================================================

#include "comm.h"

bool breathReady() {
    if ((t.current - t.previous) >= 1000) {
        t.previous    = t.current;
        sweepRequired = true;  // resets within updateSweeps() after an update
                               // required for better update handling
        return true;
    } else {
        return false;
    }
}

void openHailingFrequency() {
    commandUpdate();
    if (volumeChanged()) { messenger(send.volume); }
    if (inhaleChanged()) { messenger(send.inhale); }
    if (bpmChanged()) { messenger(send.bpm); }
}

void commandUpdate() {
    if (digitalRead(enc1buttonPin) == LOW) {
        setVolumeCommand();  // sets value for command to send current volume
        setInhaleCommand();  // sets value for command to send current inhale
        setBpmCommand();
    }
}

void messenger(uint8_t command) {
    Wire.beginTransmission(SLAVE_ADDR);
    Wire.write(command);
    Wire.endTransmission();

    uint8_t response;
    Wire.requestFrom(SLAVE_ADDR, ANSWER_SIZE);
    while (Wire.available()) {
        response = Wire.read();
    }

    Serial.println("command: " + String(command));
    Serial.println("response: " + String(response));
}

bool volumeChanged() {
    sendVolumeCurrent = send.volume;
    if (sendVolumeCurrent != sendVolumePrevious) {
        sendVolumePrevious = sendVolumeCurrent;
        return true;
    } else {
        return false;
    }
}

bool inhaleChanged() {
    sendInhaleCurrent = send.inhale;
    if (sendInhaleCurrent != sendInhalePrevious) {
        sendInhalePrevious = sendInhaleCurrent;
        return true;
    } else {
        return false;
    }
}

bool bpmChanged() {
    sendBpmCurrent = send.bpm;
    if (sendBpmCurrent != sendBpmPrevious) {
        sendInhalePrevious = sendInhaleCurrent;
        return true;
    } else {
        return false;
    }
}

void setVolumeCommand() {
    send.volume = (50.0 * volume.target) + 10.0;
}

void setInhaleCommand() {
    send.inhale = (20.0 * inhale.target) + 40.0;
}

void setBpmCommand() {
    send.bpm = 72 + bpm.target;
}