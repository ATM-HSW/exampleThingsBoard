// This sketch demonstrates connecting and sending telemetry 
// using ThingsBoard SDK
//
// Hardware:
//  - Arduino Uno
//  - ESP8266 connected to Arduino Uno

#include "mbed.h"
#include "mbed_error.h"
#include "mbed_fault_handler.h"
#include "network-helper.h"
#include "ThingsBoard.h"
#include "SparkFunHTU21D.h"

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
//Create an instance of the object
HTU21D myHTU21;

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

int main() {
  SocketAddress adr;
  nsapi_error_t result;
  bool bret;
  
  Telemetry data1[2] = {
    { "temperature",  0.0 },
    { "humidity",     0.0 },
  };
  Telemetry data2[3] = {
    { "boot",  0 },
    { "errorstatus",  0 },
    { "erroraddress", 0 }
  };
 

  printf("\n");
#ifdef MBED_MAJOR_VERSION
  int num = printf("Mbed OS version: %d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  puts("");
  PRINT_STR_REPEAT("-", num);
#endif

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
#else
    printf("App crash restart is not enabled\n");
#endif

  net = connect_to_default_network_interface();
  if (!net) {
    printf("Error! No network interface found.\n");
    thread_sleep_for(30000);
    system_reset();
    return -1;
  }

  result = net->gethostbyname(THINGSBOARD_HOST, &adr);
  if (result != NSAPI_ERROR_OK ) {
    printf("Error! net->gethostbyname returned: %d\n", result);
    thread_sleep_for(30000);
    system_reset();
  }
  adr.set_port(THINGSBOARD_PORT);
  
  tb.begin(TOKEN, THINGSBOARD_HOST, THINGSBOARD_PORT);
  
  myHTU21.begin(i2c);

  while(true) {
    socket = new TLSSocket();
  
    result = socket->open(net);
    if (result != NSAPI_ERROR_OK) {
      printf("Error! socket.open(net) returned: %d\n", result);
      thread_sleep_for(30000);
      system_reset();
    }

    result = socket->set_root_ca_cert(SSL_CA_PEM);
    if (result != NSAPI_ERROR_OK) {
      printf("Error! socket.set_root_ca_cert(ssl_ca_pem) returned: %d\n", result);
      thread_sleep_for(30000);
      system_reset();
    }
    
    socket->set_hostname(THINGSBOARD_HOST);

    result = socket->connect(adr);
    if (result != NSAPI_ERROR_OK) {
      printf("Error! socket.connect(adr) Failed (%d).\n", result);
      thread_sleep_for(30000);
      system_reset();
    }
    
    tb.setSocket(socket);

    printf("Sending data...\n");

    if(bBoot) {
      if(reboot_error_happened) {
        data2[0].setValue(1);
        data2[1].setValue((uint32_t)err_status);
        data2[2].setValue((uint32_t)error_address);
        bret = tb.sendTelemetry(data2, 3);
        if(!bret) printf("error sending telemetry\n");
      } else {
        data2[0].setValue(1);
        data2[1].setValue(0);
        data2[2].setValue(0);
        bret = tb.sendTelemetry(data2, 3);
        if(!bret) printf("error sending telemetry\n");
      }
      reboot_error_happened = false;
      bBoot = false;
    }

    // Uploads new telemetry to ThingsBoard using http
    data1[0].setValue(myHTU21.readTemperature());
    data1[1].setValue(myHTU21.readHumidity());
    bret = tb.sendTelemetry(data1, 2);
    if(!bret) printf("error sending telemetry\n");
    
    socket->~TLSSocket();
    
    thread_sleep_for(15000);
  }
}
