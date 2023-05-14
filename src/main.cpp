#include <string>

#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ezTime.h>

#include <PxMatrix.h>
#include <Ticker.h>

char ntp[] = "fr.pool.ntp.org";
const char HOSTNAME[] = "PixelTimes";
Timezone France;
bool shouldSaveWifiConfig = true;

// Pins definition
#define P_LAT 16
#define P_A 5
#define P_B 4
#define P_C 15
#define P_OE 2

#define matrix_width 32
#define matrix_height 16

// This defines the 'on' time of the display is us. The larger this number,
// the brighter the display. If too large the ESP will crash
uint8_t display_draw_time = 50; // 30-70 is usually fine

Ticker display_ticker;
PxMATRIX display(32, 16, P_LAT, P_OE, P_A, P_B, P_C);

// Some standard colors
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);

void display_updater()
{
  display.display(display_draw_time);
}

void display_update_enable(bool is_enable)
{
  if (is_enable)
  {
    display_ticker.attach(0.004, display_updater);
  }
  else
  {
    display_ticker.detach();
  }
}

void saveConfigCallback()
{
  Serial.println("Should save config");
  shouldSaveWifiConfig = true;
}

void configModeCallback(WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setupWiFi()
{
  wifi_station_set_hostname(const_cast<char *>(HOSTNAME));
  WiFiManager wifiManager;
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setMinimumSignalQuality();
  wifiManager.setTimeout(300);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  Serial.println("start wifi connection");

  if (!wifiManager.autoConnect(HOSTNAME))
  {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    ESP.restart();
    delay(5000);
  }

  Serial.println("wifi connected ok");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(115200);
  setupWiFi();
  waitForSync();
  France.setLocation("Europe/Paris");

  // Display setup
  display.begin(8);
  display.clearDisplay();
  display.setTextColor(myCYAN);
  display.setCursor(2, 0);
  display.print("Pixel");
  display.setTextColor(myMAGENTA);
  display.setCursor(2, 8);
  display.print("Time");
  display_update_enable(true);
  delay(3000);
}

void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB)
{
  uint16_t text_length = text.length();
  display.setTextWrap(false); // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(display.color565(colorR, colorG, colorB));

  // Asuming 5 pixel average character width
  for (int xpos = matrix_width; xpos > -(matrix_width + text_length * 5); xpos--)
  {
    display.setTextColor(display.color565(colorR, colorG, colorB));
    display.clearDisplay();
    display.setCursor(xpos, ypos);
    display.println(text);
    delay(scroll_delay);
    yield();

    // This might smooth the transition a bit if we go slow
    // display.setTextColor(display.color565(colorR/4,colorG/4,colorB/4));
    // display.setCursor(xpos-1,ypos);
    // display.println(text);

    delay(scroll_delay / 5);
    yield();
  }
}

void text(uint8_t xpos, uint8_t ypos, String text, uint16_t color)
{
  display.setTextWrap(false);
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(color);
  display.setCursor(xpos, ypos);
  display.println(text);
  yield();
}

void loop()
{
  char date[8] = "";
  sprintf(date, "%02d/%02d", France.day(), France.month());

  char time[8] = "";
  sprintf(time, "%02d:%02d", France.hour(), France.minute());

  display.clearDisplay();
  text(1, 1, time, myGREEN);
  text(1, 9, date, myBLUE);
  delay(1000);
}