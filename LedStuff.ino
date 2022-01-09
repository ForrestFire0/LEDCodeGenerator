#define NUM_LEDS_PER_STRIP 100
#define NUM_STRIPS 2
#define NUM_LEDS 100
CRGB leds[NUM_LEDS];
#define fr(f)         \
  {                 \
    if (f >= 256) \
      f -= 256; \
  };
#define fri(f, i)     \
  {                 \
    f += i;       \
    if (f >= 256) \
      f -= 256; \
  };
#include <FastLED.h>
//mic
#define SOUND_PIN A0
//stuff for BriansFunction
CRGBPalette16 currentPalette = CloudColors_p;
TBlendType currentBlending;
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;
static float Gravity = -9.81;
static int StartHeight = 1;
static float Height[5];
static float ImpactVelocityStart = sqrt(-2 * Gravity * StartHeight);
static float ImpactVelocity[5];
static float TimeSinceLastBounce[5];
static int Position[5];
static long ClockTimeSinceLastBounce[5];
static float Dampening[5];

void startLEDs()
{
  //pins 12, 13, 14 and 15 (or pins 6,7,5 and 8 on the NodeMCU boards/pin layout).
  FastLED.addLeds<WS2812B_PORTA, NUM_STRIPS>(leds, NUM_LEDS_PER_STRIP);
  FastLED.show();
#ifdef ENABLE_WIFI
  byte i;
  while (WiFi.status() != WL_CONNECTED)
  {
    i++;
    FastLED.showColor(CRGB::Blue, i);
    //  if(i%500 == 0) p(".");
    delay(5);
  }
  FastLED.clear(true);
#endif
}

void runInitLEDS()
{
  for (int i = 0; i < 5; i++)
  {
    FastLED.showColor(CRGB::White, 10);
    delay(100);
    FastLED.showColor(CRGB::White, 0);
    server.handleClient();
    delay(50);
  }
  FastLED.clear(true);
}

float spdInc(byte speed)
{
  if (speed == 0)
    return 0;
  return 0.0281f * exp(0.0301f * speed);
}

bool needFlip;   //Set to true to flip the second half of the array.
bool needUnFlip; //Set to true to flip the second half of the array back after writing and restore the state of the LED array.
bool needWrite;
float i; //A variable that measures step for all functions. Added to by speed.

void runLEDs()
{
  needWrite = true;
  switch (selected)
  {
    case RGB_ROTATE:
      {
        float inc = spdInc(d.rr.speed);
        fri(d.rr.red, inc)
        fri(d.rr.green, inc)
        fri(d.rr.blue, inc)
        setAll(CRGB(sin8(d.rr.red), sin8(d.rr.green), sin8(d.rr.blue)));
      }
      break;
    case RANDOM:
      i += spdInc(d.r.speed); //I increments with speed.
      if (i >= 60)
      { //one frame
        for (uint16_t i = 0; i < NUM_LEDS; i++)
        {
          leds[i] = CHSV(random(255), 255, 255);
        }
        i = 0;
      }
      //recieves a variable between 0 and 60. Turn to byte between 0 and 255
      FastLED.setBrightness(255 - cos8((byte)i * 4.26666667));
      break;
    case DOT:
      leds[d.d.intled] = d.d.secondaryColor;
      d.d.led += spdInc(d.d.speed);
      d.d.intled = (int)d.d.led;
      if (d.d.intled == NUM_LEDS)
        d.d.led = 0;
      leds[d.d.intled] = d.d.color;
      break;
    case HSV_PULSE:
      {
        fri(i, spdInc(d.hp.pulseSpeed));   //I increments with speed. Rollover increment.
        fri(d.hp.h, spdInc(d.hp.hueRate)); //I increments with speed. Rollover increment.
        byte brightness;
        if (d.hp.sinusoidalDimming)
          brightness = (float)d.hp.pulseIntensity * (0.5 * sin(0.0245436926 * i) + 0.5) + (255 - d.hp.pulseIntensity);
        else
          brightness = 255 * (i > 128);
        setAll(CHSV((byte)d.hp.h, 255, brightness));
      }
      break;
    case SOLID:
      needWrite = false;
      if (d.s.color != d.s.oldcolor)
      {
        setAll(d.s.color);
        needWrite = true;
        d.s.oldcolor = d.s.color;
      }
      break;
    case PULSE:
      setAll(d.p.color);
      fri(i, spdInc(d.p.pulseSpeed)); //I increments with speed. Rollover increment.
      if (d.p.sinusoidalDimming)
        FastLED.setBrightness((float)d.p.pulseIntensity * (0.5 * sin(0.0245436926 * i) + 0.5) + (255 - d.p.pulseIntensity));
      else
        FastLED.setBrightness(255 * (i > 128));
      break;
    case PULSE_RANDOM:
      i += spdInc(d.pr.pulseSpeed); //I increments with speed.
      if (i >= 60)
      { //one second
        setAll(CHSV(random(255), 255, 255));
        i = 0;
      }
      if (d.pr.sinusoidalDimming)
        FastLED.setBrightness(255 - cos8((byte)i * 4.26666667));
      else
        FastLED.setBrightness(255 * (i > 30));
      break;
    case FIREWORKS:
      clearLEDs();
      if (d.f.fireworkCount != d.f.oldfc)
      { //if the number of fireworks has been updated.
        byte oldSize = min(d.f.oldfc, d.f.fireworkCount);
        Firework old[oldSize];
        for (int i = 0; i < oldSize; i++)
        {
          old[i] = d.f.fireworks[i];
        }
        delete[] d.f.fireworks;
        d.f.fireworks = new Firework[d.f.fireworkCount]();
        for (int i = 0; i < oldSize; i++)
        {
          d.f.fireworks[i] = old[i];
        }
        d.f.oldfc = d.f.fireworkCount;
      }
      for (int i = 0; i < d.f.fireworkCount; i++)
      {
        if (d.f.fireworks[i].lifetime > d.f.fireworks[i].maxSpread * 10)
        {
          d.f.fireworks[i].maxSpread = 0;
        }
      }
      //First, if any of them don't exist, we need to make them.
      for (int i = 0; i < d.f.fireworkCount; i++)
      {
        if (d.f.fireworks[i].maxSpread == 0)
        {
          d.f.fireworks[i].lifetime = 1;
          d.f.fireworks[i].maxSpread = (byte)random(20, 40);
          d.f.fireworks[i].location = (unsigned int)random(NUM_LEDS);
          d.f.fireworks[i].hue = (byte)random(255);
        }
        d.f.fireworks[i].lifetime++; //Second, we need to update said fireworks. Location stays the same, but lifetime should increase.
      }

      //Now, lets animate the fireworks.
      for (int i = 0; i < d.f.fireworkCount; i++)
      {
        float sLife = ((float)d.f.fireworks[i].lifetime) / 60.0;
        float s1 = exp(-sLife);                               //The current spread.
        float b1 = pow(sLife, 0.25);                          //The initial
        float brightness = min(s1, b1);                       //Use s1 for brightness, unless b1 is smaller.
        byte spread = (float)d.f.fireworks[i].maxSpread * b1; //Use b1 for spread.
        int8_t skip = -1;
        if (d.f.fireworks[i].lifetime > d.f.fireworks[i].maxSpread * 6)
        {
          skip = (float)d.f.fireworks[i].maxSpread * pow((float)(d.f.fireworks[i].lifetime - (d.f.fireworks[i].maxSpread * 6)) / 60.0, 0.5);
        }
        //Now we gotta set those LEDs, however on the edge it should be dimmer.
        for (int local_loc = -spread; local_loc <= spread; local_loc++)
        {
          if (skip < abs(local_loc))
          {
            register byte pixBrightness = 255 * (-pow(local_loc / spread, 4) + 1) * brightness;
            register unsigned int global_loc = d.f.fireworks[i].location + local_loc;
            leds[getLEDIndex(global_loc)] += CHSV(d.f.fireworks[i].hue + random(-30, 30), 255, pixBrightness);
          }
        }
      }
      break;
    case WAVES:
      {
        if (d.w.waveCount != d.w.oldsize)
        {
          byte oldSize = min(d.w.waveCount, d.w.oldsize);
          MovingVertex old[oldSize];
          for (int i = 0; i < oldSize; i++)
          {
            old[i] = d.w.pts[i];
          }
          delete[] d.w.pts;
          d.w.pts = new MovingVertex[d.w.waveCount]();
          for (int i = 0; i < oldSize; i++)
          {
            d.w.pts[i] = old[i];
          }
          d.w.oldsize = d.w.waveCount;
        }
        //Determine if we need to re-randomize any of them (aka, don't change where they currently are, just where they are going.
        for (byte i = 0; i < d.w.waveCount; i++)
        {
          if (d.w.pts[i].iloc == d.w.pts[i].finalLocation)
          {
#define MAX_MOVE 1000
            d.w.pts[i].finalLocation = getLEDIndex(d.w.pts[i].location + random(-MAX_MOVE, MAX_MOVE)); //Move to it by a random amount
            if (d.w.random)
              d.w.pts[i].finalC = CHSV(random(255), 255, 255); //Soon, select between a list of colors, for now, selected randomly. Maybe option to select randomly? Checkbox
            else
            {
              if (d.w.colorsLength != 0)
              {
                d.w.pts[i].finalC = d.w.colors[i % d.w.colorsLength];
              }
              else
              {
                d.w.pts[i].finalC = CRGB::Black;
              }
            }
          }
        }
        //Increment the location and color.
        for (byte i = 0; i < d.w.waveCount; i++)
        {
          d.w.pts[i].location += (d.w.pts[i].finalLocation - d.w.pts[i].location) * 0.02 * spdInc(d.w.speed);
          d.w.pts[i].iloc = (int)(d.w.pts[i].location + 0.5);
          // pmove(d.w.pts[i]);
        }
        //Sort those guys
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

        if (d.w.waveCount == 0)
        {
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
        for (int i = 0; i < NUM_LEDS; i++)
        {
          if (i == front.iloc)
          {
            //it is time to advance.
            back = front; //Set the back pointer to point at the front
            currentFrontIndex++;
            if (currentFrontIndex == d.w.waveCount)
              front = d.w.pts[0];
            else
              front = d.w.pts[currentFrontIndex];
            leds[i] = back.currentC;
          }
          else
          {
            //Now, lets find the color by linearly interpolating front the back to the front.
            //i will not equal front location, however it will equal back location often.
            register byte frac;
            if (front.iloc > back.iloc)
              frac = ((i - back.iloc) * 256) / (front.iloc - back.iloc);
            else
            {
              if (i >= back.iloc)
                frac = ((i - back.iloc) * 256) / (int)(front.iloc - back.iloc + NUM_LEDS);
              else
                frac = ((i - back.iloc + NUM_LEDS) * 256) / (int)(front.iloc - back.iloc + NUM_LEDS);
            }
            //                        leds[i] = back.currentC.lerp8(front.currentC, frac);
            leds[i] = back.finalC.lerp8(front.finalC, frac);
          }
        }
      }
      break;
    case SHIFT:
      //If the VLA is empty, turn off the LEDs and return.
      if (d.sh.colorsLength == 0)
      {
        clearLEDs();
        break;
      }
      else if (d.sh.colorsLength == 1)
      {
        setAll(d.sh.colors[0]);
        break;
      }
      else
      {
        //If either index is above the size of colors, lower it.
        if (d.sh.newIndex >= d.sh.colorsLength)
          d.sh.newIndex = d.sh.colorsLength - 1;
        if (d.sh.oldIndex >= d.sh.colorsLength)
          d.sh.oldIndex = d.sh.colorsLength - 1;
        //Increment I.
        d.sh.i += min(spdInc(d.sh.speed), 100.0f);
        //Overflow/remapping colors
        if (d.sh.i > 100)
        {
          d.sh.i = 0;
          d.sh.oldIndex = d.sh.newIndex;
          while (d.sh.newIndex == d.sh.oldIndex) //Make sure same one is selected again
            d.sh.newIndex = random(0, d.sh.colorsLength);
        } //If in the first 1/4, set it based on how much time has elapsed.
        if (d.sh.i < d.sh.transitionTime)
        {
          setAll(d.sh.colors[d.sh.oldIndex].lerp8(d.sh.colors[d.sh.newIndex], (int)(d.sh.i * 256.0f / d.sh.transitionTime)));
        }
        else
        { //Otherwise, set it just to the new color.
          setAll(d.sh.colors[d.sh.newIndex]);
        }
      }
      break;
    case OFF:
      fadeall();
      delay(100);
      break;
    case BRIANS_FUNCTION:
      {
        static float startIndex = 0;
        startIndex += (0.0281 * exp(0.0301 * d.bf.speed)); /* motion speed */
        Serial.print(0.0281 * exp(0.0301 * d.bf.speed));
        Serial.print(" " );
        Serial.print((d.bf.speed));
        Serial.print(" " );

        Serial.println(startIndex);
        //        if (startIndex >=16)
        //          startIndex = 0;
        int colorIndex = int(startIndex);

        if (d.bf.blend == 1)
          currentBlending = NOBLEND;
        else
          currentBlending = LINEARBLEND;
        CRGBPalette16 Palettes[] = {RainbowColors_p, RainbowStripeColors_p, OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, PartyColors_p, myRedWhiteBluePalette_p};
        if (d.bf.select < sizeof(Palettes))
        {
          for (int i = 0; i < 32; i++)
          {

            leds[i] = ColorFromPalette(Palettes[d.bf.select], colorIndex, d.bf.brightness, currentBlending);
            colorIndex += 1;
            Serial.print("\t");
            Serial.println(colorIndex);
            delay(5);
          }
        }
      }
      break;


    case CYLON:
      {
        byte red = 0xff;
        byte green = 0;
        byte blue = 0;
        int EyeSize = 4;
        int SpeedDelay = 10;
        int ReturnDelay = 50;
        Serial.println("hi");
        for (int i = 0; i < NUM_LEDS - EyeSize - 2; i++)
        {
          setAlls(0, 0, 0);
          setPixel(i, red / 10, green / 10, blue / 10);
          for (int j = 1; j <= EyeSize; j++)
          {
            setPixel(i + j, red, green, blue);
          }
          setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
          FastLED.show();
          delay(SpeedDelay);
        }

        delay(ReturnDelay);

        for (int i = NUM_LEDS - EyeSize - 2; i > 0; i--)
        {
          setAlls(0, 0, 0);
          setPixel(i, red / 10, green / 10, blue / 10);
          for (int j = 1; j <= EyeSize; j++)
          {
            setPixel(i + j, red, green, blue);
          }
          setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
          FastLED.show();
          delay(SpeedDelay);
        }

        delay(ReturnDelay);
      }
      break;
    case TWINKLE:
      {
        Serial.println("twinkle");
        byte red = 255;
        byte green = 0;
        byte blue = 0;
        int Count = d.t.select;
        int SpeedDelay = 100;
        boolean OnlyOne = false;
        setAlls(0, 0, 0);
        for (int i = 0; i < 10; i++) {
          leds[random(NUM_LEDS)] = d.t.color;
        }
        delay(SpeedDelay);
      }
      break;
    case MUSIC:
      {
        const int MAX = 50;

        int k = analogRead(SOUND_PIN); //sensor Value --> i
        if (k > 0) i = k;
        int timeDelay = d.m.timeDelay;
        //        Serial.print("sensor = ");
        //        Serial.println(i);

        int  center = NUM_LEDS / 2;
        int incoming;
        leds[center] = d.m.color;
        int m = d.m.max * 10;
        if (i > m) m = i;
        incoming = map(i, 0, m, 0, MAX);
        for (int j = 0; j < NUM_LEDS; j++) {
          if (i < 15) {
//            Serial.println("Music Paused");
            while (i < 15) {
              if (i > m) m = i;
              int k = analogRead(SOUND_PIN); //sensor Value --> i
              if (k > 0) i = k;
//              Serial.print("pause ");
              incoming = map(i, 0, m, 0, MAX);
//              delay(5);
            }
            leds[j] = CHSV(0, 0, 46);
          }
          else leds[j] = CRGB::Black;
        }
        for (int z = center - incoming; z < center + incoming; z++) {
          if (d.m.randomColor) {
            leds[z] = CHSV(map(i, 0, m, 0, 255), 255, 255);
          } else {
            leds[z] = d.m.color;
          }

        }
        Serial.println(i);
//        delay(timeDelay);
      }
      break;
    case THEATRE_CHASE:
      {
        byte red = 0;
        byte green = 0;
        byte blue = 255;
        int SpeedDelay = 50;
        for (int j = 0; j < 3; j++)
        { //do 2 cycles of chasing
          for (int q = 0; q < 3; q++)
          {
            for (int i = 0; i < NUM_LEDS; i = i + 3)
            {
              setPixel(i + q, red, green, blue); //turn every third pixel on
            }
            FastLED.show();

            delay(SpeedDelay);

            for (int i = 0; i < NUM_LEDS; i = i + 3)
            {
              setPixel(i + q, 0, 0, 0); //turn every third pixel off
            }
          }
        }
        break;
      }
    case FIRE:
      {
        int Cooling = d.fi.cooling;
        int Sparking = d.fi.sparking;
        int SpeedDelay = 15;
        static byte heat[NUM_LEDS];
        int cooldown;

        // Step 1.  Cool down every cell a little
        for (int i = 0; i < NUM_LEDS; i++)
        {
          cooldown = random(0, ((Cooling * 10) / NUM_LEDS) + 2);

          if (cooldown > heat[i])
          {
            heat[i] = 0;
          }
          else
          {
            heat[i] = heat[i] - cooldown;
          }
        }

        // Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for (int k = NUM_LEDS - 1; k >= 2; k--)
        {
          heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
        }

        // Step 3.  Randomly ignite new 'sparks' near the bottom
        if (random(255) < Sparking)
        {
          int y = random(7);
          heat[y] = heat[y] + random(160, 255);
          //heat[y] = random(160,255);
        }

        // Step 4.  Convert heat to LED colors
        for (int j = 0; j < NUM_LEDS; j++)
        {
          setPixelHeatColor(j, heat[j]);
        }
      }
      break;
    case BOUNCING_BALLS:
      {
        int ballSize = d.bb.ballSize;
        const int BallCount = d.bb.count;
        float Gravity = (d.bb.gravity * -1);
        int StartHeight = d.bb.height;
        for (int i = 0; i < BallCount; i++)
        {
          TimeSinceLastBounce[i] = millis() - ClockTimeSinceLastBounce[i];
          Height[i] = 0.5 * Gravity * pow(TimeSinceLastBounce[i] / 1000, 2.0) + ImpactVelocity[i] * TimeSinceLastBounce[i] / 1000;

          if (Height[i] < 0)
          {
            Height[i] = 0;
            ImpactVelocity[i] = Dampening[i] * ImpactVelocity[i];
            ClockTimeSinceLastBounce[i] = millis();

            if (ImpactVelocity[i] < 0.01)
            {
              ImpactVelocity[i] = ImpactVelocityStart;
            }
          }
          Position[i] = round(Height[i] * (NUM_LEDS - ballSize) / StartHeight);

        }
        setAll(CRGB:: Black);
        for (int i = 0; i < BallCount; i++)
        {
          for (int j = 0; j < ballSize; j++)
          {
            if (Position[i] + j > NUM_LEDS) {
              leds[NUM_LEDS - 1] = d.bb.color;
            }
            else {
              leds[Position[i] + j] = d.bb.color;
            }
          }
        }



      }
      break;
    case METEOR_RAIN:
      {
        delay(d.mr.speed );
        static uint8_t hue = 0;

        // First slide the led in one direction
        if (d.mr.i > NUM_LEDS) {
          d.mr.flip = 1;

          Serial.println("flipped");
        } else if (d.mr.i == 0) {
          d.mr.flip = 0;

          Serial.println("Unflipped");

        }
        if (d.mr.flip == 0) {
          // Set the i'th led to red
          leds[d.mr.i] = CHSV(hue++, 255, 255);
          // Show the leds
          // now that we've shown the leds, reset the i'th led to black
          // leds[i] = CRGB::Black;
          fadeall();
          // Wait a little bit before we loop around and do it again
          d.mr.i++;
        }

        // Now go in the other direction.
        if (d.mr.flip == 1) {
          // Set the i'th led to red
          leds[d.mr.i] = CHSV(hue++, 255, 255);
          // Show the leds
          // now that we've shown the leds, reset the i'th led to black
          // leds[i] = CRGB::Black;
          fadeall();
          // Wait a little bit before we loop around and do it again
          d.mr.i--;
        }

      }
      break;
  }

  if (!needWrite)
    return;
  if (needFlip)
  {
    //                    (      658                )
    for (int i = NUM_LEDS_PER_STRIP; i < (NUM_LEDS - NUM_LEDS_PER_STRIP) / 2 + NUM_LEDS_PER_STRIP; i++)
    {
      register unsigned int ind1 = i;
      register unsigned int ind2 = NUM_LEDS - (i - NUM_LEDS_PER_STRIP) - 1;
      CRGB temp = leds[ind1];
      leds[ind1] = leds[ind2];
      leds[ind2] = temp;
    }
  }
  FastLED.show();
  if (needUnFlip)
  {
    for (int i = NUM_LEDS_PER_STRIP; i < (NUM_LEDS - NUM_LEDS_PER_STRIP) / 2 + NUM_LEDS_PER_STRIP; i++)
    {
      register unsigned int ind1 = i;
      register unsigned int ind2 = NUM_LEDS - (i - NUM_LEDS_PER_STRIP) - 1;
      CRGB temp = leds[ind1];
      leds[ind1] = leds[ind2];
      leds[ind2] = temp;
    }
  }
}
void fadeall() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(250);
  }
}

void clearLEDs()
{
  memset(leds, 0, sizeof(CRGB) * NUM_LEDS);
}

void setAll(CRGB color)
{
  for (uint16_t i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = color;
  }
}

inline __attribute__((always_inline)) unsigned int getLEDIndex(int i)
{
  while (i >= NUM_LEDS)
    i -= NUM_LEDS;
  while (i < 0)
    i += NUM_LEDS;
  return i;
}

/*void printLEDs() {
    for (int i = 0; i < NUM_LEDS; i++) {
        if (leds[i].r > 0 || leds[i].g > 0 || leds[i].b > 0) {
            p(i);
            ps("-");
            crgb(leds[i]);
        }
    }
    pl();
  }*/

void ChangePalettePeriodically()
{
  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if (lastSecond != secondHand)
  {
    lastSecond = secondHand;
    if (secondHand == 0)
    {
      currentPalette = RainbowColors_p;
      currentBlending = LINEARBLEND;
    }
    if (secondHand == 10)
    {
      currentPalette = RainbowStripeColors_p;
      currentBlending = NOBLEND;
    }
    if (secondHand == 15)
    {
      currentPalette = RainbowStripeColors_p;
      currentBlending = LINEARBLEND;
    }

    if (secondHand == 40)
    {
      currentPalette = CloudColors_p;
      currentBlending = LINEARBLEND;
    }
    if (secondHand == 45)
    {
      currentPalette = PartyColors_p;
      currentBlending = LINEARBLEND;
    }
    if (secondHand == 50)
    {
      currentPalette = myRedWhiteBluePalette_p;
      currentBlending = NOBLEND;
    }
    if (secondHand == 55)
    {
      currentPalette = myRedWhiteBluePalette_p;
      currentBlending = LINEARBLEND;
    }
  }
}

void SetupTotallyRandomPalette()
{
  for (int i = 0; i < 16; i++)
  {
    currentPalette[i] = CHSV(random8(), 255, random8());
  }
}

void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid(currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
  currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
  currentPalette[12] = CRGB::White;
}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV(HUE_PURPLE, 255, 255);
  CRGB green = CHSV(HUE_GREEN, 255, 255);
  CRGB black = CRGB::Black;

  currentPalette = CRGBPalette16(
                     green, green, black, black,
                     purple, purple, black, black,
                     green, green, black, black,
                     purple, purple, black, black);
}

void setPixel(int Pixel, byte red, byte green, byte blue)
{
  // FastLED
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
}

static void setAlls(byte red, byte green, byte blue)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    setPixel(i, red, green, blue);
  }
  FastLED.show();
}

void setPixelHeatColor(int Pixel, byte temperature)
{
  // Scale 'heat' down from 0-255 to 0-191
  byte t192 = round((temperature / 255.0) * 191);

  // calculate ramp up from
  byte heatramp = t192 & 0x3F; // 0..63
  heatramp <<= 2;              // scale up to 0..252

  // figure out which third of the spectrum we're in:
  if (t192 > 0x80)
  { // hottest
    setPixel(Pixel, 255, 255, heatramp);
  }
  else if (t192 > 0x40)
  { // middle
    setPixel(Pixel, 255, heatramp, 0);
  }
  else
  { // coolest
    setPixel(Pixel, heatramp, 0, 0);
  }
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
