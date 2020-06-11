// ! Dial Interface Drawing ! ==================================================

#pragma once
#include <Adafruit_GFX.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold18pt7b.h>
#include <UTFTGLUE.h>

// 16bit unsigned int version of the famous Solarized Color Scheme:
// https://ethanschoonover.com/solarized/
// Adafruit GFX library supports RGB565 format colors
//                                           RGB24  =    HEX24  =  COLOR
const uint16_t Base03  = 326;    //    0,  43,  54  =  #002b36  =  brblack
const uint16_t Base02  = 424;    //    7,  54,  66  =  #073642  =  black
const uint16_t Base00  = 25552;  //  101, 123, 131  =  #657b83  =  bryellow
const uint16_t Base0   = 33970;  //  131, 148, 150  =  #839496  =  brBase02
const uint16_t Base1   = 38164;  //  147, 161, 161  =  #93a1a1  =  brcyan
const uint16_t Base2   = 61274;  //  238, 232, 213  =  #eee8d5  =  Base2
const uint16_t Base3   = 65468;  //  253, 246, 227  =  #fdf6e3  =  brBase2
const uint16_t Yellow  = 46144;  //  181, 137,   0  =  #b58900  =  yellow
const uint16_t Orange  = 51778;  //  203,  75,  22  =  #cb4b16  =  brred
const uint16_t Red     = 55685;  //  220,  50,  47  =  #dc322f  =  red
const uint16_t Magenta = 53680;  //  211,  54, 130  =  #d33682  =  magenta
const uint16_t Violet  = 27544;  //  108, 113, 196  =  #6c71c4  =  brmagenta
const uint16_t Blue    = 9306;   //   38, 139, 210  =  #268bd2  =  Blue
const uint16_t Cyan    = 11539;  //   42, 161, 152  =  #2aa198  =  cyan
const uint16_t Green   = 33984;  //  133, 153,   0  =  #859900  =  green

#define smallFont &FreeSans12pt7b
#define smallFontBold &FreeSansBold12pt7b
#define largeFont &FreeSans18pt7b
#define largeFontBold &FreeSansBold18pt7b

const GFXfont
    *m_smallFont     = smallFont,
    *m_smallFontBold = smallFontBold,
    *m_largeFont     = largeFont,
    *m_largeFontBold = largeFontBold;

typedef struct dial {
    int
        targetNeedle[12],
        targetNeedleCurrent[12],
        targetNeedlePrevious[12],
        currentNeedle[12],
        currentNeedleCurrent[12],
        currentNeedlePrevious[12];
    float
        min,
        max,
        inc,
        target,
        targetPrevious,
        targetCurrent,
        current,
        currentPrevious,
        currentCurrent;
    int
        x,
        y,
        position,
        targetAngle,
        currentAngle,
        targetPosition,
        currentPosition,
        error,
        errorCurrent,
        errorPrevious,
        precision,
        maxPosition,
        labelX,
        labelY,
        targetX,
        targetY,
        currentX,
        currentY,
        errorX,
        errorY,
        cutoutMidX,
        cutoutStartX,
        cutoutEndX,
        cutoutUpperY,
        cutoutLowerY,
        direction,
        targetDirection,
        currentDirection;
    String
        label,
        errorUpper,
        errorLower;
};

typedef struct color {
    uint16_t
        back    = Base03,
        label   = Blue,
        face    = Base3,
        target  = Magenta,
        current = Cyan,
        error   = Red,
        mode    = Orange;
};

struct color c;

// build the coordinates from points on the circumference of circles r1 and r2
// calculates 6 coordinates in order to draw a polygon of 4 triangles
// needle shape is that of a segment of the dial face an angle from its origin
// the needles width is 6, as that draws most clearly for the size of dial used
// buildNeedle returns void, as the destination array is passed in along with
// the dial struct to avoid unnecessary globals
void buildNeedle(dial *d, int angle, int *array);

// draw needle from array of coordinates in such a fashion
// 1----2----3
// |   /|\   |
// |  / | \  |
// | /  |  \ |
// |/   |   \|
// 4----5----6
void drawNeedle(int *array, int color);

// check if two needle coordinate arrays are the same
bool needleCompare(int *a, int *b);

// copy all the elements from needle a into needle b
void needleCopy(int *a, int *b);

// target value is printed in bold and in magenta as the top value in the dial
void drawTargetValue(dial d);

// clearing takes place by re-drawing previous value with the background color
void clearTargetValue(dial d);

// current value is printed in cyan below the target value
void drawCurrentValue(dial d);

// clearing takes place by re-drawing previous with the background color
void clearCurrentValue(dial d);

// target needle is drawn at the coresponding angle on the face of the dial
// in the target color magenta
void drawTargetNeedle(dial d);

// clearing takes place by re-drawing previous needle position with the face
// color of the dial
void clearTargetNeedle(dial d);

// current needle is drawn at the coresponding angle onf the face of the dial
// in the current color Cyan
void drawCurrentNeedle(dial d);

// clearing takes place by re-drawing previous needle position with the face
// color of the dial
void clearCurrentNeedle(dial d);

// this error gets drawn in the center of the dial face for over range errors
void drawErrorUpper(dial d);

// clearing takes place by re-drawing the error message with background color
void clearErrorUpper(dial d);

// this error gets drawn in the center of the dial face for under range errors
void drawErrorLower(dial d);

// clearing takes place by re-drawing the error message with background color
void clearErrorLower(dial d);

// adjust needle angle for a min of 270 degrees up to a max of -45 degrees
int calcAngle(dial d, float value);

// use this to update the target and angle when from a position within the
// possible range of positions on a dial
void updateTargetByPosition(dial *d);

// use this to update the target and angle when issuing an update for an
// increment int the positive or negative direction
void updateTargetByDirection(dial *d);

// use this to update target and angle when issuing an update to a specific
// angle on the dial face
void updateTargetByAngle(dial *d);

// use this to update target and angle when issuing update for a specific
// target value
void updateTargetByValue(dial *d);

// use this to update the current and angle when from a position within the
// possible range of positions on a dial
void updateCurrentByPosition(dial *d);

// use this to update the current and angle when issuing an update for an
// increment int the positive or negative direction
void updateCurrentByDirection(dial *d);

// use this to update current and angle when issuing an update to a specific
// angle on the dial face
void updateCurrentByAngle(dial *d);

// use this to update current and angle when issuing update for a specific
// current value
void updateCurrentByValue(dial *d);

// draws the face of the dial and its label by drawing two circles of radius
// r1, and r2. r1 being the outside circumference, and r2 a smaller diameter
// within r1 and the same color as the background. the bottom notch is removed
// from the dial face by a centered right angle triangle and the dials label
// drawn centered within the notch
void drawDialBase(dial d);

// never called, but can wipe the dial from the screen by drawing over it
void clearDialBase(dial d);

// draws both target value and target needle elements and handles clearing of
// previous drawings by way of previous and current state attributes
void drawTargetElements(dial *d);

// draws both current value and current needle elements and handles clearing of
// previous drawings by way of previous and current state attributes
void drawCurrentElements(dial *d);
