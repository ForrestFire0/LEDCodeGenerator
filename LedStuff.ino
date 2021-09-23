#define NUM_LEDS_PER_STRIP 450
#define NUM_STRIPS 2
#define NUM_LEDS 867
CRGB leds[NUM_LEDS];
#define fr(f) {if(f >= 256) f-=256;};
#define fri(f, i) {f+=i;if(f >= 256) f-=256;};

void startLEDs() {
    //pins 12, 13, 14 and 15 (or pins 6,7,5 and 8 on the NodeMCU boards/pin layout).
    FastLED.addLeds<WS2812B_PORTA, NUM_STRIPS>(leds, NUM_LEDS_PER_STRIP);
#ifdef ENABLE_INTRO
    FastLED.show();
    delay(1000);
    for (int i = 0; i < 255; i++) {
        FastLED.showColor(CRGB::Blue, i);
        delay(5);
    }
    FastLED.clear(true);
#endif
}

void blip() {
    FastLED.showColor(CRGB::White, 10);
    delay(100);
    FastLED.showColor(CRGB::White, 0);

}
void runInitLEDS() {
    for (int i = 0; i < 5; i++) {
        blip();
        delay(100);
    }
    FastLED.clear(true);
}

float spdInc(byte speed) {
    return 0.0281 * exp(0.0301 * speed);
}

bool needFlip; //Set to true to flip the second half of the array.
float i; //A variable that measures step for all functions. Added to by speed.

void runLEDs() {
    switch (selected) {
        case RGB_ROTATE: {
                float inc = spdInc(d.rr.speed);
                fri(d.rr.red, inc)
                fri(d.rr.green, inc)
                fri(d.rr.blue, inc)
                setAll(CRGB(sin8(d.rr.red), sin8(d.rr.green), sin8(d.rr.blue)));
            } break;
        case RANDOM:
            i += spdInc(d.r.speed); //I increments with speed.
            if (i >= REFRESH_RATE) { //one frame
                for (uint16_t i = 0; i < NUM_LEDS; i++) {
                    leds[i] = CHSV(random(255), 255, 255);
                }
                i = 0;
            }
            //recieves a variable between 0 and 60. Turn to byte between 0 and 255
            FastLED.setBrightness(255 - cos8((byte) i * 4.26666667));
            break;
        case DOT:
            leds[d.d.led] = d.d.secondaryColor;
            d.d.led += spdInc(d.d.speed);
            if (d.d.led == NUM_LEDS) d.d.led = 0;
            leds[d.d.led] = d.d.color;
            break;
        case HSV_ROTATE:
            fri(i, spdInc(d.hr.pulseSpeed)); //I increments with speed. Rollover increment.
            fri(d.hr.h, spdInc(d.hr.hueRate)); //I increments with speed. Rollover increment.
            setAll(CHSV((byte) d.hr.h, 255, (float) d.hr.pulseIntensity * (0.5 * sin(0.0245436926 * i) + 0.5) + (255 - d.hr.pulseIntensity)));
            break;
        case SOLID:
            setAll(d.s.color);
            break;
        case PULSE:
            setAll(d.p.color);
            fri(i, spdInc(d.p.pulseSpeed)); //I increments with speed. Rollover increment.
            FastLED.setBrightness((float) d.p.pulseIntensity * ((float)sin8((byte)i) / 255) + (255 - d.p.pulseIntensity)); //With pulse intensity at 10, we do 10*sin() + 255-10
            break;
        case PULSE_RANDOM:
            i += spdInc(d.pr.pulseSpeed); //I increments with speed.
            if (i >= 60) { //one second
                setAll(CHSV(random(255), 255, 255));
                i = 0;
            }
            //recieves a variable between 0 and 60. Turn to byte between 0 and 255
            FastLED.setBrightness(255 - cos8((byte) i * 4.26666667));
            break;
        case FIREWORKS:
            clearLEDs();
            if (d.f.fireworkCount != d.f.oldfc) { //if the number of fireworks has been updated.
                //Allocate a temporary array. Set it equal to the data
                //                ps("Reallocating array...");
                //                pt(d.f.fireworkCount);
                //                pt(d.f.oldfc);
                Firework old[min(d.f.oldfc, d.f.fireworkCount)];
                for (int i = 0; i < min(d.f.oldfc, d.f.fireworkCount); i++) {
                    old[i] = d.f.fireworks[i];
                }
                delete[] d.f.fireworks;
                // Create the new array. For the
                d.f.fireworks = new Firework[d.f.fireworkCount]();
                for (int i = 0; i < d.f.oldfc; i++) {
                    d.f.fireworks[i] = old[i];
                }
                d.f.oldfc = d.f.fireworkCount;
            }
            for (int i = 0; i < d.f.fireworkCount; i++) {
                //If the lifetime is bigger than the maxSpread by a constant, it is time for it to die.
                if (d.f.fireworks[i].lifetime > d.f.fireworks[i].maxSpread * 8) {
                    //                    ps("Killing firework at index");
                    //                    pt(i);
                    d.f.fireworks[i].maxSpread = 0;
                    //                    pfire(d.f.fireworks[i]);
                }
            }
            //First, if any of them don't exist, we need to make them.
            for (int i = 0; i < d.f.fireworkCount; i++) {
                if (d.f.fireworks[i].maxSpread == 0) {
                    //                    ps("Making firework at index");
                    //                    pt(i);
                    d.f.fireworks[i].lifetime = 1;
                    d.f.fireworks[i].maxSpread = (byte) random(10, 40);
                    d.f.fireworks[i].location = (unsigned int) random(0, NUM_LEDS);
                    d.f.fireworks[i].hue = (byte) random(255);
                    //                    pfire(d.f.fireworks[i]);
                }
            }
            //            ps("Done making fireworks");
            //Second, we need to update said fireworks. Location stays the same, but lifetime should increase.
            for (int i = 0; i < d.f.fireworkCount; i++) {
                d.f.fireworks[i].lifetime++;
            }

            //Now, lets animate the fireworks.
            for (int i = 0; i < d.f.fireworkCount; i++) {
                //                ps("Animating");
                //                pfire(d.f.fireworks[i]);
                float sLife = ((float) d.f.fireworks[i].lifetime) / 60.0;
                //                pt(sLife);
                float s1 = exp(-sLife); //The current spread.
                float b1 = pow(sLife, 0.166666667); //The initial
                float brightness = min(s1, b1); //Use s1 for brightness, unless b1 is smaller.
                byte spread = (float) d.f.fireworks[i].maxSpread * b1; //Use b1 for spread.
                //                pt(b1);
                //                pt(brightness);
                //                pt(spread);
                //Now we gotta set those LEDs, however on the edge it should be dimmer.
                for (int local_loc = -spread; local_loc <= spread; local_loc++) {
                    register byte pixBrightness = 255 * (-pow(local_loc / spread, 4) + 1) * brightness;
                    register int global_loc = d.f.fireworks[i].location + local_loc;
                    addToLED(global_loc, CHSV(d.f.fireworks[i].hue, 255, pixBrightness));
                    //^= because there might be overlapping fireworks. It should be brighter.
                }
            }
            break;

    }
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
}


void clearLEDs() {
    memset(leds, 0, sizeof(CRGB)*NUM_LEDS);
}

void setAll(CRGB color) {
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        leds[i] = color;
    }
}

inline __attribute__((always_inline)) void setLED(int i, CRGB color)  {
    if (i >= NUM_LEDS) i -= NUM_LEDS;
    if (i < 0) i += NUM_LEDS;
    leds[i] = color;
}

inline __attribute__((always_inline)) void addToLED(int i, CRGB color) {
    if (i >= NUM_LEDS) i -= NUM_LEDS;
    if (i < 0) i += NUM_LEDS;
    leds[i] += color;
}
