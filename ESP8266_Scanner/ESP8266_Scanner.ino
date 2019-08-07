#include "ESP8266WiFi.h"
#include "FS.h"

String lastBSSIDs[20] = {};
bool checker = 0;
int cont = 0;


void leerSerie() {
  String cmd = "";

  while (Serial.available() > 0) {
    char c = Serial.read();
    if ((c == '\n') || (c == '\r')) {
      procesarComando(cmd);
      cmd = "";
    } else cmd += c;
  }
}

void procesarComando(String cmd) {
  Serial.print(cmd);
  if (cmd == "\n") {
  }
  else if (cmd.equalsIgnoreCase("d")) {
    Serial.println("imprimir archivo");
    imprimirArchivo("/BSSIDs.txt");
  }
  else if (cmd.equalsIgnoreCase("r")) {
    Serial.println("borrar archivo");
    borrarArchivo("/BSSIDs.txt");
  }
  else if (cmd.equalsIgnoreCase("h")) {
    Serial.println("imprimir menu de ayuda");
    imprimirAyuda();
  }
  else Serial.println("Comando no valido, introduce H para ver una lista de comandos diponibles");
}

void imprimirAyuda() {
  Serial.println("Comandos disponibles: ");
  Serial.println("D = 'dump' volcado del archivo /BSSIDs.txt al puerto serie");
  Serial.println("R = 'remove' elimina el archivo del sistema de ficheros del microcontrolador");
  Serial.println("H = 'help' pinta lista de comandos disponibles");
}


void borrarArchivo(String filePath) {
  File f = SPIFFS.open(filePath, "w");

  delay(100);

  if (!f) {
    Serial.println("File doesn't exist yet. Creating it");

    // open the file in write mode
    File f = SPIFFS.open(filePath, "w");
    if (!f) {
      Serial.println("file creation failed");
    }
  }
  f.close();
}

void imprimirArchivo(String filePath) {
  File f = SPIFFS.open(filePath, "r");

  delay(100);

  if (!f) {
    Serial.println("File doesn't exist");
  } else {
    Serial.println("Contenidos:");
    // we could open the file
    while (f.available()) {
      //Lets read line by line from the file
      String line = f.readStringUntil('\n');
      Serial.println(line);
    }
  }
  f.close();
}

void setup()
{
  Serial.begin(115200);
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println("Setup done");
}

void loop()
{

  if (Serial) {
    Serial.print("\n>>> ");
    leerSerie();
  }
  int arraySize = sizeof(lastBSSIDs) / sizeof(lastBSSIDs[0]);
  Serial.println(arraySize);
  Serial.println(checker);

  Serial.println("Scan start");

  int numNetworks = WiFi.scanNetworks();
  Serial.println("Scan done");

  if (numNetworks == 0)
  {
    Serial.println("No networks found");
  }
  else
  {
    Serial.print(numNetworks);
    Serial.println(" networks found");

    File f = SPIFFS.open("/BSSIDs.txt", "a");

    for (int i = 0; i < numNetworks; i++)
    {
      if (WiFi.RSSI(i) > -70) {
        String BSSID_str = WiFi.BSSIDstr(i);
        Serial.print("BSSID = ");
        Serial.println(BSSID_str);
        checker = 0;
        for (int x = 0; x < arraySize ; x++) {
          Serial.println(lastBSSIDs[x]);
          if ( BSSID_str == lastBSSIDs[x]) {
            checker = 1;
          }
        }
        if (checker == 0) {
          lastBSSIDs[cont] = BSSID_str;
          f.println(BSSID_str);
          f.close();

          cont ++;
        }
        if (cont == 20) {
          cont = 0;
        }
        Serial.print("     RSSI = ");
        Serial.println(WiFi.RSSI(i));
        delay(10);
        yield();
      }
    }
  }
  Serial.println("");

  delay(5000);
  yield();
}
