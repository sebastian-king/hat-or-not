#include "WiFi.h"
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"

//Http POST
#include "HTTPClient.h"

#if !(defined ESP32)
#error Please select the ArduCAM ESP32 UNO board in the Tools/Board
#endif

#if !(defined OV2640_MINI_2MP_PLUS)
  #error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif

// Chip select pin for SLI
const int CS = 5; // Enable pin
const int CAM_POWER_ON = 4; // note: D10 on ArduCAM ESP32 is PIN 4
const int LED = 13; // LED pin for debugging
const int INPUT_BTN = 32; // Input from from button push

// Select the appropriate camera module
#if defined (OV2640_MINI_2MP_PLUS)
  ArduCAM myCAM( OV2640, CS );
#else
  ArduCAM myCAM( OV5642, CS );
#endif

// Set WiFi details
const char *ssid = "HackDFW"; // Put your SSID here
const char *password = "sayyestodallas"; // Put your PASSWORD here

// Init global variables
static const size_t bufferSize = 57344;
static uint8_t buffer[bufferSize] = {0xFF};
uint8_t temp = 0, temp_last = 0;
int i = 0;
bool is_header = false;

static const size_t spiBufferSize = 1024;
static uint8_t spiBuffer[spiBufferSize] = {0xFF};

void enableDebugLED() {
    digitalWrite(LED, HIGH);
    Serial.println(F("Turning debug LED to ON"));
}

void disableDebugLED() {
    digitalWrite(LED, LOW);
    Serial.println(F("Turning debug LED to OFF"));
}

void start_capture() {
  myCAM.flush_fifo();
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
}

void sendData(uint8_t * payload, size_t len) {
  Serial.print(F("About to send data to server, len: "));
  Serial.println(len);
  HTTPClient http;

  http.begin("http://hat-or-not.helpfulseb.com:8080/ingest/outfit"); 
  http.addHeader("Content-Type", "Content-Type: image/jpeg"); 

  int httpResponseCode = http.POST(payload, len);
  
  if (httpResponseCode >= 0){
    String response = http.getString();  //Get the response to the request
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);           //Print request answer
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
  http.end();
}

void camCapture(ArduCAM myCAM) {
  start_capture();
  Serial.println(F("Started capture, waiting for capturing to be done"));
  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK));
  Serial.println(F("Photo captured, probably"));
  
  uint32_t len  = myCAM.read_fifo_length();
  if (len >= MAX_FIFO_SIZE) //8M
  {
    Serial.println(F("Over size."));
  }
  if (len == 0 ) //0 kb
  {
    Serial.println(F("Size is 0."));
  }
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  
  i = 0;
  while ( len-- )
  {
    temp_last = temp;
    temp =  SPI.transfer(0x00);
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) //If find the end ,break while,
    {
      buffer[i++] = temp;  //save the last  0XD9
      // Write the remaining bytes in the buffer
      // WARNING: client.write(&buffer[0], i);
      is_header = false;
      i = 0;
      myCAM.CS_HIGH();
      break;
    }
    if (is_header == true)
    {
      // Write image data to buffer if not full
      if (i < bufferSize)
        buffer[i++] = temp;
      else
      {
        // Write bufferSize bytes image data to file
        // WARNING: client.write(&buffer[0], bufferSize);
        i = 0;
        buffer[i++] = temp;
      }
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF))
    {
      is_header = true;
      buffer[i++] = temp_last;
      buffer[i++] = temp;
    }
  }
  
  sendData(&buffer[0], len);
}

void setup() {  
  uint8_t vid, pid;
  uint8_t temp;
  
  // Set the CS, EN and LED pins as an outputs
  pinMode(CS, OUTPUT);
  pinMode(CAM_POWER_ON , OUTPUT);
  pinMode(LED, OUTPUT);

  // Set the INPUT_BTN pin as an input
  pinMode(INPUT_BTN, INPUT);

  // Enable the camera
  digitalWrite(CAM_POWER_ON, HIGH);
  
  Wire.begin();

  Serial.begin(115200);
  Serial.printf("ArduCAM Start!");

  // Init SPI
  SPI.begin();
  SPI.setFrequency(8000000); //8MHz (standard is 4MHz)

  // Check if the ArduCAM SPI bus is functional
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55) {
    Serial.println(temp, HEX);
    Serial.println(F("Failed to communicate with the ArduCAM over SPI on first attempt."));
    while(1);
  }

  // Check if the ArduCAM SPI bus is functional (again, yes we need to test this twice)
  myCAM.write_reg(ARDUCHIP_TEST1, 0x55);
  temp = myCAM.read_reg(ARDUCHIP_TEST1);
  if (temp != 0x55) {
    Serial.println(F("Failed to communicate with the ArduCAM over SPI on second attempt."));
    while(1);
  }

  // Check if the camera module type is OV2640
  myCAM.wrSensorReg8_8(0xff, 0x01);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_HIGH, &vid);
  myCAM.rdSensorReg8_8(OV2640_CHIPID_LOW, &pid);
  if ((vid != 0x26 ) && (( pid != 0x41 ) || ( pid != 0x42 ))) {
    Serial.println(F("Can't find OV2640 module!"));
  } else {
    Serial.println(F("OV2640 detected."));
    enableDebugLED();
  }
  
  // Change to JPEG capture mode and initialize the camera module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  
  myCAM.OV2640_set_JPEG_size(OV2640_640x480);

  myCAM.clear_fifo_flag();
  
  if (!strcmp(ssid, "SSID")) {
    Serial.println(F("Please set your WiFi SSID"));
    while(1);
  }
  if (!strcmp(password, "PASSWORD")) {
    Serial.println(F("Please set your WiFi PASSWORD"));
    while(1);
  }
  
  // Connect to WiFi network
  Serial.printf("Connecting to %s", ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  //wifiMulti.addAP(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(750);
    Serial.println(F("."));
  }
  Serial.print(F("WiFi connected: "));
  Serial.println(WiFi.localIP());
}

void loop() {
  if (digitalRead(INPUT_BTN)) {
    Serial.println(F("Capturing and sending..."));
    camCapture(myCAM);
    delay(500);
  }
  delay(1000);
}
