// ! Implementation of dial ! ==================================================

#include "dial.h"
#include "util.h"

void buildNeedle(dial *d, int angle, int *array) {
    float degToRad    = 0.0174532925;
    float radianMid   = angle * degToRad;
    float radianSide0 = (angle - (6 / 2)) * degToRad;
    float radianSide1 = (angle + (6 / 2)) * degToRad;
    int   needle[12];

    // radius * cosine of an angle gives the x from the origin.
    // radius * sine of an angle gives the y from the origin.
    // add for m_x and subtract for m_y
    int x1 = d->x + int((r1 + 1) * cos(radianSide0));
    int y1 = d->y - int((r1 + 1) * sin(radianSide0));
    int x4 = d->x + int((r2 + 1) * cos(radianSide0));
    int y4 = d->y - int((r2 + 1) * sin(radianSide0));
    int x2 = d->x + int((r1 + 1) * cos(radianMid));
    int y2 = d->y - int((r1 + 1) * sin(radianMid));
    int x5 = d->x + int((r2 + 1) * cos(radianMid));
    int y5 = d->y - int((r2 + 1) * sin(radianMid));
    int x3 = d->x + int((r1 + 1) * cos(radianSide1));
    int y3 = d->y - int((r1 + 1) * sin(radianSide1));
    int x6 = d->x + int((r2 + 1) * cos(radianSide1));
    int y6 = d->y - int((r2 + 1) * sin(radianSide1));

    // global array, functions that use this capture needle[] after use.
    // it's a pretty dirty way to do it, but whatever.
    needle[0] = x1, needle[1] = y1;
    needle[2] = x2, needle[3] = y2;
    needle[4] = x3, needle[5] = y3;
    needle[6] = x4, needle[7] = y4;
    needle[8] = x5, needle[9] = y5;
    needle[10] = x6, needle[11] = y6;
    needleCopy(needle, array);
}

void drawNeedle(int *array, int color) {
    int x1 = array[0], y1 = array[1];
    int x2 = array[2], y2 = array[3];
    int x3 = array[4], y3 = array[5];
    int x4 = array[6], y4 = array[7];
    int x5 = array[8], y5 = array[9];
    int x6 = array[10], y6 = array[11];
    display.fillTriangle(x1, y1, x2, y2, x4, y4, color);
    display.fillTriangle(x4, y4, x5, y5, x2, y2, color);
    display.fillTriangle(x2, y2, x3, y3, x6, y6, color);
    display.fillTriangle(x5, y5, x6, y6, x2, y2, color);
}

bool needleCompare(int *a, int *b) {
    for (int n = 0; n < 12; n++) {
        if (a[n] != b[n])
            return false;
    }
    return true;
}

void needleCopy(int *a, int *b) {
    for (int n = 0; n < 12; n++)
        b[n] = a[n];
}

void drawTargetValue(dial d) {
    display.setColor(c.target);
    display.setFont(largeFontBold);
    display.print(String(d.targetCurrent, d.precision), d.targetX, d.targetY);
}

void clearTargetValue(dial d) {
    display.setColor(c.back);
    display.setFont(largeFontBold);
    display.print(String(d.targetPrevious, d.precision), d.targetX, d.targetY);
}

void drawCurrentValue(dial d) {
    display.setColor(c.current);
    display.setFont(largeFont);
    display.print(String(d.currentCurrent, d.precision), d.currentX, d.currentY);
}

void clearCurrentValue(dial d) {
    display.setColor(c.back);
    display.setFont(largeFont);
    display.print(String(d.currentPrevious, d.precision), d.currentX, d.currentY);
}

void drawTargetNeedle(dial d) {
    drawNeedle(d.targetNeedle, c.target);
}

void clearTargetNeedle(dial d) {
    drawNeedle(d.targetNeedlePrevious, c.back);
}

void drawCurrentNeedle(dial d) {
    drawNeedle(d.currentNeedle, c.current);
}

void clearCurrentNeedle(dial d) {
    drawNeedle(d.currentNeedlePrevious, c.back);
}

void drawErrorUpper(dial d) {
    display.setColor(c.error);
    display.setFont(largeFontBold);
    display.print(d.errorUpper, d.errorX, d.errorY);
}

void clearErrorUpper(dial d) {
    display.setColor(c.back);
    display.setFont(largeFontBold);
    display.print(d.errorUpper, d.errorX, d.errorY);
}

void drawErrorLower(dial d) {
    display.setColor(c.error);
    display.setFont(largeFontBold);
    display.print(d.errorLower, d.errorX, d.errorY);
}

void clearErrorLower(dial d) {
    display.setColor(c.back);
    display.setFont(largeFontBold);
    display.print(d.errorLower, d.errorX, d.errorY);
}

int calcAngle(dial d, float value) {
    float v = (value - d.min) / (d.max - d.min) * (minDeg - maxDeg) + maxDeg;
    // Returns 225 for min and -45 for max input values
    int resultAngle = v - offsetDeg;
    return resultAngle;
}

void updateTargetByPosition(dial *d) {
    d->target      = d->min + (d->targetPosition * d->inc);
    d->targetAngle = calcAngle(*d, d->target);
    //Serial.println(d->label);
    //Serial.println(d->target);
    //Serial.println(d->targetAngle);
}

void updateTargetByDirection(dial *d) {
    if (d->direction > 0)
        d->target = d->target + d->inc;
    if (d->direction < 0)
        d->target = d->target - d->inc;
    d->targetAngle = calcAngle(*d, d->target);
    //Serial.println(d->label);
    //Serial.println(d->target);
    //Serial.println(d->targetAngle);
}

void updateTargetByAngle(dial *d) {
    d->target      = mapf(d->targetAngle, minDeg, maxDeg, d->min, d->max);
    d->targetAngle = calcAngle(*d, d->target);
    //Serial.println(d->label);
    //Serial.println(d->target);
    //Serial.println(d->targetAngle);
}

void updateTargetByValue(dial *d) {
    if (d->target > d->max) {
        drawErrorUpper(*d);
    } else if (d->target < d->min) {
        drawErrorLower(*d);
    } else {
        d->targetAngle = calcAngle(*d, d->target);
    }
    //Serial.println(d->label);
    //Serial.println(d->target);
    //Serial.println(d->targetAngle);
}

void updateCurrentByPosition(dial *d) {
    d->current      = d->min + (d->currentPosition * d->inc);
    d->currentAngle = calcAngle(*d, d->current);
    //Serial.println(d->label);
    //Serial.println(d->target);
    //Serial.println(d->targetAngle);
}

void updateCurrentByDirection(dial *d) {
    if (d->direction > 0)
        d->current = d->current + d->inc;
    if (d->direction < 0)
        d->current = d->current - d->inc;
    d->currentAngle = calcAngle(*d, d->current);
    //Serial.println(d->label);
    //Serial.println(d->target);
    //Serial.println(d->targetAngle);
}

void updateCurrentByAngle(dial *d) {
    d->current      = mapf(d->currentAngle, minDeg, maxDeg, d->min, d->max);
    d->currentAngle = calcAngle(*d, d->current);
    //Serial.println(d->label);
    //Serial.println(d->target);
    //Serial.println(d->targetAngle);
}

void updateCurrentByValue(dial *d) {
    if (d->current > d->max) {
        drawErrorUpper(*d);
    } else if (d->current < d->min) {
        drawErrorLower(*d);
    } else {
        d->currentAngle = calcAngle(*d, d->current);
    }
    //Serial.println(d->label);
    //Serial.println(d->target);
    //Serial.println(d->targetAngle);
}

void drawDialBase(dial d) {
    display.setColor(c.face);
    display.fillCircle(d.x, d.y, r1);
    display.setColor(c.back);
    display.fillCircle(d.x, d.y, r2);
    display.fillTriangle(
        d.cutoutStartX,
        d.cutoutLowerY,
        d.cutoutMidX,
        d.cutoutUpperY,
        d.cutoutEndX,
        d.cutoutLowerY,
        c.back);
    display.setColor(c.label);
    display.setFont(smallFontBold);
    display.print(d.label, d.labelX, d.labelY);
}

void clearDialBase(dial d) {
    display.setColor(c.back);
    display.fillCircle(d.x, d.y, r1);
    display.fillTriangle(
        d.cutoutStartX,
        d.cutoutUpperY,
        d.cutoutEndX,
        d.cutoutLowerY,
        d.cutoutEndX,
        d.cutoutLowerY,
        c.back);
    display.setColor(c.back);
    display.setFont(smallFontBold);
    display.print(d.label, d.labelX, d.labelY);
}

void drawTargetElements(dial *d) {
    buildNeedle(d, d->targetAngle, d->targetNeedle);

    // handle target needle drawing and clearing
    needleCopy(d->targetNeedle, d->targetNeedleCurrent);
    if (!needleCompare(d->targetNeedleCurrent, d->targetNeedlePrevious)) {
        if (needleCompare(d->targetNeedlePrevious, d->currentNeedleCurrent)) {
            // re-draw current needle if passed over
            drawNeedle(d->targetNeedlePrevious, c.current);
        } else {
            // re-draw previous needle to delete it
            drawNeedle(d->targetNeedlePrevious, c.face);
        }
        // draw new needle
        drawNeedle(d->targetNeedleCurrent, c.target);
    }
    needleCopy(d->targetNeedleCurrent, d->targetNeedlePrevious);

    // detect if the target value is over, under, or within range
    // error(0) = within, error(1) = over, error(2) = under
    if (d->target > d->max) {
        d->error = 1;
    } else if (d->target < d->min) {
        d->error = 2;
    } else {
        d->error = 0;
    }

    // only draw values if there's no error
    if (d->error == 0) {
        // redraw values coming out of over range error
        if (d->errorPrevious == 1) {
            clearErrorUpper(*d);
            drawTargetValue(*d);
            drawCurrentValue(*d);
        }
        // redraw values coming out of under range error
        if (d->errorPrevious == 2) {
            clearErrorLower(*d);
            drawTargetValue(*d);
            drawCurrentValue(*d);
        }
        d->targetCurrent = d->target;
        if (d->targetCurrent != d->targetPrevious) {
            clearTargetValue(*d);
            d->targetCurrent = d->target;
            drawTargetValue(*d);
        }
        d->targetPrevious = d->targetCurrent;
    }

    // manage error drawing so it doesn't re-draw itself over and over
    d->errorCurrent = d->error;
    if (d->errorCurrent != d->errorPrevious) {
        if (d->errorCurrent == 1) {
            clearTargetValue(*d);
            clearCurrentValue(*d);
            drawErrorUpper(*d);
        }
        if (d->errorCurrent == 2) {
            clearTargetValue(*d);
            clearCurrentValue(*d);
            drawErrorLower(*d);
        }
    }
    d->errorPrevious = d->errorCurrent;
}

void drawCurrentElements(dial *d) {
    buildNeedle(d, d->currentAngle, d->currentNeedle);

    // handle current needle drawing and clearing
    needleCopy(d->currentNeedle, d->currentNeedleCurrent);
    if (!needleCompare(d->currentNeedleCurrent, d->currentNeedlePrevious)) {
        if (needleCompare(d->currentNeedlePrevious, d->targetNeedleCurrent)) {
            // re-draw target needle if passed over
            drawNeedle(d->currentNeedlePrevious, c.target);
        } else {
            // re-draw previous needle to delete it
            drawNeedle(d->currentNeedlePrevious, c.face);
        }
        // draw new needle
        drawNeedle(d->currentNeedleCurrent, c.current);
    }
    needleCopy(d->currentNeedleCurrent, d->currentNeedlePrevious);

    // handle value drawing and clearing
    d->currentCurrent = d->current;
    if (d->currentCurrent != d->currentPrevious) {
        clearCurrentValue(*d);
        d->currentCurrent = d->current;
        drawCurrentValue(*d);
    }
    d->currentPrevious = d->currentCurrent;
}
