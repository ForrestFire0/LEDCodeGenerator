#define NUM_LEDS_PER_STRIP 450
#define NUM_STRIPS 2
#define NUM_LEDS 867
CRGB leds[NUM_LEDS];
#define fr(f) {if(f >= 256) f-=256;};
#define fri(f, i) {f+=i;if(f >= 256) f-=256;};
#include <FastLED.h>
//stuff for BriansFunction
CRGBPalette16 currentPalette  = CloudColors_p;
TBlendType    currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;

void startLEDs() {
    //pins 12, 13, 14 and 15 (or pins 6,7,5 and 8 on the NodeMCU boards/pin layout).
    FastLED.addLeds<WS2812B_PORTA, NUM_STRIPS>(leds, NUM_LEDS_PER_STRIP);
    FastLED.show();
#ifdef ENABLE_WIFI
    byte i;
    while (WiFi.status() != WL_CONNECTED) {
        i++;
        FastLED.showColor(CRGB::Blue, i);
        Serial.print(".");
        delay(5);
    }
    FastLED.clear(true);
#endif
}

void runInitLEDS() {
    for (int i = 0; i < 5; i++) {
        FastLED.showColor(CRGB::White, 10);
        delay(100);
        FastLED.showColor(CRGB::White, 0);
        server.handleClient();
        delay(50);
    }
    FastLED.clear(true);
}

float spdInc(byte speed) {
    if (speed == 0) return 0;
    return 0.0281 * exp(0.0301 * speed);
}

bool needFlip; //Set to true to flip the second half of the array.
bool needUnFlip; //Set to true to flip the second half of the array back after writing and restore the state of the LED array.
bool needWrite;
float i; //A variable that measures step for all functions. Added to by speed.

void runLEDs() {
    needWrite = true;
    switch (selected) {
        case RGB_ROTATE: {
                float inc = spdInc(d.rr.speed);
                fri(d.rr.red, inc)
                fri(d.rr.green, inc)
                fri(d.rr.blue, inc)
                setAll(CRGB(sin8(d.rr.red), sin8(d.rr.green), sin8(d.rr.blue)));
            }
            break;
        case RANDOM:
            i += spdInc(d.r.speed); //I increments with speed.
            if (i >= 60) { //one frame
                for (uint16_t i = 0; i < NUM_LEDS; i++) {
                    leds[i] = CHSV(random(255), 255, 255);
                }
                i = 0;
            }
            //recieves a variable between 0 and 60. Turn to byte between 0 and 255
            FastLED.setBrightness(255 - cos8((byte) i * 4.26666667));
            break;
        case DOT:
            leds[d.d.intled] = d.d.secondaryColor;
            d.d.led += spdInc(d.d.speed);
            d.d.intled = (int) d.d.led;
            if (d.d.intled == NUM_LEDS) d.d.led = 0;
            leds[d.d.intled] = d.d.color;
            break;
        case HSV_PULSE: {
                fri(i, spdInc(d.hp.pulseSpeed)); //I increments with speed. Rollover increment.
                fri(d.hp.h, spdInc(d.hp.hueRate)); //I increments with speed. Rollover increment.
                byte brightness;
                if (d.hp.sinusoidalDimming)
                    brightness = (float) d.hp.pulseIntensity * (0.5 * sin(0.0245436926 * i) + 0.5) + (255 - d.hp.pulseIntensity);
                else
                    brightness = 255 * (i > 128);
                setAll(CHSV((byte) d.hp.h, 255, brightness));
            }
            break;
        case SOLID:
            needWrite = false;
            if (d.s.color != d.s.oldcolor) {
                setAll(d.s.color);
                needWrite = true;
                d.s.oldcolor = d.s.color;
            }
            break;
        case PULSE:
            setAll(d.p.color);
            fri(i, spdInc(d.p.pulseSpeed)); //I increments with speed. Rollover increment.
            if (d.p.sinusoidalDimming)
                FastLED.setBrightness((float) d.p.pulseIntensity * (0.5 * sin(0.0245436926 * i) + 0.5) + (255 - d.p.pulseIntensity));
            else
                FastLED.setBrightness(255 * (i > 128));
            break;
        case PULSE_RANDOM:
            i += spdInc(d.pr.pulseSpeed); //I increments with speed.
            if (i >= 60) { //one second
                setAll(CHSV(random(255), 255, 255));
                i = 0;
            }
            if (d.pr.sinusoidalDimming)
                FastLED.setBrightness(255 - cos8((byte) i * 4.26666667));
            else
                FastLED.setBrightness(255 * (i > 30));
            break;
        case FIREWORKS:
            clearLEDs();
            if (d.f.fireworkCount != d.f.oldfc) { //if the number of fireworks has been updated.
                byte oldSize = min(d.f.oldfc, d.f.fireworkCount);
                Firework old[oldSize];
                for (int i = 0; i < oldSize; i++) {
                    old[i] = d.f.fireworks[i];
                }
                delete[] d.f.fireworks;
                d.f.fireworks = new Firework[d.f.fireworkCount]();
                for (int i = 0; i < oldSize; i++) {
                    d.f.fireworks[i] = old[i];
                }
                d.f.oldfc = d.f.fireworkCount;
            }
            for (int i = 0; i < d.f.fireworkCount; i++) {
                if (d.f.fireworks[i].lifetime > d.f.fireworks[i].maxSpread * 10) {
                    d.f.fireworks[i].maxSpread = 0;
                }
            }
            //First, if any of them don't exist, we need to make them.
            for (int i = 0; i < d.f.fireworkCount; i++) {
                if (d.f.fireworks[i].maxSpread == 0) {
                    d.f.fireworks[i].lifetime = 1;
                    d.f.fireworks[i].maxSpread = (byte) random(20, 40);
                    d.f.fireworks[i].location = (unsigned int) random(NUM_LEDS);
                    d.f.fireworks[i].hue = (byte) random(255);
                }
                d.f.fireworks[i].lifetime++; //Second, we need to update said fireworks. Location stays the same, but lifetime should increase.
            }

            //Now, lets animate the fireworks.
            for (int i = 0; i < d.f.fireworkCount; i++) {
                float sLife = ((float) d.f.fireworks[i].lifetime) / 60.0;
                float s1 = exp(-sLife); //The current spread.
                float b1 = pow(sLife, 0.25); //The initial
                float brightness = min(s1, b1); //Use s1 for brightness, unless b1 is smaller.
                byte spread = (float) d.f.fireworks[i].maxSpread * b1; //Use b1 for spread.
                int8_t skip = -1;
                if (d.f.fireworks[i].lifetime > d.f.fireworks[i].maxSpread * 6) {
                    skip = (float) d.f.fireworks[i].maxSpread * pow((float) (d.f.fireworks[i].lifetime - (d.f.fireworks[i].maxSpread * 6)) / 60.0, 0.5);
                }
                //Now we gotta set those LEDs, however on the edge it should be dimmer.
                for (int local_loc = -spread; local_loc <= spread; local_loc++) {
                    if (skip < abs(local_loc)) {
                        register byte pixBrightness = 255 * (-pow(local_loc / spread, 4) + 1) * brightness;
                        register unsigned int global_loc = d.f.fireworks[i].location + local_loc;
                        leds[getLEDIndex(global_loc)] += CHSV(d.f.fireworks[i].hue + random(-30, 30), 255, pixBrightness);
                    }
                }
            }
            break;
        case WAVES: {
                if (d.w.waveCount != d.w.oldsize) {
                    byte oldSize = min(d.w.waveCount, d.w.oldsize);
                    MovingVertex old[oldSize];
                    for (int i = 0; i < oldSize; i++) {
                        old[i] = d.w.pts[i];
                    }
                    delete[] d.w.pts;
                    d.w.pts = new MovingVertex[d.w.waveCount]();
                    for (int i = 0; i < oldSize; i++) {
                        d.w.pts[i] = old[i];
                    }
                    d.w.oldsize = d.w.waveCount;
                }
                //Determine if we need to re-randomize any of them (aka, don't change where they currently are, just where they are going.
                for (byte i = 0; i < d.w.waveCount; i++) {
                    if (d.w.pts[i].iloc == 0) {
                        d.w.pts[i].location = random(NUM_LEDS); //Move to it by
#define MAX_MOVE 1000
                        d.w.pts[i].finalLocation = getLEDIndex(d.w.pts[i].location + random(-MAX_MOVE, MAX_MOVE)); //Move to it by a random amount
                        if (d.w.random)
                            d.w.pts[i].finalC = CHSV(random(255), 255, 255); //Soon, select between a list of colors, for now, selected randomly. Maybe option to select randomly? Checkbox
                        else {
                            if (d.w.colorsLength != 0) {
                                d.w.pts[i].finalC = d.w.colors[i % d.w.colorsLength];
                            }
                            else {
                                d.w.pts[i].finalC = CRGB::Black;
                            }
                        }
                    } else if (d.w.pts[i].iloc == d.w.pts[i].finalLocation) {
                        d.w.pts[i].finalLocation += random(-MAX_MOVE, MAX_MOVE); //Move to it by
                        if (d.w.pts[i].finalLocation < 0) d.w.pts[i].finalLocation += NUM_LEDS;
                        d.w.pts[i].finalC = CHSV(random(255), 255, 255); //Soon, select between a list of colors, for now, selected randomly. Maybe option to select randomly? Checkbox
                    }
                }
                //Increment the location and color.
                for (byte i = 0; i < d.w.waveCount; i++) {
                    d.w.pts[i].location += (d.w.pts[i].finalLocation - d.w.pts[i].location) * 0.02 * spdInc(d.w.speed);
                    d.w.pts[i].currentC = d.w.pts[i].currentC.lerp8(d.w.pts[i].finalC, 3);
                    d.w.pts[i].iloc = (int) (d.w.pts[i].location + 0.5);
                }
                //Sort those motherf***ers
                MovingVertex temp;
                for (byte i = 0; i < d.w.waveCount; i++)
                {
                    for (byte j = i + 1; j < d.w.waveCount; j++)
                    {
                        if (d.w.pts[i].location == d.w.pts[j].location)
                            d.w.pts[i].location += 1;
                        if (d.w.pts[i].location > d.w.pts[j].location)
                        {
                            temp = d.w.pts[i];
                            d.w.pts[i] = d.w.pts[j];
                            d.w.pts[j] = temp;
                        }
                    }
                }

                if (d.w.waveCount == 0) {
                    setAll(CRGB::Black);
                    return;
                }
                if (d.w.waveCount == 1)
                    setAll(d.w.pts[1].currentC);
                //Nothing every dies, it just moves. We might need to resize the array however, and create new ones.
                //We need to sort the array by index.
                MovingVertex back = d.w.pts[d.w.waveCount - 1];
                MovingVertex front = d.w.pts[0];
                byte currentFrontIndex = 0;
                //Now, we need to go through each LED
                for (int i = 0; i < NUM_LEDS; i++) {
                    if (i == front.iloc) {
                        //it is time to advance.
                        back = front; //Set the back pointer to point at the front
                        currentFrontIndex++;
                        if (currentFrontIndex == d.w.waveCount)
                            front = d.w.pts[0];
                        else
                            front = d.w.pts[currentFrontIndex];
                        leds[i] = back.currentC;
                    } else {
                        //Now, lets find the color by linearly interpolating front the back to the front.
                        //i will not equal front location, however it will equal back location often.
                        register byte frac;
                        if (front.iloc > back.iloc)
                            frac = ((i - back.iloc) * 256) / (front.iloc - back.iloc);
                        else {
                            if (i >= back.iloc)
                                frac = ((i - back.iloc) * 256) / (int)(front.iloc - back.iloc + NUM_LEDS);
                            else
                                frac = ((i - back.iloc + NUM_LEDS) * 256) / (int)(front.iloc - back.iloc + NUM_LEDS);
                        }
                        leds[i] = back.currentC.lerp8(front.currentC, frac);
                    }
                }
            }
            break;
        case SHIFT:
            //If the VLA is empty, turn off the LEDs and return.
            if (d.sh.colorsLength == 0) {
                clearLEDs();
                break;
            }
            //If either index is above the size of colors, lower it.
            if (d.sh.newIndex >= d.sh.colorsLength) d.sh.newIndex = d.sh.colorsLength - 1;
            if (d.sh.oldIndex >= d.sh.colorsLength) d.sh.oldIndex = d.sh.colorsLength - 1;
            //Increment I.
            d.sh.i += spdInc(d.sh.speed);
            //Overflow/remapping colors
            if (d.sh.i > 100) {
                d.sh.i = 0;
                d.sh.oldIndex = d.sh.newIndex;
                d.sh.newIndex = random(0, colorsLength);
            }
            //If in the first 1/4, set it based on how much time has elapsed.
            if (d.sh.i < 25.6) {
                setAll(d.sh.colors[d.sh.oldIndex].lerp8(d.sh.colors[d.sh.newIndex], (int) (d.sh.i * 10)));
            } else //Otherwise, set it just to the new color.
                setAll(d.sh.colors[d.sh.newIndex]);
            break;
        case OFF:
            needWrite = false;
            break;
        case BRIANS_FUNCTION:
            ChangePalettePeriodically();
            static uint8_t startIndex = 0;
            startIndex = startIndex + 1; /* motion speed */
            FillLEDsFromPaletteColors( startIndex);
            break;
        case BRIANS_FUNCTION_2:
            currentBlending = LINEARBLEND;
            startIndex = 0;
            startIndex = startIndex + 1; /* motion speed */
            currentPalette = ForestColors_p;
            FillLEDsFromPaletteColors(startIndex);
            break;
        case BRIANS_FUNCTION_STATIC:
            currentBlending = LINEARBLEND;
            startIndex = 0;
            startIndex = startIndex + 1; /* motion speed */
            currentPalette = OceanColors_p;
            FillLEDsFromPaletteColors(startIndex);
            break;
    }
    if (!needWrite) return;
    if (needFlip) {
        //                    (      658                )
        for (int i = 450; i < (NUM_LEDS - 450) / 2 + 450; i++) {
            register unsigned int ind1 = i;
            register unsigned int ind2 = NUM_LEDS - (i - 450) - 1;
            CRGB temp = leds[ind1];
            leds[ind1] = leds[ind2];
            leds[ind2] = temp;
        }
    }
    FastLED.show();
    if (needUnFlip) {
        for (int i = 450; i < (NUM_LEDS - 450) / 2 + 450; i++) {
            register unsigned int ind1 = i;
            register unsigned int ind2 = NUM_LEDS - (i - 450) - 1;
            CRGB temp = leds[ind1];
            leds[ind1] = leds[ind2];
            leds[ind2] = temp;
        }
    }
}



void clearLEDs() {
    memset(leds, 0, sizeof(CRGB)*NUM_LEDS);
}

void setAll(CRGB color) {
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
}

inline __attribute__((always_inline)) unsigned int getLEDIndex(int i)  {
    if (i >= NUM_LEDS) i -= NUM_LEDS;
    if (i < 0) i += NUM_LEDS;
    return i;
}


void printLEDs() {
    for (int i = 0; i < 867; i++) {
        if (leds[i].r > 0 || leds[i].g > 0 || leds[i].b > 0) {
            Serial.print(i);
            Serial.print(" - ");
            prgb(leds[i]);
        }
    }
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
    uint8_t brightness = 255;
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;
    leds[0] = ColorFromPalette(CloudColors_p, colorIndex, brightness, LINEARBLEND);
    colorIndex += 3;
}

void ChangePalettePeriodically()
{
    uint8_t secondHand = (millis() / 1000) % 60;
    static uint8_t lastSecond = 99;

    if ( lastSecond != secondHand) {
        lastSecond = secondHand;
        if ( secondHand ==  0)  {
            currentPalette = RainbowColors_p;
            currentBlending = LINEARBLEND;
        }
        if ( secondHand == 10)  {
            currentPalette = RainbowStripeColors_p;
            currentBlending = NOBLEND;
        }
        if ( secondHand == 15)  {
            currentPalette = RainbowStripeColors_p;
            currentBlending = LINEARBLEND;
        }
        if ( secondHand == 20)  {
            SetupPurpleAndGreenPalette();
            currentBlending = LINEARBLEND;
        }
        if ( secondHand == 25)  {
            SetupTotallyRandomPalette();
            currentBlending = LINEARBLEND;
        }
        if ( secondHand == 30)  {
            SetupBlackAndWhiteStripedPalette();
            currentBlending = NOBLEND;
        }
        if ( secondHand == 35)  {
            SetupBlackAndWhiteStripedPalette();
            currentBlending = LINEARBLEND;
        }
        if ( secondHand == 40)  {
            currentPalette = CloudColors_p;
            currentBlending = LINEARBLEND;
        }
        if ( secondHand == 45)  {
            currentPalette = PartyColors_p;
            currentBlending = LINEARBLEND;
        }
        if ( secondHand == 50)  {
            currentPalette = myRedWhiteBluePalette_p;
            currentBlending = NOBLEND;
        }
        if ( secondHand == 55)  {
            currentPalette = myRedWhiteBluePalette_p;
            currentBlending = LINEARBLEND;
        }
    }
}

void SetupTotallyRandomPalette()
{
    for ( int i = 0; i < 16; i++) {
        currentPalette[i] = CHSV( random8(), 255, random8());
    }
}

void SetupBlackAndWhiteStripedPalette()
{
    // 'black out' all 16 palette entries...
    fill_solid( currentPalette, 16, CRGB::Black);
    // and set every fourth one to white.
    currentPalette[0] = CRGB::White;
    currentPalette[4] = CRGB::White;
    currentPalette[8] = CRGB::White;
    currentPalette[12] = CRGB::White;

}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
    CRGB purple = CHSV( HUE_PURPLE, 255, 255);
    CRGB green  = CHSV( HUE_GREEN, 255, 255);
    CRGB black  = CRGB::Black;

    currentPalette = CRGBPalette16(
                         green,  green,  black,  black,
                         purple, purple, black,  black,
                         green,  green,  black,  black,
                         purple, purple, black,  black );
}



const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
    CRGB::Red,
    CRGB::Gray, // 'white' is too bright compared to red and blue
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Black,

    CRGB::Red,
    CRGB::Red,
    CRGB::Gray,
    CRGB::Gray,
    CRGB::Blue,
    CRGB::Blue,
    CRGB::Black,
    CRGB::Black
};
