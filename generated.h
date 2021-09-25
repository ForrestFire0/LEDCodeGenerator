struct Firework {int location; byte maxSpread; int lifetime; byte hue;};

#define pt(x) {Serial.print(#x ": "); Serial.println(x);}
#define ps(x) Serial.println(F(x));
#define pfire(x) {Serial.print(F("Firework {")); Serial.print(x.location); Serial.print(", "); Serial.print(x.maxSpread); Serial.print(", "); Serial.print(x.lifetime); Serial.print(", "); Serial.print(x.hue); Serial.println("}");};

struct RGBRotateData {
     byte speed;
     float red;
     float green;
     float blue;
};

struct RandomData {
     byte speed;
};

struct DotData {
     CRGB color;
     CRGB secondaryColor;
     byte speed;
     float led;
     uint16_t intled;
};

struct HSVPulseData {
     byte hueRate;
     byte pulseSpeed;
     byte pulseIntensity;
     bool sinusoidalDimming;
     float h;
     byte v;
     byte other;
};

struct SolidData {
     CRGB color;
     CRGB oldcolor;
};

struct PulseData {
     CRGB color;
     byte pulseSpeed;
     byte pulseIntensity;
     bool sinusoidalDimming;
     byte i;
};

struct PulseRandomData {
     byte pulseSpeed;
     bool sinusoidalDimming;
};

struct FireworksData {
     byte fireworkCount;
     byte oldfc;
     Firework* fireworks;
};

struct OffData {
};

union Data {
     RGBRotateData rr;
     RandomData r;
     DotData d;
     HSVPulseData hp;
     SolidData s;
     PulseData p;
     PulseRandomData pr;
     FireworksData f;
     OffData o;
} d;


//**LEDOptions**
char *LEDOptions[] = {"RGB Rotate", "Random", "Dot", "HSV Pulse", "Solid", "Pulse", "Pulse Random", "Fireworks", "Off"};
enum Mode {RGB_ROTATE, RANDOM, DOT, HSV_PULSE, SOLID, PULSE, PULSE_RANDOM, FIREWORKS, OFF};

//**SETTERS CODE**
void fillInArgs(Mode selected, ESP8266WebServer &server) {
    char buff[7];
    switch (selected) {
     case RGB_ROTATE:
        d.rr.speed = (byte) server.arg(1).toInt();
        break;
     case RANDOM:
        d.r.speed = (byte) server.arg(1).toInt();
        break;
     case DOT:
        server.arg(1).substring(1).toCharArray(buff, 7);
        d.d.color = CRGB(strtoul(buff, NULL, 16));
        server.arg(2).substring(1).toCharArray(buff, 7);
        d.d.secondaryColor = CRGB(strtoul(buff, NULL, 16));
        d.d.speed = (byte) server.arg(3).toInt();
        break;
     case HSV_PULSE:
        d.hp.hueRate = (byte) server.arg(1).toInt();
        d.hp.pulseSpeed = (byte) server.arg(2).toInt();
        d.hp.pulseIntensity = (byte) server.arg(3).toInt();
        d.hp.sinusoidalDimming = server.arg(4).equals("true");
        break;
     case SOLID:
        server.arg(1).substring(1).toCharArray(buff, 7);
        d.s.color = CRGB(strtoul(buff, NULL, 16));
        break;
     case PULSE:
        server.arg(1).substring(1).toCharArray(buff, 7);
        d.p.color = CRGB(strtoul(buff, NULL, 16));
        d.p.pulseSpeed = (byte) server.arg(2).toInt();
        d.p.pulseIntensity = (byte) server.arg(3).toInt();
        d.p.sinusoidalDimming = server.arg(4).equals("true");
        break;
     case PULSE_RANDOM:
        d.pr.pulseSpeed = (byte) server.arg(1).toInt();
        d.pr.sinusoidalDimming = server.arg(2).equals("true");
        break;
     case FIREWORKS:
        d.f.fireworkCount = (byte) server.arg(1).toInt();
        break;
     case OFF:
        break;
     }
}




//**HTML STRING**
char HTMLTemplate[] = "<!DOCTYPE html>\
<html lang='en'>\
<meta name='viewport' content='width=device-width, initial-scale=1.0'>\
<head>\
<title>LED Controller</title>\
<link href='https://fonts.googleapis.com/css2?family=Comfortaa&display=swap' rel='stylesheet'>\
<style>\
body {\
font-family: 'Comfortaa', cursive;\
background-color: #cccccc;\
Color: #000088;\
}\
h1 {\
font-size: 10vw;\
margin-top: 1vw;\
margin-bottom: 1vw;\
}\
h3 {\
font-size: 6vw;\
margin-bottom: 1vw;\
}\
select {\
width: min(75vw, 600px);\
font-size: min(6vw, 30px);\
}\
</style>\
</head>\
<body>\
<h3><i>Forrest and Brian's</i></h3>\
<h1>LED Controller</h1>\
<center>\
<select onchange='change()'>\
<option value='0'>RGB Rotate</option><option value='1'>Random</option><option value='2'>Dot</option><option value='3'>HSV Pulse</option><option value='4'>Solid</option><option value='5'>Pulse</option><option value='6'>Pulse Random</option><option value='7'>Fireworks</option><option value='8'>Off</option>\
</select>\
<div style='margin-bottom: 3vh' id='s'></div>\
<i style='font-size: x-large;' id='fps'></i>\
</center>\
</body>\
<script>\
let newData = false;\
setInterval(send, 100);\
setInterval(getFPS, 1500);\
const inputs = {\
     'RGB Rotate': ['Speed: <input type=\"range\" id=\"0\" max=\"255\" value=\"118\" oninput=\"u()\"><br>',1],\
     'Random': ['Speed: <input type=\"range\" id=\"0\" max=\"255\" value=\"100\" oninput=\"u()\"><br>',1],\
     'Dot': ['Color: <input type=\"color\" id=\"0\" value=\"#ff0000\" oninput=\"u()\"><br>Secondary Color: <input type=\"color\" id=\"1\" value=\"#000000\" oninput=\"u()\"><br>Speed: <input type=\"range\" id=\"2\" max=\"255\" value=\"118\" oninput=\"u()\"><br>',3],\
     'HSV Pulse': ['Hue Rate: <input type=\"range\" id=\"0\" max=\"255\" value=\"118\" oninput=\"u()\"><br>Pulse Speed: <input type=\"range\" id=\"1\" max=\"255\" value=\"118\" oninput=\"u()\"><br>Pulse Intensity: <input type=\"range\" id=\"2\" max=\"255\" value=\"127\" oninput=\"u()\"><br>Sinusoidal Dimming: <input type=\"checkbox\" id=\"3\"checked oninput=\"u()\"><br>',4],\
     'Solid': ['Color: <input type=\"color\" id=\"0\" value=\"#0f0f0f\" oninput=\"u()\"><br>',1],\
     'Pulse': ['Color: <input type=\"color\" id=\"0\" value=\"#0000ff\" oninput=\"u()\"><br>Pulse Speed: <input type=\"range\" id=\"1\" max=\"255\" value=\"118\" oninput=\"u()\"><br>Pulse Intensity: <input type=\"range\" id=\"2\" max=\"255\" value=\"127\" oninput=\"u()\"><br>Sinusoidal Dimming: <input type=\"checkbox\" id=\"3\"checked oninput=\"u()\"><br>',4],\
     'Pulse Random': ['Pulse Speed: <input type=\"range\" id=\"0\" max=\"255\" value=\"118\" oninput=\"u()\"><br>Sinusoidal Dimming: <input type=\"checkbox\" id=\"1\"checked oninput=\"u()\"><br>',2],\
     'Fireworks': ['Firework Count: <input type=\"range\" id=\"0\" max=\"30\" value=\"1\" oninput=\"u()\"><br>',1],\
     'Off': ['',0],\
};\
const s = document.getElementsByTagName('select')[0];\
s.selectedIndex = 8;\
document.getElementById('s').innerHTML = inputs[s.options[s.selectedIndex].text][0];\
function change() {\
document.getElementById('s').innerHTML = inputs[s.options[s.selectedIndex].text][0];\
u();}\
function getFPS() {\
let xmlHttp = new XMLHttpRequest();\
xmlHttp.onload = () => {\
document.getElementById('fps').innerHTML = xmlHttp.responseText + \" fps\";\
};\
xmlHttp.open('GET', '/gt', true);\
xmlHttp.send(null);\
}\
function u() { newData = true; }\
function send() {\
if (newData) {\
let xmlHttp = new XMLHttpRequest();\
const params = {};\
for (let i = 0; i < inputs[s.options[s.selectedIndex].text][1]; i++) {\
if(document.getElementById('' + i).type === 'checkbox'){params['p' + i] = document.getElementById('' + i).checked;}\
else{params['p' + i] = document.getElementById('' + i).value;}\
}\
xmlHttp.open('GET', '/set?s=' + s.selectedIndex + '&' + new URLSearchParams(params).toString(), true);\
xmlHttp.send(null);\
newData = false;\
}\
}\
</script>\
</html>";
