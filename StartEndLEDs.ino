
//Garunteed that after this function is called, the external members will be set
void startSelected()
{
  needFlip = false;
  needUnFlip = false;
  memset(&d, 0, sizeof(d));
  FastLED.setBrightness(255);
  switch (selected)
  {
  case RGB_ROTATE:
    d.rr.red = (float)256 * ((float)0 / 3);
    d.rr.blue = (float)256 * ((float)1 / 3);
    d.rr.green = (float)256 * ((float)2 / 3);
    break;
  case RANDOM:
    for (uint16_t i = 0; i < NUM_LEDS; i++)
    {
      leds[i] = CHSV(random(255), 255, 255);
    }
    break;
  case DOT:
    needFlip = true;
    needUnFlip = true;
    clearLEDs();
    break;
  case FIREWORKS:
    needFlip = true;
    d.f.fireworks = new Firework[0]();
    break;
  case OFF:
    clearLEDs();
    FastLED.show();
    break;
  case WAVES:
    needFlip = true;
    d.w.pts = new MovingVertex[0]();
    d.w.colors = new CRGB[2]();
    break;
  case BRIANS_FUNCTION:
    needFlip = true;
    currentPalette = RainbowColors_p;
    currentBlending = LINEARBLEND;

  case BOUNCING_BALLS:
  for (int i = 0; i < 5; i++)
{
    ClockTimeSinceLastBounce[i] = millis();
    Height[i] = StartHeight;
    Position[i] = 0;
    ImpactVelocity[i] = ImpactVelocityStart;
    TimeSinceLastBounce[i] = 0;
    Dampening[i] = 0.90 - float(i) / pow(5, 2);
}
    needFlip = true;
    needUnFlip = true;
  case METEOR_RAIN:
    needFlip = true;
    needUnFlip = true;
  }
}

void endSelected(Mode oldselected)
{
  switch (oldselected)
  {
  case FIREWORKS:
    delete[] d.f.fireworks;
    break;
  case WAVES:
    delete[] d.w.pts;
    delete[] d.w.colors;
    break;
  }
}
