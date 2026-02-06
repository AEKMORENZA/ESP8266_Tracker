#include "ESP8266WiFi.h"
#include "ESP8266HTTPClient.h"
#include "FS.h"

// --- Configuración del dispositivo ---
const char* DEVICE_ID = "esp8266-tracker-01";
const char* PREFERRED_SSID = "MACARENA";        // Si se define, se intentará primero esta red.
const char* PREFERRED_PASSWORD = "DEADBEEF";    // Password de PREFERRED_SSID (vacío si es abierta).
const char* SERVER_URL = "http://tu-servidor-remoto.example/api/bssids";

const int MAX_BSSIDS_CACHE = 20;
const int SCAN_RSSI_THRESHOLD = -70;
const int CONNECT_RSSI_THRESHOLD = -85;
const unsigned long LOOP_DELAY_MS = 5000;

String lastBSSIDs[MAX_BSSIDS_CACHE] = {};
int cont = 0;

void leerSerie();
void procesarComando(String cmd);
void imprimirAyuda();
void borrarArchivo(const String& filePath);
void imprimirArchivo(const String& filePath);
void registrarBSSIDSiNueva(const String& bssid, File& file);
bool hayBssidsPendientes(const String& filePath);
String construirPayloadJSON(const String& filePath);
void enviarPendientesAServidor();
int buscarRedPreferida(int numNetworks);
int buscarRedAbierta(int numNetworks);
bool conectarAIndiceRed(int networkIndex);

void leerSerie() {
  String cmd = "";
  while (Serial.available() > 0) {
    char c = Serial.read();
    if ((c == '\n') || (c == '\r')) {
      if (cmd.length() > 0) {
        procesarComando(cmd);
      }
      cmd = "";
    } else {
      cmd += c;
    }
  }
}

void procesarComando(String cmd) {
  Serial.print("Comando recibido: ");
  Serial.println(cmd);

  if (cmd.equalsIgnoreCase("d")) {
    Serial.println("Imprimir archivo");
    imprimirArchivo("/BSSIDs.txt");
  } else if (cmd.equalsIgnoreCase("r")) {
    Serial.println("Borrar archivo");
    borrarArchivo("/BSSIDs.txt");
  } else if (cmd.equalsIgnoreCase("h")) {
    imprimirAyuda();
  } else {
    Serial.println("Comando no válido. Introduce H para ver la ayuda.");
  }
}

void imprimirAyuda() {
  Serial.println("Comandos disponibles:");
  Serial.println("D = dump del archivo /BSSIDs.txt por puerto serie");
  Serial.println("R = borrar contenido de /BSSIDs.txt");
  Serial.println("H = mostrar esta ayuda");
}

void borrarArchivo(const String& filePath) {
  File f = SPIFFS.open(filePath, "w");
  if (!f) {
    Serial.println("No se pudo abrir/crear el archivo para borrarlo.");
    return;
  }
  f.close();
  Serial.println("Archivo borrado (contenido reiniciado).");
}

void imprimirArchivo(const String& filePath) {
  File f = SPIFFS.open(filePath, "r");
  if (!f) {
    Serial.println("El archivo no existe.");
    return;
  }

  Serial.println("Contenidos:");
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      Serial.println(line);
    }
  }
  f.close();
}

void registrarBSSIDSiNueva(const String& bssid, File& file) {
  bool yaVista = false;
  for (int i = 0; i < MAX_BSSIDS_CACHE; i++) {
    if (bssid == lastBSSIDs[i]) {
      yaVista = true;
      break;
    }
  }

  if (!yaVista) {
    lastBSSIDs[cont] = bssid;
    cont++;
    if (cont >= MAX_BSSIDS_CACHE) {
      cont = 0;
    }

    file.println(bssid);
    Serial.print("BSSID guardada: ");
    Serial.println(bssid);
  }
}

bool hayBssidsPendientes(const String& filePath) {
  File f = SPIFFS.open(filePath, "r");
  if (!f) {
    return false;
  }

  bool pending = false;
  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() > 0) {
      pending = true;
      break;
    }
  }
  f.close();
  return pending;
}

String construirPayloadJSON(const String& filePath) {
  File f = SPIFFS.open(filePath, "r");
  if (!f) {
    return "";
  }

  String payload = "{\"device_id\":\"" + String(DEVICE_ID) + "\",\"bssids\":[";
  bool first = true;

  while (f.available()) {
    String line = f.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
      continue;
    }

    if (!first) {
      payload += ",";
    }
    payload += "\"" + line + "\"";
    first = false;
  }

  payload += "]}";
  f.close();
  return payload;
}

int buscarRedPreferida(int numNetworks) {
  if (String(PREFERRED_SSID).length() == 0) {
    return -1;
  }

  for (int i = 0; i < numNetworks; i++) {
    if (WiFi.SSID(i) == String(PREFERRED_SSID) && WiFi.RSSI(i) >= CONNECT_RSSI_THRESHOLD) {
      return i;
    }
  }
  return -1;
}

int buscarRedAbierta(int numNetworks) {
  int bestIndex = -1;
  int bestRssi = -1000;

  for (int i = 0; i < numNetworks; i++) {
    if (WiFi.encryptionType(i) == ENC_TYPE_NONE && WiFi.RSSI(i) >= CONNECT_RSSI_THRESHOLD) {
      if (WiFi.RSSI(i) > bestRssi) {
        bestRssi = WiFi.RSSI(i);
        bestIndex = i;
      }
    }
  }

  return bestIndex;
}

bool conectarAIndiceRed(int networkIndex) {
  if (networkIndex < 0) {
    return false;
  }

  String ssid = WiFi.SSID(networkIndex);
  bool preferred = (ssid == String(PREFERRED_SSID) && String(PREFERRED_SSID).length() > 0);

  Serial.print("Conectando a SSID: ");
  Serial.println(ssid);

  if (preferred) {
    WiFi.begin(PREFERRED_SSID, PREFERRED_PASSWORD);
  } else {
    WiFi.begin(ssid.c_str());
  }

  const unsigned long timeoutMs = 15000;
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
    delay(300);
    Serial.print('.');
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Conectado. IP: ");
    Serial.println(WiFi.localIP());
    return true;
  }

  Serial.println("No se pudo conectar a la red seleccionada.");
  return false;
}

void enviarPendientesAServidor() {
  const String filePath = "/BSSIDs.txt";
  if (!hayBssidsPendientes(filePath)) {
    return;
  }

  Serial.println("Hay BSSIDs pendientes de envío.");

  int numNetworks = WiFi.scanNetworks();
  if (numNetworks <= 0) {
    Serial.println("No hay redes para intentar subida.");
    return;
  }

  int idx = buscarRedPreferida(numNetworks);
  if (idx < 0) {
    idx = buscarRedAbierta(numNetworks);
  }

  if (idx < 0) {
    Serial.println("No se encontró red válida (preferida o abierta). Reintento en próximo ciclo.");
    return;
  }

  if (!conectarAIndiceRed(idx)) {
    WiFi.disconnect();
    return;
  }

  String payload = construirPayloadJSON(filePath);
  if (payload.length() == 0) {
    Serial.println("No se pudo construir payload.");
    WiFi.disconnect();
    return;
  }

  WiFiClient client;
  HTTPClient http;
  http.begin(client, SERVER_URL);
  http.addHeader("Content-Type", "application/json");

  Serial.println("Enviando payload al servidor remoto...");
  int httpCode = http.POST(payload);
  Serial.print("HTTP code: ");
  Serial.println(httpCode);

  if (httpCode > 0 && httpCode < 300) {
    Serial.println("Envío OK. Limpiando pendientes locales.");
    borrarArchivo(filePath);
  } else {
    Serial.println("Fallo en envío. Se conservan pendientes para próximo intento.");
  }

  http.end();
  WiFi.disconnect();
}

void setup() {
  Serial.begin(115200);
  bool result = SPIFFS.begin();
  Serial.print("SPIFFS opened: ");
  Serial.println(result ? "true" : "false");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println("Setup done");
  imprimirAyuda();
}

void loop() {
  if (Serial) {
    Serial.print("\n>>> ");
    leerSerie();
  }

  Serial.println("Scan start");
  int numNetworks = WiFi.scanNetworks();
  Serial.println("Scan done");

  if (numNetworks <= 0) {
    Serial.println("No networks found");
  } else {
    Serial.print(numNetworks);
    Serial.println(" networks found");

    File f = SPIFFS.open("/BSSIDs.txt", "a");
    if (!f) {
      Serial.println("No se pudo abrir /BSSIDs.txt para append");
    } else {
      for (int i = 0; i < numNetworks; i++) {
        if (WiFi.RSSI(i) > SCAN_RSSI_THRESHOLD) {
          String bssid = WiFi.BSSIDstr(i);
          Serial.print("BSSID = ");
          Serial.print(bssid);
          Serial.print(" RSSI = ");
          Serial.println(WiFi.RSSI(i));
          registrarBSSIDSiNueva(bssid, f);
          delay(10);
          yield();
        }
      }
      f.close();
    }
  }

  enviarPendientesAServidor();

  delay(LOOP_DELAY_MS);
  yield();
}
