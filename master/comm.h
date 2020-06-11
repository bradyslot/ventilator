// ! Master <--> Slave communicaiton ! =========================================

#pragma once
#include <Wire.h>

const uint8_t SLAVE_ADDR  = 9;                // Address of slave motor
const uint8_t ANSWER_SIZE = sizeof(uint8_t);  // Bytes of transmission

uint8_t
    sendVolumePrevious,
    sendVolumeCurrent,
    sendInhalePrevious,
    sendInhaleCurrent,
    sendBpmPrevious,
    sendBpmCurrent;

uint8_t responseList[] = {
    1,   // valid
    2,   // invalid
    18,  // updating
    19   // finished
};

typedef struct commands {
    uint8_t
        ready,
        cancel,
        enable,
        disable,
        volumeMode,
        pressureMode,
        sigh,
        volume,
        inhale,
        bpm;
    commands()
        : ready{3}         // ready for breath
        , cancel{4}        // cancel breath part way
        , enable{5}        // enable motor
        , disable{6}       // disbale motor
        , volumeMode{7}    // volume controlled mode
        , pressureMode{8}  // pressure controlled mode
        , sigh{9}          // sigh performed every hour
        , volume{35}       // range of 20 - 50, +1 = 1 INC (default 0.5 L)
        , inhale{80}       // range of 50 - 80, +1 = 1 INC (default 2.0 s)
        , bpm{84}          // range of 80 - 115, +1 equals 1 bpm (default 12)
    {}
};

struct commands send;

// placeholder until i2c communication with slave is implimented
bool breathReady();

void openHailingFrequency();

// prepare corrent command to be sent
void commandUpdate();

void messenger(uint8_t command);

bool volumeChanged();

bool inhaleChanged();

bool bpmChanged();

void setVolumeCommand();

void setInhaleCommand();

void setBpmCommand();
