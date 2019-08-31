#include "ESP8266WiFi.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <SD.h>
#include "RTClib.h"

//rtc related setup
RTC_PCF8523 rtc;

//oled feather related stuff.
#define OLED_RESET 3
Adafruit_SSD1306 display(128, 32, &Wire, OLED_RESET);
#define BUTTON_A  0
#define BUTTON_B 16
#define BUTTON_C  2
// Button state counter
int stateA = 0;
int stateB = 0;
int stateC = 0;


//wifi scan related stuff
int netNum = 0;
int totalNet = 0;
typedef struct Single_Network {
  String SSID;
  uint8_t encryptionType;
  int32_t RSSI;
  uint8_t* BSSID;
  String BSSIDstr;
  int32_t channel;
  bool isHidden;
};
// Holds up to 50 networks... figured it should be enough for 1 scan.
Single_Network Network[50];

//datalogger related stuff
//Setup chip set for logger feather.
const int chipSelect = 15;

void showDate(DateTime now) {

    char daysOfTheWeek[7] = {'U', 'M', 'T', 'W', 'R', 'F', 'S'};

    Serial.print(now.year(), DEC);
    Serial.print('-');
    Serial.print(now.month(), DEC);
    Serial.print('-');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
    
}

void setup() {
  Serial.begin(9600); // Display to serial monitor
  Serial.println("Setup begins....");

  //RTC related setup :
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  Serial.println("Setting or getting the current time.");
  if (! rtc.initialized()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
  DateTime now = rtc.now();
  Serial.println(now.unixtime());
  showDate(now);

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
  Serial.println("card initialized.");



  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  delay(100);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  // Clear the buffer.5
  display.clearDisplay();
  delay(500);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("1:Scan");
  display.println("2:Continuous Scan ");
  display.println("3:Top Dog SSID");
  display.print("EPOCH:");
  display.println(now.unixtime());
  display.display();



  delay(1000);

}

//Main Scan for networks function
void scanNets(void) {
  netNum = 0;
  totalNet = 0;

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Scanning");
  display.display();

  WiFi.scanNetworksAsync(prinScanResult, 1);


  while (WiFi.scanComplete () < 0 ) {
    display.print(".");
    display.display();
    delay(500);
  }

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Scan done");
  Serial.println("scan done");
  display.println();
  display.println("Press C for result");
  display.display();
}



//Function to write the results to the files after a scan.
void prinScanResult(int networksFound)
{

  DateTime now = rtc.now();
  String uTime = String(now.unixtime());
  Serial.printf("%d network(s) found\n", networksFound);
  totalNet = networksFound;

  for (int i = 0; i < networksFound; i++)
  {
    File openFile = SD.open("open.txt", FILE_WRITE);
    File allFile = SD.open("all.txt", FILE_WRITE);

    Serial.printf("%d: %s, Ch:%d (%ddBm) %s %s\n", i + 1, WiFi.SSID(i).c_str(), WiFi.channel(i), WiFi.RSSI(i), WiFi.encryptionType(i) == ENC_TYPE_NONE ? "open" : "", WiFi.isHidden(i) ? "hidden" : "");

    if (WiFi.isHidden(i)) {

      Network[i].SSID = "* Hidden";

    } else {
      Network[i].SSID = WiFi.SSID(i).c_str();
    }

    Network[i].channel = WiFi.channel(i);
    Network[i].RSSI = WiFi.RSSI(i);
    Network[i].encryptionType = WiFi.encryptionType(i);
    Network[i].BSSID = WiFi.BSSID(i);
    Network[i].BSSIDstr = WiFi.BSSIDstr(i);


    // Write found SSIDs to the files on the root of the sd card.

    if (openFile) {


      // Log OPEN files.

      if (WiFi.encryptionType(i) == ENC_TYPE_NONE)
      {
        openFile.print(uTime);
        openFile.print(",");
        openFile.print(Network[i].SSID);
        openFile.print(",");
        openFile.print(Network[i].BSSIDstr);
        openFile.print(",");
        openFile.print( Network[i].channel);
        openFile.print("ch");
        openFile.print(",");
        openFile.print("OPEN");
        openFile.println();
        openFile.close();
      }
    }


    // Log EVERYTHING files.
    if (allFile) {
      allFile.print(uTime);
      allFile.print(",");
      allFile.print(Network[i].SSID);
      allFile.print(",");
      allFile.print(Network[i].BSSIDstr);
      allFile.print(",");
      allFile.print( Network[i].channel);
      allFile.print("ch");
      allFile.print(",");
      allFile.print(EncryptionType(Network[i].encryptionType));
      allFile.println();
      allFile.close();

    }


  }



}

//Convert EncryptionType Byte to human readable form.
String EncryptionType(uint8_t  encType)
{
  String encryptType;

  switch (encType)
  {
    case 4:
      {
        encryptType = "CCMP(WPA)";
        break;
      }
    case 5:
      {
        encryptType = "WEP";
        break;
      }
    case 2:
      {
        encryptType = "TKIP(WPA)";
        break;
      }
    case 7:
      {
        encryptType = "None";
        break;
      }
    case 8:
      {
        encryptType = "Auto";
        break;
      }
    default:
      {
        encryptType = "Other";
        break;
      }
  }

  return encryptType;
}




// Display the details of 1 network.

void showNetwork(void)
{

  Serial.print("display network:");
  Serial.print(netNum);
  Serial.print("/");
  Serial.println(totalNet);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(netNum + 1);
  display.print("/");
  display.print(totalNet);
  display.print(":");
  display.println(Network[netNum].SSID);
  display.println();
  display.print("Ch:");
  display.print(Network[netNum].channel);
  display.println("  (" + String(Network[netNum].RSSI) + " dBm)");
  display.print("Auth:" + EncryptionType(Network[netNum].encryptionType));
  display.display();

  //Increment or round robin the counter to that the scroll is round robined
  if (netNum == totalNet - 1) {
    netNum = 0;
  } else {
    netNum++;
  }


}

// Perform continuous scan
void contScan(void) {



  while (true) {

    int sN = 0;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Scanning cont");
    boolean writing = false;


    if (writing == false) {
      //Psuedo progress bar-ish.

      if (sN == 77) {
        display.clearDisplay();
        display.display();
        display.setCursor(0, 0);
        display.print("Scanning");
        sN = 0;
      }

      WiFi.scanNetworksAsync(prinScanResult, true);


      while (WiFi.scanComplete() < 0 ) {
        display.print(".");
        display.display();
        delay(500);
      }

    }

    if (WiFi.scanComplete() >= 0) {
      writing = true;
      display.clearDisplay();
      display.setCursor(0, 0);
      display.print("Writing results");
      delay (5000);
    }



    display.print(".");
    sN++;
    //  Serial.println(sN);
    display.display();

    delay(500);
  }


  WiFi.scanDelete();
}





void topDog(void) {

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print("Finding Top Dog SSID");
  display.display();


  int n = WiFi.scanNetworks(false, true);
  String ssid;
  uint8_t encryptionType;
  int32_t RSSI;
  uint8_t* BSSID;
  int32_t channel;
  bool isHidden;

  int topDog_rssi = -100;
  int topDog;

  for (int i = 0; i < n; i++)
  {


    WiFi.getNetworkInfo(i, ssid, encryptionType, RSSI, BSSID, channel, isHidden);

    Serial.printf("%d: %s, Ch:%d (%ddBm) %s %s\n", i + 1, ssid.c_str(), channel, RSSI, encryptionType == ENC_TYPE_NONE ? "open" : "", isHidden ? "hidden" : "");

    if (WiFi.RSSI(i) > topDog_rssi) {
      topDog = i ;
      topDog_rssi = WiFi.RSSI(i);
      if (WiFi.isHidden(i)) {

        Network[topDog].SSID = "* Hidden";

      } else {
        Network[topDog].SSID = WiFi.SSID(i).c_str();
      }


      Network[topDog].channel = WiFi.channel(i);
      Network[topDog].RSSI = WiFi.RSSI(i);
      Network[topDog].encryptionType = WiFi.encryptionType(i);
      Network[topDog].BSSID = WiFi.BSSID(i);

    }

  }

  display.clearDisplay();
  delay(500);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(Network[topDog].SSID);
  display.println();
  display.println("Ch:" + String(Network[topDog].channel) + "  " + "(" + String(Network[topDog].RSSI) + " dBM)" );
  display.println("Auth:" + EncryptionType(Network[topDog].encryptionType));
  display.print("MAC:");
  display.print((char*)(Network[topDog].BSSID));
  display.display();


}





void loop() {



  if (!digitalRead(BUTTON_A))  {

    stateA = 1;
    stateB = 1;
    stateC = 1;

    scanNets();

  }

  if (!digitalRead(BUTTON_B)) {

    if (stateB == 0) {
      contScan();
    }




  }

  if (!digitalRead(BUTTON_C)) {
    Serial.println("State C" + stateC);
    if (stateC == 1) {
      showNetwork();
    }

    if (stateC == 0) {
      topDog();
      Serial.println("top dogging");
    }

  }
  delay(200);

  yield();

  display.display();
}
