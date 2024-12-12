/*********************************** Smart Ruler FREERTOS  **********************************
* Autor:   Henrique Rosa                                                                     *
* Projeto: Smart Ruler                                                                       *
* Versão:  1.3                                                                               *
* Data: 09/12/2024                                                                           *
* Descrição: Versão simplista para comunicação com Broker- IOT HUB                           *
*********************************************************************************************/

#include <WiFi.h>
#include <HTTPClient.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//*************************** Configurações de Wi-Fi *************************************//
const char* WIFI_SSID = "SMP";
const char* WIFI_PASSWORD = "12345678";

//*************************** Configurações do Broker HTTP *******************************//
const char* BROKER_URL = "https://boe-php.eletromidia.com.br/device/regua/pub";
const char* EVENT_TYPE = "keep_alive"; //
const char* DETAIL = "regua-inteligente"; //

//************************ Configurações do NTP ******************************************//
const char* NTP_SERVER = "pool.ntp.org";
const long GMT_OFFSET_SEC = -3 * 3600; //
const int DAYLIGHT_OFFSET_SEC = 0;    //

//********************** Intervalo de Keep Alive *****************************************//
const unsigned long KEEP_ALIVE_INTERVAL = 30000; // Intervalo de 30 segundos
unsigned long lastKeepAlive = 0; // Controle de tempo para envio do último Keep Alive

//**************************** Objetos Globais *******************************************//
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_SERVER, GMT_OFFSET_SEC, 60000);

//*************************** Funções Prototipadas **************************************//
void setupWiFi();
String obterTimestamp();
void enviarKeepAlive();

void setup() {
  Serial.begin(115200);

  // Conecta ao Wi-Fi
  setupWiFi();

  // Inicializa o cliente NTP
  timeClient.begin();
}

void loop() {
  unsigned long currentMillis = millis();

  // Atualiza o cliente NTP periodicamente
  if (!timeClient.update()) {
    timeClient.forceUpdate();
  }

  // Envia o Keep Alive se o intervalo foi atingido
  if (currentMillis - lastKeepAlive >= KEEP_ALIVE_INTERVAL) {
    enviarKeepAlive();
    lastKeepAlive = currentMillis;
  }
}

//************************** Função para Conexão Wi-Fi ***********************************//
void setupWiFi() {
  Serial.print("Conectando ao WiFi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi conectado!");
}

//*************************** Função para Obter Timestamp ********************************//
String obterTimestamp() {
  timeClient.update();
  time_t rawTime = timeClient.getEpochTime();
  struct tm* timeInfo = gmtime(&rawTime); // Converte para UTC
  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", timeInfo); //
  return String(buffer);
}

//************************ Função para Enviar Keep Alive *********************************//
void enviarKeepAlive() {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    // Inicializa a requisição HTTP
    http.begin(BROKER_URL);
    http.addHeader("Content-Type", "application/json");

    // Obtém o MAC Address
    String macAddress = WiFi.macAddress();

    // Cria o JSON de payload
    String payload = "{";
    payload += "\"csrf\": \"" + String("csrfehchato") + "\",";  // Adicione seu token CSRF se necessário
    payload += "\"event\": \"" + String(EVENT_TYPE) + "\",";
    payload += "\"detail\": \"" + String(DETAIL) + "\",";
    payload += "\"timestamp\": \"" + obterTimestamp() + "\",";
    payload += "\"mac_address\": \"" + macAddress + "\"";  // Adiciona o MAC Address
    payload += "}";

    Serial.print("Payload: ");
    Serial.println(payload);

    // Envia a requisição POST
    int httpResponseCode = http.POST(payload);

    // Trata a resposta do servidor
    if (httpResponseCode == 200) {
      // Lê o corpo da resposta
      String response = http.getString();
      Serial.println("Keep Alive enviado com sucesso!");
      Serial.println("Resposta do servidor: " + response);
    } else {
      Serial.println("Falha ao enviar Keep Alive. Código: " + String(httpResponseCode) +
                     ". Erro: " + http.errorToString(httpResponseCode));
      String errorResponse = http.getString();
      Serial.println("Mensagem de erro do servidor: " + errorResponse);
    }

    // Finaliza a conexão HTTP
    http.end();
  } else {
    Serial.println("WiFi desconectado. Não foi possível enviar o Keep Alive.");
  }
}
