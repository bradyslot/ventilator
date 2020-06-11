// ! The Mechatronic Ventilator - Slave ! ======================================

#include <Arduino.h>
#include <SpeedyStepper.h>
#include <Wire.h>

// * SPECS =====================================================================

const float MAX_TV  = 0.8;
const float MIN_TV  = 0.2;
const float INC_TV  = 0.02;
const int   MIN_IT  = 500;
const int   MAX_IT  = 2000;
const int   INC_IT  = 50;
const int   MAX_BPM = 40;
const int   MIN_BPM = 5;
const int   INC_BPM = 1;

// * MOTOR PARAMETERS ==========================================================

const float MICRO_STEP       = 1.0;    // Microstepping (0.5 for 1/2)
const float REDUCTION        = 80.0;   // Reduction Ratio
const float STEP_ANGLE       = 0.9;    // Angle per step for your motor
const float STEPS            = 400.0;  // Steps per rotation for your motor
const float MIN_DEG          = 7.5;    // Angle for min volume
const float MAX_DEG          = 25.0;   // Angle for max volume
const float MAX_SPEED        = 16.0;   // Max speed in RPS
const float MAX_ACCELERATION = 24000;  // Accel in SPS
const float ACCEL            = 8500;   // A "big enough" value to not matter
const float DECEL            = 8500;   // A "big enough" value to not matter
const int   INHALE_DIR       = 1;      // Inhale direction
const int   EXHALE_DIR       = -1;     // Exhale direction

// * PIN DEFINITIONS ===========================================================

const uint8_t DIR    = 2;  // direction pin
const uint8_t STEP   = 3;  // step pin
const uint8_t LIMIT  = 4;  // limit switch
const uint8_t ENABLE = 5;  // enable/disable motor

// * OBJECTS ===================================================================

SpeedyStepper stepper;

// * GLOBALS ===================================================================

bool    once = false;
uint8_t command;
uint8_t response;
float   volumeTarget;
int     inhaleTarget;
int     bpmTarget;
bool    atVolumeTarget = true;
bool    atInhaleTarget = true;
bool    atBpmTarget    = true;

// * STRUCTURES ================================================================

struct timers {
    unsigned long
        start,
        end,
        current,
        entered,
        elapsed,
        exited;
} t;

struct cycle {
    int
        state,  // 0 = resting, 1 = inhaling, 3 = exhaling
        inhalePeriod,
        exhalePeriod,
        cyclePeriod,
        restPeriod,
        inhaleDiff,
        steps,
        speed;
    float
        volume,
        angle,
        speedAdjustment;
    bool
        ready,
        inhaleComplete,
        exhaleComplete;
} breath;

// * INIT DEFAULTS =============================================================

void initDefaults() {
    breath.cyclePeriod  = 5000;
    breath.inhalePeriod = 2000;
    breath.exhalePeriod = 500;  // equal to min inhale time
    breath.restPeriod   = breath.cyclePeriod - (breath.inhalePeriod + breath.exhalePeriod);
    breath.volume       = 0.5;
}

// * HOMING ====================================================================

void motorEnable() { digitalWrite(ENABLE, LOW); }
void motorDisable() { digitalWrite(ENABLE, HIGH); }

// homing likes to stay on the limit switch, this moves 0.5 deg away from it
void moveAwayFromHome() {
    int travel = degreeToSteps(0.5);
    stepper.setSpeedInStepsPerSecond(1000);
    stepper.setAccelerationInStepsPerSecondPerSecond(MAX_ACCELERATION);
    stepper.setCurrentPositionInSteps(0);
    stepper.moveToPositionInSteps(INHALE_DIR * travel);
}

// homing moves at a medium speed for a distance guaranteed to intercept
void moveToHome() {
    stepper.setAccelerationInStepsPerSecondPerSecond(MAX_ACCELERATION);
    stepper.moveToHomeInSteps(EXHALE_DIR, STEPS * 5, STEPS * 6, LIMIT);
    moveAwayFromHome();
}

// * BREATHING LOGIC ===========================================================

// perform the inhale with corresponding speed for the inhale time
void inhale() {
    breath.angle = volumeToDegree(breath.volume);
    breath.steps = degreeToSteps(breath.angle);
    breath.speed = float(breath.steps) / (float(breath.inhalePeriod) / 1000.0);
    //Serial.print("breath.speed: ");
    //Serial.println(String(breath.speed));
    // TODO: Implement some kind of speed adjust to meet inhale time target
    //Serial.print("breath.speed(adjusted): ");
    //Serial.println(String(breath.speed));

    //Serial.println("breath.angle: " + String(breath.angle, 2));
    //Serial.println("breath.steps: " + String(breath.steps));
    //Serial.println("ACCEL: " + String(MAX_ACCELERATION));
    stepper.setSpeedInStepsPerSecond(breath.speed);
    stepper.setAccelerationInStepsPerSecondPerSecond(MAX_ACCELERATION);
    stepper.setCurrentPositionInSteps(0);
    stepper.setupMoveInSteps(INHALE_DIR * breath.steps);
}

// exhale is meant to open the mechanism as fast as possible to let the BVM bag
// re-fill naturally
void exhale() {
    stepper.setAccelerationInStepsPerSecondPerSecond(MAX_ACCELERATION);
    stepper.moveToHomeInSteps(EXHALE_DIR, STEPS * 14, STEPS * 6, LIMIT);
    moveAwayFromHome();
}

void manager() {
    updateHandler();

    if (digitalRead(LIMIT) == 0) {
        moveAwayFromHome();
    }

    if (breath.state == 0) {
        if (!once) {
            motorDisable();
            t.entered = millis();
            once      = true;
            if (!atVolumeTarget) { volumeUpdate(); }
            if (!atInhaleTarget) { inhaleUpdate(); }
            //Serial.print("entered rest: ");
            //Serial.println(String(float(t.entered) / 1000.0, 2));
        }

        t.elapsed = millis() - t.entered;

        if (t.elapsed >= breath.restPeriod || breath.ready) {
            t.exited = t.elapsed;
            //Serial.print("exited rest: +");
            //Serial.println(String(float(t.exited) / 1000.0, 2));
            //Serial.print("breath.restPeriod: ");
            //Serial.println(String(float(breath.restPeriod) / 1000.0, 2));
            //Serial.print("difference: ");
            //Serial.println(String(float(t.exited - breath.restPeriod) / 1000.0, 2));
            //Serial.println("--------------------");
            breath.state = 1;
            once         = false;
            t.elapsed    = 0;
            t.entered    = 0;
            t.exited     = 0;
        }
    }

    if (breath.state == 1) {
        if (!once) {
            t.entered = millis();
            motorEnable();
            inhale();  // gets called once, processMovement() does the rest
            once         = true;
            breath.ready = false;  // reset
            //Serial.print("entered inhale: ");
            //Serial.println(String(float(t.entered) / 1000.0, 2));
        }

        t.elapsed = millis() - t.entered;

        if (t.elapsed >= breath.inhalePeriod && stepper.motionComplete()) {
            t.exited = t.elapsed;
            //breath.inhaleDiff = t.exited - breath.inhalePeriod;
            //Serial.print("exited inhale: +");
            //Serial.println(String(float(t.exited) / 1000.0, 2));
            //Serial.print("breath.inhalePeriod: ");
            //Serial.println(String(float(breath.inhalePeriod) / 1000.0, 2));
            //Serial.print("difference: ");
            //Serial.println(String(float(t.exited - breath.inhalePeriod) / 1000.0, 2));
            //Serial.println("--------------------");
            breath.state = 2;
            once         = false;
            t.elapsed    = 0;
            t.entered    = 0;
            t.exited     = 0;
        }
    }

    if (breath.state == 2) {
        if (!once) {
            t.entered = millis();
            once      = true;
            exhale();  // just homes as quickly as possible
            //Serial.print("entered exhale: ");
            //Serial.println(String(float(t.entered) / 1000.0, 2));
        }
        t.elapsed = millis() - t.entered;
        if (t.elapsed >= breath.exhalePeriod && stepper.motionComplete()) {
            t.exited = t.elapsed;
            //Serial.print("exited exhale: +");
            //Serial.println(String(float(t.exited) / 1000.0, 2));
            //Serial.print("breath.exhalePeriod: ");
            //Serial.println(String(float(breath.exhalePeriod) / 1000.0, 2));
            //Serial.print("difference: ");
            //Serial.println(String(float(t.exited - breath.exhalePeriod) / 1000.0, 2));
            //Serial.println("--------------------");
            breath.state = 0;
            once         = false;
            t.elapsed    = 0;
            t.entered    = 0;
            t.exited     = 0;
        }
    }
}

// * MISC ======================================================================

// floating point re-implementation of map()
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
    float val = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    return val;
}

// floating point comparison can fail by rounding tolerances
bool roughlyEqual(float a, float b) {
    if (abs(a - b) < 0.0001)
        return true;
    else
        return false;
}

// get the amount of steps to travel a given angle
int degreeToSteps(float deg) {
    return deg * ((STEPS * REDUCTION * MICRO_STEP) / 360);
}

// TODO: Re-implement more volume delivery functions
// get the travel angle for a given volume
float volumeToDegree(float volume) {
    volume = mapf(volume, MIN_TV, MAX_TV, MIN_DEG, MAX_DEG);
    return volume;
}

// * I2C =======================================================================

const uint8_t SLAVE_ADDR  = 9;                // This slaves address
const uint8_t ANSWER_SIZE = sizeof(uint8_t);  // Bytes of transmission

uint8_t commandList[] = {
    3,  // ready for breath
    4,  // cancel breath part way
    5,  // enable motor
    6,  // disable motor
    7,  // volume control mode
    8,  // pressure control mode
    9   // sigh
};

struct responses {
    uint8_t
        valid,
        invalid,
        updating,
        finished;
    responses()
        : valid{1}
        , invalid{2}
        , updating{18}
        , finished{19} {}
} send;

// populate commandList with valid command ranges for volume, bpm and inhale
void fillCommandList() {
    for (i = 20; i <= 115; i++) {
        commandList[i] = i;
    }
}

void receive() {
    while (0 < Wire.available()) {
        command = Wire.read();
    }
    Serial.println("receive: " + String(command));
    checkValidity(command);
}

void respond() {
    Wire.write(response);
    Serial.println("response: " + String(response));
}

// forms the response
void checkValidity(uint8_t cmd) {
    response    = send.invalid;
    size_t size = sizeof(commandList) / sizeof(uint8_t);
    for (int i = 0; i < size; i++) {
        if (commandList[i] == cmd) {
            response = send.valid;
            break;
        }
    }
}

// runs within manager() always, only updates when necessary
void updateHandler() {
    if (command == 3) {
        breath.ready = true;
        command      = 0;
    }
    if (command >= 20 && command <= 50) {
        volumeTarget   = (float(command) - 10.0) / 50.0;
        atVolumeTarget = false;
        command        = 0;
        Serial.println("volume target: " + String(volumeTarget));
    }
    if (command >= 50 && command <= 80) {
        inhaleTarget   = ((float(command) / 20.0) - 2.0) * 1000.0;
        atInhaleTarget = false;
        command        = 0;
        Serial.println("inhale target: " + String(inhaleTarget));
    }
    if (command >= 80 && command <= 115) {
        bpmTarget   = command - 72;
        atBpmTarget = false;
        command     = 0;
        Serial.println("bpm target: " + String(bpmTarget));
    }
}

// updates volume once per breath cycle 1 increment at a time until reached
void volumeUpdate() {
    // This insures that the current values are only incremented as they're
    // supposed too. Occasionally a command doesn't get sent.
    if (roughlyEqual(volumeTarget, breath.volume)) {
        Serial.println("reached target volume");
        breath.volume  = volumeTarget;
        atVolumeTarget = true;
    } else if (volumeTarget > breath.volume) {
        breath.volume += INC_TV;
        Serial.println("current volume: " + String(breath.volume, 2));
    } else {
        breath.volume -= INC_TV;
        Serial.println("current volume: " + String(breath.volume, 2));
    }
}

// updates inhale time once per breath cycle 1 increment at a time until reached
void inhaleUpdate() {
    if (inhaleTarget == breath.inhalePeriod) {
        Serial.println("reached target inhale");
        atInhaleTarget = true;
    } else if (inhaleTarget > breath.inhalePeriod) {
        breath.inhalePeriod += INC_IT;
        Serial.print("current inhale: ");
        Serial.println(String(float(breath.inhalePeriod / 1000.0), 2));
    } else {
        breath.inhalePeriod -= INC_IT;
        Serial.print("current inhale: ");
        Serial.println(String(float(breath.inhalePeriod / 1000.0), 2));
    }
}

// updates bpm once per breath cycle 1 increment at a time until reached
void bpmUpdate() {
    int tempCyclePeriod = (60.0 / float(bpmTarget)) * 1000.0;
    if (tempCyclePeriod == breath.cyclePeriod) {
        Serial.println("reached bpm target");
    } else if (tempCyclePeriod > breath.cyclePeriod) {
        breath.cyclePeriod += INC_BPM;
        Serial.print("current bpm: " + String(breath.cyclePeriod));
    } else {
        breath.cyclePeriod -= INC_BPM;
        Serial.println("current bpm: " + String(breath.cyclePeriod));
    }
}

// * MAIN START ================================================================

void setup() {
    Wire.begin(SLAVE_ADDR);
    Wire.onRequest(respond);
    Wire.onReceive(receive);

    Serial.begin(115200);
    while (!Serial) {
        ;  // wait till connected
    }
    Serial.println("SETUP START");

    initDefaults();
    fillCommandList();

    // Modify Stepper motor mode pins for different microstepping
    //pinMode(MODE0, LOW);
    //pinMode(MODE1, LOW);
    //pinMode(MODE2, LOW);

    // stepper motor enable/disable
    pinMode(ENABLE, OUTPUT);
    motorDisable();

    // Stepper motor controller
    stepper.connectToPins(STEP, DIR);
    pinMode(LIMIT, INPUT_PULLUP);

    motorEnable();
    moveToHome();
    t.current = millis();
    Serial.println("SETUP END");
}

void loop() {
    t.current = millis();
    manager();

    while (!stepper.motionComplete()) {
        // Code ran in here must be super fast!
        manager();
        stepper.processMovement();
    }
}

// * MAIN END ==================================================================