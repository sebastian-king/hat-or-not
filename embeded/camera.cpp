#include <WiFi.h>
#include <Wire.h>
#include <ArduCAM.h>
#include <SPI.h>
#include "memorysaver.h"
#include <WiFiUdp.h>

//#if !(defined ESP32)
//#error Please select the ArduCAM ESP32 UNO board in the Tools/Board
//#endif

#if !(defined (OV2640_MINI_2MP)||defined (OV5640_MINI_5MP_PLUS) || defined (OV5642_MINI_5MP_PLUS) \
    || defined (OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_BIT_ROTATION_FIXED) \
    ||(defined (ARDUCAM_SHIELD_V2) && (defined (OV2640_CAM) || defined (OV5640_CAM) || defined (OV5642_CAM))))
#error Please select the hardware platform and camera module in the ../libraries/ArduCAM/memorysaver.h file
#endif

// Chip select pin for SLI
const int CS = 5; // Enable pin
const int CAM_POWER_ON = 4; // note: D10 on ArduCAM ESP32 is PIN 4
const int LED = 13; // LED pin for debugging

// Select the appropriate camera module
#if defined (OV2640_MINI_2MP) || defined (OV2640_CAM)
ArduCAM myCAM(OV2640, CS);
#elif defined (OV5640_MINI_5MP_PLUS) || defined (OV5640_CAM)
ArduCAM myCAM(OV5640, CS);
#elif defined (OV5642_MINI_5MP_PLUS) || defined (OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_BIT_ROTATION_FIXED) ||(defined (OV5642_CAM))
ArduCAM myCAM(OV5642, CS);
#endif

WiFiUDP Udp;

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

bool ws_connected = true;

void enableDebugLED() {
    digitalWrite(LED, HIGH);
    Serial.println(F("Turning debug LED to ON"));
}

void disableDebugLED() {
    digitalWrite(LED, LOW);
    Serial.println(F("Turning debug LED to OFF"));
}

void start_capture() {
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
}

void sendData(uint8_t * payload, size_t len) {
  //Udp.beginPacket("untrobotics.com", 2115);
  Udp.beginPacket("http://hat-or-not.helpfulseb.com/ingest/outfit", 8080);
  Udp.write(payload, len);
  Udp.endPacket();
}

void serverStream() {
  // jpeg byte format references: https://docs.fileformat.com/image/jpeg/
  delay(1000);

  int total_time = 0;
  int capture_total_time = 0;
  int send_total_time = 0;
  int spi_total_time = 0;
  total_time = millis();
  capture_total_time = total_time;
  
  myCAM.clear_fifo_flag();
  myCAM.start_capture();
  Serial.println(F("CAM Capturing"));

  while (!myCAM.get_bit(ARDUCHIP_TRIG, CAP_DONE_MASK)) {
    //Serial.printf("Awaiting capture completion: %d\r", millis() - capture_total_time);
  }
  capture_total_time = millis() - capture_total_time;
  Serial.print(F("capture total_time used (in miliseconds):"));
  Serial.println(capture_total_time, DEC);

  Serial.println(F("CAM Capture Done."));
  
  send_total_time = millis();
  
  uint32_t len = myCAM.read_fifo_length();
  
  if (len >= MAX_FIFO_SIZE) // 8M
  {
    Serial.println(F("Over size."));
  }
  
  if (len == 0 ) // 0 kb
  {
    Serial.println(F("Size is 0."));
  }
  
  myCAM.CS_LOW();
  myCAM.set_fifo_burst();
  
  //if (!ws_connected) return;
  
  i = 0;
  while ( len-- ) {
    temp_last = temp;
    int spi_time = millis();
    temp = SPI.transfer(0x00);
    spi_total_time += millis() - spi_time;
    
    //Read JPEG data from FIFO
    if ( (temp == 0xD9) && (temp_last == 0xFF) ) { // end of image //If find the end, finish by breaking out of the while loop
      buffer[i++] = temp; // save the last 0XD9
      
      // Write the remaining bytes in the buffer
      //if (!ws_connected) break;
      //client.write(&buffer[0], i);
      int data_time = millis();
      //webSocket.sendBIN(&buffer[0], i);
      sendData(&buffer[0], i);
      Serial.printf("Data time (in milliseconds): %d\n", millis() - data_time);
      
      is_header = false;
      i = 0;
      myCAM.CS_HIGH();
      break;
    }
    
    if (is_header == true) { // middle of image
      // Write image data to buffer if not full
      if (i < bufferSize) {
        buffer[i++] = temp;
      } else {
        // Write bufferSize bytes image data to socket
        //if (!ws_connected) break;
        
        //client.write(&buffer[0], bufferSize);
        //webSocket.sendBIN(&buffer[0], bufferSize);
        sendData(&buffer[0], bufferSize);
        
        i = 0;
        buffer[i++] = temp;
      }
    }
    else if ((temp == 0xD8) & (temp_last == 0xFF)) // start of image
    {
      is_header = true;
      buffer[i++] = temp_last;
      buffer[i++] = temp;
    }
  }
  
  send_total_time = millis() - send_total_time;
  Serial.print(F("send total_time used (in miliseconds):"));
  Serial.println(send_total_time, DEC);
  Serial.println(F("CAM send Done."));
  total_time = millis() - total_time;
  
  Serial.print(F("SPI time (in miliseconds):"));
  Serial.println(spi_total_time, DEC);
  
  Serial.print(F("Frame time (in miliseconds):"));
  Serial.println(total_time, DEC);
}

void setup() {
  uint8_t vid, pid;
  uint8_t temp;
  
  // Set the CS, EN and LED pins as an outputs
  pinMode(CS, OUTPUT);
  pinMode(CAM_POWER_ON , OUTPUT);
  pinMode(LED, OUTPUT);

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

  // Initialise & detect the camera module
  #if defined (OV2640_MINI_2MP) || defined (OV2640_CAM)
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
  #elif defined (OV5640_MINI_5MP_PLUS) || defined (OV5640_CAM)
    // Check if the camera module type is OV5640
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5640_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x40)) {
      Serial.println(F("Can't find OV5640 module!"));
    } else {
      Serial.println(F("OV5640 detected."));
      enableDebugLED();
    }
  #elif defined (OV5642_MINI_5MP_PLUS) || defined (OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_BIT_ROTATION_FIXED) ||(defined (OV5642_CAM))
    // Check if the camera module type is OV5642
    myCAM.wrSensorReg16_8(0xff, 0x01);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_HIGH, &vid);
    myCAM.rdSensorReg16_8(OV5642_CHIPID_LOW, &pid);
    if ((vid != 0x56) || (pid != 0x42)) {
      Serial.println(F("Can't find OV5642 module!"));
    } else {
      Serial.println(F("OV5642 detected."));
      enableDebugLED();
    }
  #endif
  
  // Change to JPEG capture mode and initialize the camera module
  myCAM.set_format(JPEG);
  myCAM.InitCAM();
  
  #if defined (OV2640_MINI_2MP) || defined (OV2640_CAM)
    myCAM.OV2640_set_JPEG_size(OV2640_640x480);
  #elif defined (OV5640_MINI_5MP_PLUS) || defined (OV5640_CAM)
    myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
    myCAM.OV5640_set_JPEG_size(OV2640_640x480);
  #elif defined (OV5642_MINI_5MP_PLUS) || defined (OV5642_MINI_5MP) || defined (OV5642_MINI_5MP_BIT_ROTATION_FIXED) ||(defined (OV5642_CAM))
    myCAM.write_reg(ARDUCHIP_TIM, VSYNC_LEVEL_MASK);   //VSYNC is active HIGH
    myCAM.OV5640_set_JPEG_size(OV2640_640x480);
  #endif

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
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println(F("."));
  }
  Serial.println(F("WiFi connected"));
  Serial.println("");
  Serial.println(WiFi.localIP());

  delay(10000);
}

void loop() {
  Serial.printf("Total PSRAM: %d\n", ESP.getPsramSize());
  if (ws_connected) {
    serverStream();
  }
  delay(1000);
}
