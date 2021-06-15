#ifndef _MBED_HTTP_EXAMPLE_H_
#define _MBED_HTTP_EXAMPLE_H_

#include "mbed.h"
#include "NetworkInterface.h"

/**
 * Connect to the network using the default networking interface,
 * you can also swap this out with a driver for a different networking interface
 * if you use WiFi: see mbed_app.json for the credentials
 */
NetworkInterface *connect_to_default_network_interface() {
  printf("[NWKH] Connecting to network...\n");

  NetworkInterface* network = NetworkInterface::get_default_instance();

  if (!network) {
      printf("[NWKH] No network interface found, select an interface in 'network-helper.h'\n");
      return NULL;
  }

  nsapi_error_t result = network->connect();

  if (result != NSAPI_ERROR_OK) {
      printf("[NWKH] Failed to connect to network (%d)\n", result);
      return NULL;
  }

  SocketAddress a;
  printf("[NWKH] Connected to the network\n");
  result = network->get_ip_address(&a);
  printf("[NWKH] IP addr: %s\n", result==NSAPI_ERROR_OK ? a.get_ip_address() : "None");
  result = network->get_netmask(&a);
  printf("[NWKH] Netmask: %s\n", result==NSAPI_ERROR_OK ? a.get_ip_address() : "None");
  result = network->get_gateway(&a);
  printf("[NWKH] Gateway: %s\n", result==NSAPI_ERROR_OK ? a.get_ip_address() : "None");

  return network;
}

bool isEthernet() {
  return NetworkInterface::get_default_instance() == EthInterface::get_default_instance();
}

#endif // _MBED_HTTP_EXAMPLE_H_
