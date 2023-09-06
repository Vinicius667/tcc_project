
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

void send_text(String text, int x, int y,int size, boolean d);
void send_info(String axis, float acc, float incline);



// this is the Width and Height of Display which is 128 xy 32 
#define DISPLAY_HEIGHT 32
#define DISPLAY_WIDTH  128 


#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

void setup()   {                
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.display();
  delay(2000);
   // Clear the buffer.
  display.clearDisplay(); 
}

String axis = "X";
String value;
String text_display;
float acc = 0.0;
float incline = 0.0;


void loop() {
  value =  String(acc, 2);// using a float and the 
  send_info("X", acc, incline);
  acc +=0.011;
  incline += 0.021;
   delay(2000); 
}

void send_info(String axis, float acc, float incline){
  display.clearDisplay();
  value =  String(acc, 2);// using a float and the 
  send_text(String(axis + " " + value), 1, 1, 2, false);
  send_text(String(incline, 2), 95, 7, 1, false);
  display.display();
  }


/////send_text("Voltage:          ", 4, 3, 1, false);
void send_text(String text, int x_pos, int y_pos, int text_size, boolean display_text) {

  display.setTextSize(text_size);
  display.setTextColor(INVERSE);
  display.setCursor(x_pos,y_pos);
  display.println(text);
  if(display_text){
    display.display();
  }
  }
