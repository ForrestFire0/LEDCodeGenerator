# LEDCodeGenerator
Hi Brian. This should be *everything* you need to get up and running for the LEDs.
1. Install the ESP8266 boards.
     1. Open Arduino settings with <kbd>Ctrl</kbd> + <kbd>,</kbd>
     1. Add `http://arduino.esp8266.com/stable/package_esp8266com_index.json` to the Additional Boards Manager URLs field.
     1. Open Tools => Board => Boards Manager.
     1. Search for "esp8266". Install the latest version of the board.
     1. Select the board "NodeMCU 1.0 (ESP-12E Module)". It is abut 60% down the list.
1. Edit FastLED code to support parralel output with our LEDs. The two changes you have to make are [here.](https://forum.makerforums.info/t/im-after-some-advice-on-ws2812b-leds-with-esp8266-and-parallel-output-specifically-its/61904/6)
1. Open ESP8266LEDServer with the Arduino IDE. Try to compile.

Ok, now ask me in person to explain how to set up a new mode for the LEDs because I am tired typing.
