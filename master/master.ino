// ! The Mechatronic Ventilator - Master ! =====================================

#include <Arduino.h>
#include <UTFTGLUE.h>
#define ENCODER_DO_NOT_USE_INTERUPTS
#include <Encoder.h>
#include <comm.h>
#include <dial.h>
#include <pressure.h>
#include <util.h>

// * PINS ======================================================================

#define DISPLAY_ID 0x9486          // hardware ID for MCUFRIEND ILI9341 3.5"
const uint8_t RS            = 57;  // Register Select
const uint8_t WR            = 56;  // LCD Write
const uint8_t CS            = 55;  // Chip Select
const uint8_t RST           = 54;  // Reset
const uint8_t RD            = 58;  // LCD Read
const uint8_t enc1dtPin     = 22;  // Encoder pins
const uint8_t enc1clkPin    = 23;  // Encoder pins
const uint8_t enc1buttonPin = 28;  // Encoder pins
const uint8_t enc2dtPin     = 24;  // Encoder pins
const uint8_t enc2clkPin    = 25;  // Encoder pins
const uint8_t enc2buttonPin = 29;  // Encoder pins
const uint8_t enc3dtPin     = 26;  // Encoder pins
const uint8_t enc3clkPin    = 27;  // Encoder pins
const uint8_t enc3buttonPin = 30;  // Encoder pins
const uint8_t speaker       = 38;  // Speaker pin
const uint8_t slcPin1       = 34;  // Selector pins
const uint8_t slcPin2       = 35;  // Selector pins
const uint8_t slcPin3       = 36;  // Selector pins
const uint8_t slcPin4       = 37;  // Selector pins

// * SPECIFICATIONS ============================================================

const int     WIDTH     = 480;   // display px
const int     HEIGHT    = 320;   // display px
const float   MIN_TV    = 0.2;   // Min tidal volume (L)
const float   MAX_TV    = 0.8;   // Max tidal volume (L)
const float   INC_TV    = 0.02;  // Increments of volume (L)
const uint8_t MIN_BPM   = 5;     // Min breaths per minute
const uint8_t MAX_BPM   = 40;    // Max breaths per minute
const uint8_t INC_BPM   = 1;     // Increments of breaths per minute
const float   MIN_IT    = 0.5;   // Min inhale time (s)
const float   MAX_IT    = 2.0;   // Max inhale time (s)
const float   INC_IT    = 0.05;  // Increments of inhale time (s)
const float   MIN_MV    = 4.0;   // Min minute volume (L)
const float   MAX_MV    = 10.0;  // Max minute volume (L)
const float   INC_MV    = 0.02;  // Increments of minute volume (L)
const float   MIN_PEEP  = 5.0;   // Min positive-end expiratory pressure (cmH20)
const float   MAX_PEEP  = 20.0;  // Max positive-end expiratory pressure (cmH20)
const float   INC_PEEP  = 0.5;   // Increments of peep (cmH20)
const float   MIN_PEAK  = 10.0;  // Min airway Pressure (cmH20)
const float   MAX_PEAK  = 40.0;  // Max airway pressure (cmH20)
const float   INC_PEAK  = 0.5;   // Increments of pressure (cmH20)
const float   HIGH_PEAK = 35;    // Pressure over this is high
const float   MAX_PLAT  = 30;    // Max plateau pressure

const float   DEFAULT_TV   = 0.5;
const uint8_t DEFAULT_BPM  = 12;
const float   DEFAULT_IT   = 2.0;
const float   DEFAULT_PEAK = 30;
const float   DEFAULT_PEEP = 5.0;

// * OBJECTS ===================================================================

UTFTGLUE display(DISPLAY_ID, RS, WR, CS, RST, RD);
Encoder  enc1obj(enc1clkPin, enc1dtPin);
Encoder  enc2obj(enc2clkPin, enc2dtPin);
Encoder  enc3obj(enc3clkPin, enc3dtPin);

// * GLOBALS ===================================================================

const int
    // dial origin coordinates
    firstX  = 85,
    firstY  = 110,
    secondX = 240, secondY = firstY,
    thirdX = 395, thirdY = firstY,
    fourthX = 85, fourthY = 250,
    fifthX = 240, fifthY = fourthY,
    sixthX = 395, sixthY = fourthY,
    r1 = 70, r2 = 60,
    minDeg = 0, maxDeg = 270, offsetDeg = 45;

bool
    sweepRequired = false,  // true if a sweep is needed this breath cycle
    sweepState    = false;  // true if currently in sweeping mode

// * STRUCTURES ================================================================

struct selector {
    int mode,
        modeCurrent,
        modePrevious;
} select;

struct timer {
    unsigned long int current;
    unsigned long int previous;
} t;

struct encoder {
    int
        buttonCurrent,
        buttonPrevious,
        counter,
        counterCurrent,
        counterPrevious,
        counterDirection,
        position,
        currentCLK,
        previousCLK;
} enc1, enc2, enc3;

struct dial volume;
struct dial bpm;
struct dial inhale;
struct dial peak;
struct dial minute;
struct dial peep;

// * INIT DEFAULTS =============================================================

void initDefaults() {
    // volume dial parameters
    volume.x               = firstX;
    volume.y               = firstY;
    volume.min             = MIN_TV;
    volume.max             = MAX_TV;
    volume.inc             = INC_TV;
    volume.target          = DEFAULT_TV;
    volume.current         = DEFAULT_TV;
    volume.maxPosition     = (volume.max - volume.min) / volume.inc;
    volume.targetPosition  = (volume.target - volume.min) / volume.inc;
    volume.currentPosition = (volume.current - volume.min) / volume.inc;
    volume.label           = "v(L)";
    volume.precision       = 2;
    volume.labelX          = volume.x - 22;
    volume.labelY          = volume.y + 30;
    volume.targetX         = volume.x - 35;
    volume.targetY         = volume.y - 45;
    volume.currentX        = volume.x - 35;
    volume.currentY        = volume.y - 15;
    volume.cutoutUpperY    = volume.y;
    volume.cutoutLowerY    = volume.y + r1;
    volume.cutoutStartX    = volume.x - r1;
    volume.cutoutMidX      = volume.x;
    volume.cutoutEndX      = volume.x + r1;

    // bpm dial parameters
    bpm.x               = secondX;
    bpm.y               = secondY;
    bpm.min             = MIN_BPM;
    bpm.max             = MAX_BPM;
    bpm.inc             = INC_BPM;
    bpm.target          = DEFAULT_BPM;
    bpm.current         = DEFAULT_BPM;
    bpm.maxPosition     = (bpm.max - bpm.min) / bpm.inc;
    bpm.targetPosition  = (bpm.target - bpm.min) / bpm.inc;
    bpm.currentPosition = (bpm.current - bpm.min) / bpm.inc;
    bpm.label           = "bpm";
    bpm.precision       = 0;
    bpm.labelX          = bpm.x - 26;
    bpm.labelY          = bpm.y + 30;
    bpm.targetX         = bpm.x - 20;
    bpm.targetY         = bpm.y - 45;
    bpm.currentX        = bpm.x - 20;
    bpm.currentY        = bpm.y - 15;
    bpm.cutoutUpperY    = bpm.y;
    bpm.cutoutLowerY    = bpm.y + r1;
    bpm.cutoutStartX    = bpm.x - r1;
    bpm.cutoutMidX      = bpm.x;
    bpm.cutoutEndX      = bpm.x + r1;

    // inhale time dial perameters
    inhale.x               = thirdX;
    inhale.y               = thirdY;
    inhale.min             = MIN_IT;
    inhale.max             = MAX_IT;
    inhale.inc             = INC_IT;
    inhale.target          = DEFAULT_IT;
    inhale.current         = DEFAULT_IT;
    inhale.maxPosition     = (inhale.max - inhale.min) / inhale.inc;
    inhale.targetPosition  = (inhale.target - inhale.min) / inhale.inc;
    inhale.currentPosition = (inhale.current - inhale.min) / inhale.inc;
    inhale.label           = "in(s)";
    inhale.precision       = 2;
    inhale.labelX          = inhale.x - 25;
    inhale.labelY          = inhale.y + 30;
    inhale.targetX         = inhale.x - 35;
    inhale.targetY         = inhale.y - 45;
    inhale.currentX        = inhale.x - 35;
    inhale.currentY        = inhale.y - 15;
    inhale.cutoutUpperY    = inhale.y;
    inhale.cutoutLowerY    = inhale.y + r1;
    inhale.cutoutStartX    = inhale.x - r1;
    inhale.cutoutMidX      = inhale.x;
    inhale.cutoutEndX      = inhale.x + r1;

    // peak pressure dial perameters
    peak.x               = fourthX;
    peak.y               = fourthY;
    peak.min             = MIN_PEAK;
    peak.max             = MAX_PEAK;
    peak.inc             = INC_PEAK;
    peak.target          = DEFAULT_PEAK;
    peak.current         = DEFAULT_PEAK;
    peak.maxPosition     = (peak.max - peak.min) / peak.inc;
    peak.targetPosition  = (peak.target - peak.min) / peak.inc;
    peak.currentPosition = (peak.current - peak.min) / peak.inc;
    peak.label           = "peak";
    peak.precision       = 1;
    peak.labelX          = peak.x - 30;
    peak.labelY          = peak.y + 30;
    peak.targetX         = peak.x - 35;
    peak.targetY         = peak.y - 45;
    peak.currentX        = peak.x - 35;
    peak.currentY        = peak.y - 15;
    peak.cutoutUpperY    = peak.y;
    peak.cutoutLowerY    = peak.y + r1;
    peak.cutoutStartX    = peak.x - r1;
    peak.cutoutMidX      = peak.x;
    peak.cutoutEndX      = peak.x + r1;

    // minute volume dial perameters
    minute.x               = fifthX;
    minute.y               = fifthY;
    minute.min             = MIN_MV;
    minute.max             = MAX_MV;
    minute.target          = volume.target * bpm.target;
    minute.current         = volume.current * bpm.current;
    minute.maxPosition     = volume.maxPosition * bpm.maxPosition;
    minute.inc             = (minute.max - minute.min) / minute.maxPosition;
    minute.targetPosition  = (minute.target - minute.min) / minute.inc;
    minute.currentPosition = (minute.current - minute.min) / minute.inc;
    minute.label           = "mv(L)";
    minute.errorUpper      = String(">" + String(minute.max, 0));
    minute.errorLower      = String("<" + String(minute.min, 0));
    minute.precision       = 2;
    minute.labelX          = minute.x - 30;
    minute.labelY          = minute.y + 30;
    minute.targetX         = minute.x - 35;
    minute.targetY         = minute.y - 45;
    minute.currentX        = minute.x - 35;
    minute.currentY        = minute.y - 15;
    minute.errorX          = minute.x - 35;
    minute.errorY          = minute.y - 30;
    minute.cutoutUpperY    = minute.y;
    minute.cutoutLowerY    = minute.y + r1;
    minute.cutoutStartX    = minute.x - r1;
    minute.cutoutMidX      = minute.x;
    minute.cutoutEndX      = minute.x + r1;

    // peep pressure dial perameters
    peep.x               = sixthX;
    peep.y               = sixthY;
    peep.min             = MIN_PEEP;
    peep.max             = MAX_PEEP;
    peep.inc             = INC_PEEP;
    peep.target          = DEFAULT_PEEP;
    peep.current         = DEFAULT_PEEP;
    peep.maxPosition     = (peep.max - peep.min) / peep.inc;
    peep.targetPosition  = (peep.target - peep.min) / peep.inc;
    peep.currentPosition = (peep.current - peep.min) / peep.inc;
    peep.label           = "peep";
    peep.precision       = 1;
    peep.labelX          = peep.x - 30;
    peep.labelY          = peep.y + 30;
    peep.targetX         = peep.x - 35;
    peep.targetY         = peep.y - 45;
    peep.currentX        = peep.x - 35;
    peep.currentY        = peep.y - 15;
    peep.cutoutUpperY    = peep.y;
    peep.cutoutLowerY    = peep.y + r1;
    peep.cutoutStartX    = peep.x - r1;
    peep.cutoutMidX      = peep.x;
    peep.cutoutEndX      = peep.x + r1;
}

// * DRAWING ===================================================================

void drawDialGUI() {
    display.clrScr();
    display.fillScr(0, 43, 54);  // c.back

    drawDialBase(volume);
    drawDialBase(bpm);
    drawDialBase(inhale);
    drawDialBase(peak);
    drawDialBase(minute);
    drawDialBase(peep);

    updateTargetByPosition(&volume);
    updateCurrentByPosition(&volume);
    drawTargetElements(&volume);
    drawCurrentElements(&volume);

    updateTargetByPosition(&bpm);
    updateCurrentByPosition(&bpm);
    drawTargetElements(&bpm);
    drawCurrentElements(&bpm);

    updateTargetByPosition(&inhale);
    updateCurrentByPosition(&inhale);
    drawTargetElements(&inhale);
    drawCurrentElements(&inhale);

    updateTargetByPosition(&peak);
    updateCurrentByPosition(&peak);
    drawTargetElements(&peak);
    drawCurrentElements(&peak);

    updateTargetByValue(&minute);
    updateCurrentByValue(&minute);
    drawTargetElements(&minute);
    drawCurrentElements(&minute);

    updateTargetByPosition(&peep);
    updateCurrentByPosition(&peep);
    drawTargetElements(&peep);
    drawCurrentElements(&peep);

    drawCenterLines();
}

// draw lines though dial origins for alignment purposes
void drawCenterLines() {
    display.setColor(c.target);
    display.drawLine(firstX, 0, firstX, HEIGHT);
    display.drawLine(secondX, 0, secondX, HEIGHT);
    display.drawLine(thirdX, 0, thirdX, HEIGHT);
    display.drawLine(0, firstY, WIDTH, firstY);
    display.drawLine(0, fourthY, WIDTH, fourthY);
}

// build the coordinates from points on the circumference of circles r1 and r2
// calculates 6 coordinates in order to draw a polygon of 4 triangles
// needle shape is that of a segment of the dial face an angle from its origin
// the needles width is 6, as that draws most clearly for the size of dial used
// buildNeedle returns void, as the destination array is passed in along with
// the dial struct to avoid unnecessary globals

// * MODE HEADINGS =============================================================

const int
    // Heading Center
    mode1X = 100,
    mode2X = 70,
    mode3X = 50,
    mode4X = 35,
    modeY  = -15;

void selectModeHeading() {
    display.setFont(largeFontBold);
    if (select.mode == 1)
        drawMode1Heading();
    if (select.mode == 2)
        drawMode2Heading();
    if (select.mode == 3)
        drawMode3Heading();
    if (select.mode == 4)
        drawMode4Heading();
}
void drawModeHeading() {
    display.setColor(c.mode);
    selectModeHeading();
}

void clearModeHeading() {
    display.setColor(c.back);
    selectModeHeading();
}

void drawMode1Heading() {
    display.print(String("Disable"), mode1X, modeY);
}

void drawMode2Heading() {
    display.print(String("Mandatory Mode"), mode2X, modeY);
}

void drawMode3Heading() {
    display.print(String("Assist Control"), mode3X, modeY);
}

void drawMode4Heading() {
    display.print(String("Spontaneous Only"), mode4X, modeY);
}

// * INPUT =====================================================================

// Mode selection
void checkSelector() {
    if (digitalRead(slcPin1) == LOW)  // Mandatory Mode
        select.modeCurrent = 1;
    if (digitalRead(slcPin2) == LOW)  // Assist/Control Mode
        select.modeCurrent = 2;
    if (digitalRead(slcPin3) == LOW)  // Pressure/Control Mode
        select.modeCurrent = 3;
    if (digitalRead(slcPin4) == LOW)  // Spontaneous Only Mode
        select.modeCurrent = 4;

    if (select.modeCurrent != select.modePrevious) {
        clearModeHeading();
        select.mode = select.modeCurrent;
        drawModeHeading();
    }

    select.modePrevious = select.modeCurrent;
}

// Count encoder movement
int countEncoders() {
    enc1.counterCurrent = enc1obj.read() / 2;
    enc2.counterCurrent = enc2obj.read() / 2;
    enc3.counterCurrent = enc3obj.read() / 2;

    if (enc1.counterCurrent != enc1.counter) {
        enc1.counter = enc1.counterCurrent;
        Serial.println("encoder 1 counter " + String(enc1.counter));
    }
    if (enc2.counterCurrent != enc2.counter) {
        enc2.counter = enc2.counterCurrent;
        Serial.println("encoder 2 counter " + String(enc2.counter));
    }
    if (enc3.counterCurrent != enc3.counter) {
        enc3.counter = enc3.counterCurrent;
        Serial.println("encoder 3 counter " + String(enc3.counter));
    }
}

// Check for button presses
void checkButtons() {
    enc1.buttonCurrent = digitalRead(enc1buttonPin);
    if (enc1.buttonCurrent != enc1.buttonPrevious && enc1.buttonCurrent == HIGH) {
        sweepSetup();
        Serial.println("encoder 1 pressed");
    }
    enc1.buttonPrevious = enc1.buttonCurrent;

    enc2.buttonCurrent = digitalRead(enc2buttonPin);
    if (enc2.buttonCurrent != enc2.buttonPrevious && enc2.buttonCurrent == HIGH) {
        //sweepSetup();
        Serial.println("encoder 2 pressed");
    }
    enc2.buttonPrevious = enc2.buttonCurrent;

    enc3.buttonCurrent = digitalRead(enc3buttonPin);
    if (enc3.buttonCurrent != enc3.buttonPrevious && enc3.buttonCurrent == HIGH) {
        //sweepSetup();
        Serial.println("encoder 3 pressed");
    }
    enc3.buttonPrevious = enc3.buttonCurrent;
}

// * UPDATES ===================================================================

// Constrain change in encoder position to within operating range
// Runs drawing logic when position updates
void updateTargets() {
    float volumeRange = volume.max - volume.min;
    float bpmRange    = bpm.max - bpm.min;
    float inhaleRange = inhale.max - inhale.min;

    // 1 or -1 for pos and neg direction
    enc1.counterDirection = enc1.counter - enc1.counterPrevious;
    if (enc1.counterDirection > 0 && volume.targetPosition < volume.maxPosition) {
        volume.targetPosition++;
        Serial.println("Volume Position: " + String(volume.targetPosition));
    }
    if (enc1.counterDirection < 0 && volume.targetPosition > 0) {
        volume.targetPosition--;
        Serial.println("Volume Position: " + String(volume.targetPosition));
    }
    if (enc1.counterDirection) {
        volume.direction = enc1.counterDirection;
        updateTargetByPosition(&volume);
        drawTargetElements(&volume);
    }

    enc2.counterDirection = enc2.counter - enc2.counterPrevious;
    if (enc2.counterDirection > 0 && bpm.targetPosition < bpm.maxPosition) {
        bpm.targetPosition++;
        Serial.println("bpm Position: " + String(bpm.targetPosition));
    }
    if (enc2.counterDirection < 0 && bpm.targetPosition > 0) {
        bpm.targetPosition--;
        Serial.println("bpm Position: " + String(bpm.targetPosition));
    }
    if (enc2.counterDirection) {
        bpm.direction = enc2.counterDirection;
        updateTargetByPosition(&bpm);
        drawTargetElements(&bpm);
    }

    enc3.counterDirection = enc3.counter - enc3.counterPrevious;
    if (enc3.counterDirection > 0 && inhale.targetPosition < inhale.maxPosition) {
        inhale.targetPosition++;
        Serial.println("inhale Position: " + String(inhale.targetPosition));
    }
    if (enc3.counterDirection < 0 && inhale.targetPosition > 0) {
        inhale.targetPosition--;
        Serial.println("inhale Position: " + String(inhale.targetPosition));
    }
    if (enc3.counterDirection) {
        inhale.direction = enc3.counterDirection;
        updateTargetByPosition(&inhale);
        drawTargetElements(&inhale);
    }

    if (enc1.counterDirection || enc2.counterDirection) {
        minute.target = volume.target * bpm.target;
        updateTargetByValue(&minute);
        drawTargetElements(&minute);
    }

    enc1.counterDirection = 0;
    volume.direction      = 0;
    enc1.counterPrevious  = enc1.counter;
    enc2.counterDirection = 0;
    bpm.direction         = 0;
    enc2.counterPrevious  = enc2.counter;
    enc3.counterDirection = 0;
    inhale.direction      = 0;
    enc3.counterPrevious  = enc3.counter;
}

// Update the sweep values for display when breath cycle complete is returned
// from the slave
void updateSweeps() {
    if (sweepRequired && sweepState) {
        if (volume.currentPosition < volume.targetPosition) {
            volume.currentPosition++;
            updateCurrentByPosition(&volume);
            drawCurrentElements(&volume);
        }

        if (volume.targetPosition < volume.currentPosition) {
            volume.currentPosition--;
            updateCurrentByPosition(&volume);
            drawCurrentElements(&volume);
        }

        if (bpm.currentPosition < bpm.targetPosition) {
            bpm.currentPosition++;
            updateCurrentByPosition(&bpm);
            drawCurrentElements(&bpm);
        }

        if (bpm.targetPosition < bpm.currentPosition) {
            bpm.currentPosition--;
            updateCurrentByPosition(&bpm);
            drawCurrentElements(&bpm);
        }

        if (inhale.currentPosition < inhale.targetPosition) {
            inhale.currentPosition++;
            updateCurrentByPosition(&inhale);
            drawCurrentElements(&inhale);
        }

        if (inhale.targetPosition < inhale.currentPosition) {
            inhale.currentPosition--;
            updateCurrentByPosition(&inhale);
            drawCurrentElements(&inhale);
        }

        minute.current = volume.current * bpm.current;
        updateCurrentByValue(&minute);
        drawCurrentElements(&minute);

        if (volume.targetPosition == volume.currentPosition)
            if (inhale.targetPosition == inhale.currentPosition)
                if (bpm.targetPosition == bpm.currentPosition)
                    sweepState = false;

        sweepRequired = false;
    }
}

void sweepSetup() {
    if (minute.error == 0) {
        sweepState = true;
    } else {
        sweepState = false;
    }
}

// * MAIN START ================================================================

void setup() {
    Wire.begin();

    Serial.begin(115200);
    while (!Serial) {
        ;  // wait till connected
    }
    Serial.println("Begin");

    if (!sensor.begin(PRESSURE_ID)) {
        Serial.println("Can't connect to pressure sensor");
        while (1) {
            delay(10);
        }
    }
    Serial.println("Pressure sensor found");

    pressureSensorCheck();

    // calibrate baseline pressure from average over 1 second
    float sum = 0.0;
    float num = 0;
    Serial.println("Calibrating baseline pressure...");
    while (num < 100) {
        sum = getPressure() + sum;
        delay(10);
        num++;
    }
    cmH20.atmosphere = sum / num;
    Serial.println("Calibration complete.");
    Serial.println("Baseline Pressure: " + String(cmH20.atmosphere, 2));

    pinMode(RD, OUTPUT);
    digitalWrite(RD, HIGH);

    display.InitLCD();
    initDefaults();
    drawDialGUI();

    pinMode(enc1buttonPin, INPUT_PULLUP);
    pinMode(enc1dtPin, INPUT_PULLUP);
    pinMode(enc1clkPin, INPUT_PULLUP);
    enc1.buttonPrevious = digitalRead(enc1buttonPin);

    pinMode(enc2buttonPin, INPUT_PULLUP);
    pinMode(enc2dtPin, INPUT_PULLUP);
    pinMode(enc2clkPin, INPUT_PULLUP);
    enc2.buttonPrevious = digitalRead(enc2buttonPin);

    pinMode(enc3buttonPin, INPUT_PULLUP);
    pinMode(enc3dtPin, INPUT_PULLUP);
    pinMode(enc3clkPin, INPUT_PULLUP);
    enc3.buttonPrevious = digitalRead(enc3buttonPin);

    pinMode(slcPin1, INPUT_PULLUP);
    pinMode(slcPin2, INPUT_PULLUP);
    pinMode(slcPin3, INPUT_PULLUP);
    pinMode(slcPin4, INPUT_PULLUP);

    t.previous = millis();
    Serial.println("End");
}

void loop() {
    t.current = millis();
    checkSelector();
    countEncoders();
    checkButtons();
    updateTargets();
    updateSweeps();
    openHailingFrequency();

    if (pressureSensorReady()) {
        cmH20.airway = getAirway();
        //Serial.println("Airway Pressure: " + String(cmH20.airway, 1));
    }
}

// * MAIN END ==================================================================