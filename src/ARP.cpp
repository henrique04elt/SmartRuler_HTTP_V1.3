#include "ARP.h"
#include <ESP32Ping.h> // Certifique-se de que a biblioteca está instalada corretamente

ARP::ARP(const char* ssid, const char* password)
    : wifiSSID(ssid), wifiPassword(password) {}

void ARP::connectWiFi() {
    WiFi.begin(wifiSSID, wifiPassword);
    unsigned long startAttemptTime = millis();
    const unsigned long timeout = 30000; // Timeout de 30 segundos

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < timeout) {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConectado ao WiFi!");
    } else {
        Serial.println("\nFalha ao conectar ao WiFi. Verifique as credenciais.");
    }
}

bool ARP::testPing(IPAddress ip) {
    if (ip == INADDR_NONE) {
        Serial.println("Endereço IP inválido para teste de ping.");
        return false;
    }

    Serial.println("Testando conectividade com o endereço...");
    if (Ping.ping(ip)) {
        Serial.println("Ping bem-sucedido!");
        return true;
    } else {
        Serial.println("Falha no Ping.");
        return false;
    }
}

String ARP::getMacAddress(IPAddress ip) {
    char macAddress[18]; // Buffer para armazenar o MAC Address

    // Valida o IP
    if (ip == INADDR_NONE) {
        Serial.println("IP inválido fornecido.");
        return "";
    }

    // Verifica se a interface de rede padrão está configurada
    if (netif_default == NULL) {
        Serial.println("Interface de rede padrão (netif_default) não configurada.");
        return "";
    }

    ip4_addr_t addr;
    addr.addr = ip;

    struct eth_addr* eth_ret = NULL;
    const ip4_addr_t* ip_ret = NULL;

    // Consulta a tabela ARP
    err_t result = etharp_find_addr(netif_default, &addr, &eth_ret, &ip_ret);
    if (result == ERR_OK && eth_ret != NULL) {
        snprintf(macAddress, sizeof(macAddress), "%02X:%02X:%02X:%02X:%02X:%02X",
                 eth_ret->addr[0], eth_ret->addr[1], eth_ret->addr[2],
                 eth_ret->addr[3], eth_ret->addr[4], eth_ret->addr[5]);
        return String(macAddress);
    }

    // Diagnóstico detalhado do erro
    switch (result) {
        case ERR_MEM:
            Serial.println("Erro ao buscar endereço MAC: falta de memória.");
            break;
        case ERR_ARG:
            Serial.println("Erro ao buscar endereço MAC: argumento inválido.");
            break;
        default:
            Serial.println("Erro ao buscar endereço MAC: dispositivo não encontrado ou IP inválido.");
    }

    return ""; // Retorna vazio se o MAC Address não foi encontrado
}
