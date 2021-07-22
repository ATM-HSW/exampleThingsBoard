// This sketch demonstrates connecting and sending telemetry 
// using ThingsBoard SDK
//
// Hardware:
//  - Arduino Uno
//  - ESP8266 connected to Arduino Uno

#include <string>

#include "mbed.h"
#include "mbed_error.h"
#include "mbed_fault_handler.h"
#include "network-helper.h"
#include "ThingsBoard.h"
#include "SparkFunHTU21D.h"
#include "SparkFun_SGP40_Arduino_Library.h"
#include "Adafruit_TSL2591.h"
#include "MHZ19.h"

#include "ResetReason.h"

#include "mbed_crash_data_offsets.h"

// wait WRITEINTERAL seconds between each writing to ThingsBoard - sensor reading is done once per second, mainly neccessary for SGP40
// wait WRITEINTERAL_STARTUP seconds before writing the first time - ca. 2min needed by SGP40 to get first correct values
#define WRITEINTERAL 15
#define WRITEINTERAL_STARTUP 120

#define PRINT_STR_REPEAT(str, times) \
{ \
  for (int i = 0; i < times; ++i) \
    printf("%s", str); \
  puts(""); \
}

// See https://thingsboard.io/docs/getting-started-guides/helloworld/ 
// to understand how to obtain an access token
#include "secrets.h"

//secrets.h has to contain the following lines with e.g.
//#define TOKEN "..."
//#define THINGSBOARD_HOST "thingsboard.cloud"
//#define THINGSBOARD_PORT 443
// certificate can be optained with: openssl s_client -showcerts -connect thingsboard.cloud:443
// ISRG Root X1 - this certificate can be used with thingsboard.cloud
// SHA1 Fingerprint=93:3C:6D:DE:E9:5C:9C:41:A4:0F:9F:50:49:3D:82:BE:03:AD:87:BF
//const char SSL_CA_PEM[] = "-----BEGIN CERTIFICATE-----\n"
//"MIIFYDCCBEigAwIBAgIQQAF3ITfU6UK47naqPGQKtzANBgkqhkiG9w0BAQsFADA/\n"
//"MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT\n"
//"DkRTVCBSb290IENBIFgzMB4XDTIxMDEyMDE5MTQwM1oXDTI0MDkzMDE4MTQwM1ow\n"
//"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
//"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwggIiMA0GCSqGSIb3DQEB\n"
//"AQUAA4ICDwAwggIKAoICAQCt6CRz9BQ385ueK1coHIe+3LffOJCMbjzmV6B493XC\n"
//"ov71am72AE8o295ohmxEk7axY/0UEmu/H9LqMZshftEzPLpI9d1537O4/xLxIZpL\n"
//"wYqGcWlKZmZsj348cL+tKSIG8+TA5oCu4kuPt5l+lAOf00eXfJlII1PoOK5PCm+D\n"
//"LtFJV4yAdLbaL9A4jXsDcCEbdfIwPPqPrt3aY6vrFk/CjhFLfs8L6P+1dy70sntK\n"
//"4EwSJQxwjQMpoOFTJOwT2e4ZvxCzSow/iaNhUd6shweU9GNx7C7ib1uYgeGJXDR5\n"
//"bHbvO5BieebbpJovJsXQEOEO3tkQjhb7t/eo98flAgeYjzYIlefiN5YNNnWe+w5y\n"
//"sR2bvAP5SQXYgd0FtCrWQemsAXaVCg/Y39W9Eh81LygXbNKYwagJZHduRze6zqxZ\n"
//"Xmidf3LWicUGQSk+WT7dJvUkyRGnWqNMQB9GoZm1pzpRboY7nn1ypxIFeFntPlF4\n"
//"FQsDj43QLwWyPntKHEtzBRL8xurgUBN8Q5N0s8p0544fAQjQMNRbcTa0B7rBMDBc\n"
//"SLeCO5imfWCKoqMpgsy6vYMEG6KDA0Gh1gXxG8K28Kh8hjtGqEgqiNx2mna/H2ql\n"
//"PRmP6zjzZN7IKw0KKP/32+IVQtQi0Cdd4Xn+GOdwiK1O5tmLOsbdJ1Fu/7xk9TND\n"
//"TwIDAQABo4IBRjCCAUIwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYw\n"
//"SwYIKwYBBQUHAQEEPzA9MDsGCCsGAQUFBzAChi9odHRwOi8vYXBwcy5pZGVudHJ1\n"
//"c3QuY29tL3Jvb3RzL2RzdHJvb3RjYXgzLnA3YzAfBgNVHSMEGDAWgBTEp7Gkeyxx\n"
//"+tvhS5B1/8QVYIWJEDBUBgNVHSAETTBLMAgGBmeBDAECATA/BgsrBgEEAYLfEwEB\n"
//"ATAwMC4GCCsGAQUFBwIBFiJodHRwOi8vY3BzLnJvb3QteDEubGV0c2VuY3J5cHQu\n"
//"b3JnMDwGA1UdHwQ1MDMwMaAvoC2GK2h0dHA6Ly9jcmwuaWRlbnRydXN0LmNvbS9E\n"
//"U1RST09UQ0FYM0NSTC5jcmwwHQYDVR0OBBYEFHm0WeZ7tuXkAXOACIjIGlj26Ztu\n"
//"MA0GCSqGSIb3DQEBCwUAA4IBAQAKcwBslm7/DlLQrt2M51oGrS+o44+/yQoDFVDC\n"
//"5WxCu2+b9LRPwkSICHXM6webFGJueN7sJ7o5XPWioW5WlHAQU7G75K/QosMrAdSW\n"
//"9MUgNTP52GE24HGNtLi1qoJFlcDyqSMo59ahy2cI2qBDLKobkx/J3vWraV0T9VuG\n"
//"WCLKTVXkcGdtwlfFRjlBz4pYg1htmf5X6DYO8A4jqv2Il9DjXA6USbW1FzXSLr9O\n"
//"he8Y4IWS6wY7bCkjCWDcRQJMEhg76fsO3txE+FiYruq9RUWhiF1myv4Q6W+CyBFC\n"
//"Dfvp7OOGAN6dEOM4+qR9sdjoSYKEBpsr6GtPAQw4dy753ec5\n"
//"-----END CERTIFICATE-----\n";

NetworkInterface *net;
TLSSocket *socket;

// Initialize ThingsBoard instance
ThingsBoardHttps tb;

I2C i2c(I2C_SDA , I2C_SCL );

HTU21D myHTU21;                        // create an instance of the HTU21 class
SGP40 mySGP40;                         // create an instance of the SGP40 class
Adafruit_TSL2591 myTSL2591;            // create an instance of the TSL2591 class
MHZ19 myMHZ19;                         // create an instance of the MHZ19 class
BufferedSerial rserMHZ19(PC_12, PD_2); // create serial port instance for MH-Z19
DigitalIn myBtn(BUTTON1);             // Calibration user button
int btnvalue; 

#if MBED_CONF_PLATFORM_CRASH_CAPTURE_ENABLED
mbed_error_status_t err_status;
uint32_t error_address;
int32_t error_reboot_count;
mbed_fault_context_t mbed_fault_context;
static bool reboot_error_happened = false;

// Application callback function for reporting error context during boot up.
void mbed_error_reboot_callback(mbed_error_ctx *error_context) {
  reboot_error_happened = true;
  err_status = error_context->error_status;
  error_address = error_context->error_address;
  error_reboot_count = error_context->error_reboot_count;
  if(error_reboot_count>3)
    mbed_reset_reboot_count();
  mbed_reset_reboot_error_info();
}
#endif

bool bBoot = true;

/**************************************************************************/
/*
    Configures the gain and integration time for the TSL2591
*/
/**************************************************************************/
void configureSensorTSL2591(void) {
  // You can change the gain on the fly, to adapt to brighter/dimmer light situations
  //tsl.setGain(TSL2591_GAIN_LOW);    // 1x gain (bright light)
  myTSL2591.setGain(TSL2591_GAIN_MED);      // 25x gain
  //tsl.setGain(TSL2591_GAIN_HIGH);   // 428x gain
  
  // Changing the integration time gives you a longer time over which to sense light
  // longer timelines are slower, but are good in very low light situtations!
  //tsl.setTiming(TSL2591_INTEGRATIONTIME_100MS);  // shortest integration time (bright light)
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_200MS);
  myTSL2591.setTiming(TSL2591_INTEGRATIONTIME_300MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_400MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_500MS);
  // tsl.setTiming(TSL2591_INTEGRATIONTIME_600MS);  // longest integration time (dim light)

  /* Display the gain and integration time for reference sake */  
  printf("TSL2591 gain:         ");
  tsl2591Gain_t gain = myTSL2591.getGain();
  switch(gain) {
    case TSL2591_GAIN_LOW:
      printf("1x (Low)\n");
      break;
    case TSL2591_GAIN_MED:
      printf("25x (Medium)\n");
      break;
    case TSL2591_GAIN_HIGH:
      printf("428x (High)\n");
      break;
    case TSL2591_GAIN_MAX:
      printf("9876x (Max)\n");
      break;
  }
}

/**************************************************************************/
/*
    read IR and Full Spectrum at once and convert to lux
*/
/**************************************************************************/
float readLux(void) {
  // More advanced data read example. Read 32 bits with top 16 bits IR, bottom 16 bits full spectrum
  // That way you can do whatever math and comparisons you want!
  uint32_t lum = myTSL2591.getFullLuminosity();
  uint16_t ir, full;
  ir = lum >> 16;
  full = lum & 0xFFFF;
  return myTSL2591.calculateLux(full, ir);
}

std::string reset_reason_to_string(const reset_reason_t reason) {
  switch (reason) {
    case RESET_REASON_POWER_ON:
      return "Power On";
    case RESET_REASON_PIN_RESET:
      return "Hardware Pin";
    case RESET_REASON_SOFTWARE:
      return "Software Reset";
    case RESET_REASON_WATCHDOG:
      return "Watchdog";
    default:
      return "Other Reason";
  }
}

int main() {
  SocketAddress adr;
  nsapi_error_t result;
  bool bret;
  bool bTSL2591 = true;
  bool bSGP40 = true;
  float fTemp, fHum, fLux;
  int32_t iVOC, iCO2;
  int writeCounter = 0;
  // wait WRITEINTERAL_STARTUP seconds before writing the first time - ca. 2min needed by SGP40 to get first correct values
  int writeinterval = WRITEINTERAL_STARTUP; 
  
  Telemetry data1[5] = {
    { "temperature",  0.0 },
    { "humidity",     0.0 },
    { "VOCindex",     0 },
    { "CO2",          0 },
    { "light",        0.0 },
  };
  Telemetry data2[3] = {
    { "reason",  0 },
    { "errorstatus",  0 },
    { "erroraddress", 0 }
  };
 

  printf("\n");
#ifdef MBED_MAJOR_VERSION
  int num = printf("Mbed OS version: %d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  puts("");
  PRINT_STR_REPEAT("-", num);
#endif
  
  const reset_reason_t reason = ResetReason::get();
  if(reason==RESET_REASON_PIN_RESET || reason==RESET_REASON_POWER_ON)
    mbed_reset_reboot_error_info();

#if MBED_CONF_PLATFORM_CRASH_CAPTURE_ENABLED
  printf("App crash restart is enabled\n");
  if (reboot_error_happened) {
    if (err_status < 0) {
      printf("2nd run: Retrieve the fault context using mbed_get_reboot_fault_context\n");
      printf("    Error status: 0x%X\n", (uint32_t)err_status);
      printf("    Address     : 0x%X\n", (uint32_t)error_address);
      printf("    Reboot count: 0x%X\n", (uint32_t)error_reboot_count);
    }
  }
  printf("last error status %d\n", MBED_CRASH_DATA.error.context.error_status);
  printf("Last system reset reason: %s\r\n", reset_reason_to_string(reason).c_str());
  printf("\n");
#else
    printf("App crash restart is not enabled\n");
#endif
  

  net = connect_to_default_network_interface();
  if (!net) {
    printf("Error! No network interface found.\n");
    MBED_CRASH_DATA.error.context.error_status = 0;
    MBED_CRASH_DATA.error.context.error_address = 1;
    thread_sleep_for(30000);
    system_reset();
    return -1;
  }

  result = net->gethostbyname(THINGSBOARD_HOST, &adr);
  if (result != NSAPI_ERROR_OK ) {
    printf("Error! net->gethostbyname returned: %d\n", result);
    MBED_CRASH_DATA.error.context.error_status = result;
    MBED_CRASH_DATA.error.context.error_address = 2;
    thread_sleep_for(30000);
    system_reset();
  }
  adr.set_port(THINGSBOARD_PORT);
  
  tb.begin(TOKEN, THINGSBOARD_HOST, THINGSBOARD_PORT);
  
  printf("\n");
  
  //Initialize temperature and humidity sensor HTU21
  myHTU21.begin(i2c);

  //Initialize light sensor TSL2591
  if(!myTSL2591.begin(i2c)) {
    printf("No sensor found ... check your wiring?\n");
    bTSL2591 = false;
  } else {
    /* Configure the sensor */
    configureSensorTSL2591();
  }

  //Initialize air quality sensor SGP40
  mySGP40.enableDebugging(false);
  if (mySGP40.begin(i2c, reason==RESET_REASON_PIN_RESET || reason==RESET_REASON_POWER_ON) == false) {
    printf("SGP40 not detected. Check connections. Freezing...\n");
    bSGP40 = false;
  }
  printf("SGP40 selftest: %s\n", (reason==RESET_REASON_PIN_RESET || reason==RESET_REASON_POWER_ON)?"yes":"no");

  //Initialize CO2 sensor MHZ19
  rserMHZ19.set_baud(9600);                                  // (Uno example) device to MH-Z19 serial start   
  myMHZ19.begin(rserMHZ19);                                // *Serial(Stream) refence must be passed to library begin(). 
  myMHZ19.printCommunication(false, false);

  myMHZ19.autoCalibration(false);                               // Turn auto calibration ON (OFF autoCalibration(false))

  char myVersion[4];          
  myMHZ19.getVersion(myVersion);
  printf("MHZ19 firmware Version: %c.%c.%c.%c\n", myVersion[0], myVersion[1], myVersion[2], myVersion[3]);
  printf("MHZ19 range: %d\n", myMHZ19.getRange());   
  //printf("MHZ19 background CO2: %d\n", myMHZ19.getBackgroundCO2());
  //printf("MHZ19 temperature Cal: %d\n", myMHZ19.getTempAdjustment());
  //printf("MHZ19 ABC Status: %s\n", myMHZ19.getABC() ? "ON" : "OFF");

  btnvalue = myBtn.read();

  printf("\n");

  while(true) {
    fTemp = myHTU21.readTemperature();
    fHum = myHTU21.readHumidity();
    int32_t iVOC = bSGP40?mySGP40.getVOCindex(fHum, fTemp):0;
    int32_t iCO2 = myMHZ19.getCO2();
    float fLux = bTSL2591?readLux():0.0f;

    if(writeCounter >= writeinterval) {
      socket = new TLSSocket();
    
      result = socket->open(net);
      if (result != NSAPI_ERROR_OK) {
        printf("Error! socket.open(net) returned: %d\n", result);
        MBED_CRASH_DATA.error.context.error_status = result;
        MBED_CRASH_DATA.error.context.error_address = 3;
        thread_sleep_for(30000);
        system_reset();
      }

      result = socket->set_root_ca_cert(SSL_CA_PEM);
      if (result != NSAPI_ERROR_OK) {
        printf("Error! socket.set_root_ca_cert(ssl_ca_pem) returned: %d\n", result);
        MBED_CRASH_DATA.error.context.error_status = result;
        MBED_CRASH_DATA.error.context.error_address = 4;
        thread_sleep_for(30000);
        system_reset();
      }
      
      socket->set_hostname(THINGSBOARD_HOST);

      result = socket->connect(adr);
      if (result != NSAPI_ERROR_OK) {
        printf("Error! socket.connect(adr) Failed (%d).\n", result);
        MBED_CRASH_DATA.error.context.error_status = result;
        MBED_CRASH_DATA.error.context.error_address = 5;
        thread_sleep_for(30000);
        system_reset();
      }
      
      tb.setSocket(socket);

      printf("Sending data...\n");

      if(bBoot) {
        printf("Sending error status ...\n");
        if(reboot_error_happened) {
          data2[0].setValue(reason);
          data2[1].setValue((uint32_t)err_status);
          data2[2].setValue((uint32_t)error_address);
          bret = tb.sendTelemetry(data2, 3);
          if(!bret) printf("error sending telemetry\n");
        } else {
          data2[0].setValue(reason);
          data2[1].setValue(reason==RESET_REASON_SOFTWARE?MBED_CRASH_DATA.error.context.error_status:0);
          data2[2].setValue(reason==RESET_REASON_SOFTWARE?MBED_CRASH_DATA.error.context.error_address:0);
          bret = tb.sendTelemetry(data2, 3);
          if(!bret) printf("error sending telemetry\n");
        }
        reboot_error_happened = false;
        bBoot = false;
        mbed_reset_reboot_error_info();
      }

      // Uploads new telemetry to ThingsBoard using http
      data1[0].setValue(fTemp);
      data1[1].setValue(fHum);
      data1[2].setValue(iVOC);
      data1[3].setValue(iCO2);
      data1[4].setValue(fLux);
      bret = tb.sendTelemetry(data1, 5);
      if(!bret) printf("error sending telemetry\n");
      
      socket->~TLSSocket();
      writeCounter = 0;
      
      // start writing to Thingsboard every WRITEINTERAL second
      writeinterval = WRITEINTERAL;
    }

    if(myBtn.read() == true && btnvalue == false) {
      myMHZ19.calibrate();
      printf("start calibration\n");
    }
    btnvalue = myBtn.read();
    
    writeCounter++;
    thread_sleep_for(1000);
  }
}
