#ifndef ARP_H
#define ARP_H

#include <WiFi.h>
#include <ESP32Ping.h>
#include <lwip/err.h>
#include <lwip/ip4_addr.h>
#include <lwip/etharp.h>

/**
 * Classe ARP para conexão Wi-Fi e obtenção de MAC Address.
 */
class ARP {
public:
    ARP(const char* ssid, const char* password);
    void connectWiFi();
    bool testPing(IPAddress ip);
    String getMacAddress(IPAddress ip);

private:
    const char* wifiSSID;
    const char* wifiPassword;
};

#endif // ARP_H
