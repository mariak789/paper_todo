#include <M5GFX.h>
#include <epdiy.h>

static M5GFX display;

void setup() {
  Serial.begin(115200);
  Serial.println("Starting Hello World on PaperS3...");

  display.init();
  display.clearDisplay();

  // Configure text settings
  display.setTextSize(2);
  display.setTextColor(TFT_BLACK, TFT_WHITE);
  display.setCursor(10, 10);

  display.println("Hello, World!");

  // For e-paper displays, update the screen to show the changes
  display.display();
  display.waitDisplay();
}

void loop() {

}