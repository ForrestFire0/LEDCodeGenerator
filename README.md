# LEDCodeGenerator

Hi Brian. This should be *everything* you need to get up and running for the LEDs.

1. Install the ESP8266 boards.
    1. Open Arduino settings with <kbd>Ctrl</kbd> + <kbd>,</kbd>
    1. Add `http://arduino.esp8266.com/stable/package_esp8266com_index.json` to the Additional Boards Manager URLs
       field.
    1. Open Tools => Board => Boards Manager.
    1. Search for "esp8266". Install the latest version of the board.
    1. Select the board "NodeMCU 1.0 (ESP-12E Module)". It is abut 60% down the list.
1. Edit FastLED code to support parralel output with our LEDs. The two changes you have to make
   are [here.](https://forum.makerforums.info/t/im-after-some-advice-on-ws2812b-leds-with-esp8266-and-parallel-output-specifically-its/61904/6)
1. Open ESP8266LEDServer with the Arduino IDE. Try to compile.

How to write a new mode:
*There are three steps to writing a new mode.*

1. Writing the configuration file.
    1. Open configs.json
    2. Find the end of the 'modes' array. (It starts line two)
    3. Add a new opject ('{' bracket) on a new line. You'll have to add a comma after the end of the previous object. I
       reccomend actually keeping the {name: "Off"} object last. The order of the json file determines the order of the
       webpage dropdown.
    4. Add the name field. This is the name of the mode which appears on the HTML page.
    5. Add the 'um' (**U**ser editable **M**embers) field. This is an array of strings which are variable delarations of
       variables you want the user to be able to edit. Currently only byte, CRGB objects, and bool/boolean values are
       allowed. Bytes will show up as a slider, CRGB objects as a color picker, and booleans as a checkbox. Note: You
       should input the name of the um variable as a normal title (ex: "CRGB Secondary Color"). This will be turned into
       a vairble name following camel case rules. (ex: "CRGB secondaryColor")
    6. Add the 'im' (**I**nternal **M**embers) field. This is an array of strings which are again, variable declarations
       of variables that you want to have in the code for that mode, but you don't need the user to be able to edit. "
       Dot" (line 23) is a good example of this. It has the colors and the speeds available to the user, but it has the
       location of the dot as an internal variable. Any type is allowed.
    7. Set the "default" field (required) to the default values you want each 'um' field to have. If the type is an
       byte, set it to a number between 0 and 255. If it is a color, set it to a color string (ex: "#ff00ff", which is a
       pink).
    8. Set the max field (optional) for any byte fields. For exmaple, the Fireworks Mode (line 84) sets the maximum
       firework count to 20. This also makes the slider on the html page only go between 0 and the max number.
1. Writing the start and end code
    1. In the file "StartEndLEDs" optionally add some setup/end code that will run when the user selects that mode from
       a different mode or when the user stops that mode. The case label should be the name of the mode capitalized with
       spaces changed to underscores. (ex: RGB Rotate -> RGB_ROTATE). You won't need to use the end code unless you have
       dynamic memory allocation. Also, in the start code, you should set the variable "needFlip" to true inside your
       case block if you have a pattern that needs the LEDs to line up (aka, fixes the problem due to the data of the
       strands not being in series.)
1. Inside LEDStuff, add a label with the same capitalized/underscores name. Inside it update the LEDs based on the
   variables. FastLED.show() is automatically called. Note that variables aren't called what you set them as in
   config.json. They are put in a union, d, which then goes into an object that is named by the first letter (lower
   case) of the mode. For example, in the RGB Rotate mode, if I declared a variable speed, I would access it with
   d.rr.speed. (Line 46).

About the `#define`s: Comment them out to disable the functionality.

`ENABLE_WIFI` - Enables wifi. Without this it does not wait for wifi before starting up but *should* run on the mode set
by the selected variable.

`ENABLE_INTRO` - Enables the blue led ramp when you first plug it in. When debugging, disabling it removes the delay.

`ENABLE_FRAME_WAIT_FOR_KEY` - Allows you to single step animations. Waits for any charachter to appear on the Serial
input (just press enter in the input box). Helpful when debugging animations to slow the stream of data.

`TIME_DEBUG`- Prints out information about how long an animation + writing to leds takes.

