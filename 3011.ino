#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define S0 2
#define S1 3
#define S2 4
#define S3 5
#define OUT 6
#define LED 7
#define RED 0
#define YELLOW 1
#define GREEN 2
#define CYAN 3
#define BLUE 4
#define MAGENTA 5
#define BUTTON 8


void setup() {
  pinMode(S0, OUTPUT);
  pinMode(S1, OUTPUT);
  pinMode(S2, OUTPUT);
  pinMode(S3, OUTPUT);
  pinMode(OUT, INPUT);
  pinMode(LED, OUTPUT);
  pinMode(BUTTON, INPUT_PULLUP);
  digitalWrite(LED, HIGH);
  digitalWrite(S0, HIGH);
  digitalWrite(S1, HIGH);
  Serial.begin(9600);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
}

struct color {
  float r;
  float g;
  float b;
};

struct color normalize(color c) {
  float length = sqrt(sq(c.r) + sq(c.g) + sq(c.b));
  return color {.r = c.r/length, .g = c.g/length, .b = c.b/length};
}

float color_distance(color c1, color c2) {
  c1 = normalize(c1);
  c2 = normalize(c2);
  return sqrt(sq(c2.r - c1.r) + sq(c2.g - c1.g) + sq(c2.b - c1.b));
}

String color_strings[6] = {
  "red = \nyellow + \nmagenta",
  "yellow = \nyellow + \nclear",
  "green = \nyellow + \ncyan",
  "cyan = \ncyan + \nclear",
  "blue = \ncyan + \nmagenta",
  "magenta = \nmagenta + \nclear"
};

color measured_color_values[6] {
  {.r = 0.31, .g = 0.73, .b = 0.61}, //red
  {.r = 0.37, .g = 0.48, .b = 0.8}, //yellow
  {.r = 0.66, .g = 0.46, .b = 0.58}, //green
  {.r = 0.77, .g = 0.48, .b = 0.43}, //cyan
  {.r = 0.74, .g = 0.56, .b = 0.37}, //blue
  {.r = 0.29, .g = 0.82, .b = 0.49}, //magenta
};

int closest_color(color input_color) {
  float min_dist = color_distance(input_color, measured_color_values[0]);
  float d;
  int min_color = 0;
  for (int i = 1; i < 6; i++) {
    d = color_distance(input_color, measured_color_values[i]);
    if (d < min_dist) {
      min_dist = d;
      min_color = i;
    }
  }
  return min_color;
}



void display_color(int calculated_color) {
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(color_strings[calculated_color]);
  display.display();      // Show initial text
}

void play_note(float freq, int millis) {
  int iterations = millis * 32 / (5 * freq) ;
  for (int j = 0; j < iterations; j++) {
    for (int i = 0; i < freq; i++) {
      analogWrite(A9, sin(i*6.28/freq) * 1024);
    }
  }
}

void play_tune(float scale) {
  play_note(64 * scale, 70);
  play_note(32 * scale, 70);
  play_note(16 * scale, 140);
  delay(50);
  play_note(32 * scale, 70);
  play_note(16 * scale, 70);
}

int temp;
int red;
int green;
int blue;
color input_color;
int calculated_color;

void loop() {
  for (;;) {
    for (int i = 0; i < 6; i++) {
      display_color(i);
      delay(1000);
    }
  }
  delay(300);
  
  // set S2 and S3 to use red filtered photodiodes
  // min: 45
  // max: 300
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  for (int i = 0; i < 100; i++ ) {
    temp += pulseIn(OUT, LOW);
  }
  red = temp/100;
  temp = 0;
  // Serial.print("R = ");
  // Serial.print(red);
  delay(100);

  
  // set S2 and S3 to use green filtered photodiodes
  // min: 160
  // max: 700
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  for (int i = 0; i < 100; i++) {
    temp += pulseIn(OUT, LOW);
  }
  green = temp/100;
  temp = 0;
  // Serial.print(" G = ");
  // Serial.print(green);
  delay(100);


  // set S2 and S3 to use blue filtered photodiodes
  // min: 120
  // max: 2000
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  for (int i = 0; i < 100; i++) {
    temp += pulseIn(OUT, LOW);
  }
  blue = temp/100;
  temp = 0;
  // Serial.print(" B = ");
  // Serial.println(blue);
  delay(100);

  input_color = color {.r = red, .g = green, .b = blue};
  color normalized = normalize(input_color);
  // Serial.print("R = ");
  // Serial.print(normalized.r);
  // Serial.print(" G = ");
  // Serial.print(normalized.g);
  // Serial.print(" B = ");
  // Serial.println(normalized.b);

  if (!digitalRead(BUTTON)) {
    calculated_color = closest_color(normalized);
    play_tune(2);
    play_tune(1);
    while (!digitalRead(BUTTON));
  }

  Serial.println(color_strings[calculated_color]);
  display_color(calculated_color);
}
