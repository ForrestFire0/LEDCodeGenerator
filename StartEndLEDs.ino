
//Garunteed that after this function is called, the external members will be set
void startSelected() {
    needFlip = false;
    memset(&d, 0, sizeof(d));
    FastLED.setBrightness(255);
    switch (selected) {
        case RGB_ROTATE:
            d.rr.red   = (float) 256 * ((float) 0 / 3);
            d.rr.blue  = (float) 256 * ((float) 1 / 3);
            d.rr.green = (float) 256 * ((float) 2 / 3);
            break;
        case RANDOM:
            for (uint16_t i = 0; i < NUM_LEDS; i++) {
                leds[i] = CHSV(random(255), 255, 255);
            }
            break;
        case DOT:
            needFlip = true;
            needUnFlip = true;
            clearLEDs();
            d.d.color = CRGB::Red;
            d.d.speed = 2;
            d.d.led = 480;
            break;
        case FIREWORKS:
            needFlip = true;
            d.f.fireworks = new Firework[0]();
        case OFF:
            clearLEDs();
            break;
    }
}

void endSelected(Mode oldselected) {
    switch (oldselected) {
        case FIREWORKS:
            delete[] d.f.fireworks;
            break;
    }
}
