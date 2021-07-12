// This sketch demonstrates connecting and sending telemetry and attributes
// it batch using ThingsBoard SDK
//
// Hardware:
//  - Arduino Uno
//  - ESP8266 connected to Arduino Uno

#include "mbed.h"
#include "network-helper.h"
#include "ThingsBoard.h"

#define PRINT_STR_REPEAT(str, times) \
{ \
  for (int i = 0; i < times; ++i) \
    printf("%s", str); \
  puts(""); \
}

#define THINGSBOARD_HOST "192.168.178.84"
#define THINGSBOARD_PORT 8888

// See https://thingsboard.io/docs/getting-started-guides/helloworld/ 
// to understand how to obtain an access token
#include "secrets.h"
// secrets.h has to contain one line with the token value e.g.
// #define TOKEN "..."

NetworkInterface *net;
TCPSocket socket;

// Initialize ThingsBoard instance
ThingsBoardHttp tb;

int main() {
  SocketAddress adr;
  nsapi_error_t result;
  bool bret;

  const int data_items = 2;
  Telemetry data[data_items] = {
    { "temperature", 42.2 },
    { "humidity",    80 },
  };

  const int attribute_items = 2;
  Attribute attributes[attribute_items] = {
    { "device_type",  "sensor" },
    { "active",       true     },
  };


  printf("\n");
#ifdef MBED_MAJOR_VERSION
  int num = printf("Mbed OS version: %d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);
  puts("");
  PRINT_STR_REPEAT("-", num);
#endif

  net = connect_to_default_network_interface();
  if (!net) {
    printf("Error! No network inteface found.\n");
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
  
  tb.begin(&socket, TOKEN, THINGSBOARD_HOST, THINGSBOARD_PORT);

  while(true) {
    result = socket.open(net);
    if (result != NSAPI_ERROR_OK) {
      printf("Error! socket.open(net) returned: %d\n", result);
      thread_sleep_for(30000);
      system_reset();
    }

    result = socket.connect(adr);
    if (result != NSAPI_ERROR_OK) {
      printf("Error! socket.connect(adr) Failed (%d).\n", result);
      thread_sleep_for(30000);
      system_reset();
    }

    printf("Sending data...\n");

    printf("Sending telemetry data...\n");
    // Uploads new telemetry to ThingsBoard using MQTT. 
    // See https://thingsboard.io/docs/reference/mqtt-api/#telemetry-upload-api 
    // for more details
    bret = tb.sendTelemetry(data, data_items);
    if(!bret) printf("error sending telemetry\n");

    printf("Sending attribute data...\n");
    // Publish attribute update to ThingsBoard using MQTT. 
    // See https://thingsboard.io/docs/reference/mqtt-api/#publish-attribute-update-to-the-server 
    // for more details
    bret = tb.sendAttributes(attributes, attribute_items);
    if(!bret) printf("error sending attribute\n");

    socket.close();
    
    thread_sleep_for(15000);
  }
}
