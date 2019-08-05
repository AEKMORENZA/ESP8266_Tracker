#include "ESP8266WiFi.h"
#include "FS.h"

String lastBSSIDs[20] = {};
bool checker = 0;
int cont = 0;


void leerSerie(){
  String cmd = "";
  while(Serial.available() > 0){
    char c = Serial.read();
    if ((c == '\n')||(c == '\r')){
      Serial.print(cmd);
      cmd = "";
      procesarComando(cmd);      
    }else cmd += c;
  }
}

void procesarComando(String cmd){
  //TBW
}


void setup()
{
  
  Serial.begin(115200);
  //TBW
  //Si hay puerto serie:
  //Imprimir prompt de linea de comandos
  //Imprimir comandos disponibles
  //D = 'dump' volcado del archivo /BSSIDs.txt al puerto serie
  //R = 'remove' elimina el archivo del sistema de ficheros del microcontrolador
  //H = 'help' pinta lista de comandos disponibles
  
  bool result = SPIFFS.begin();
  Serial.println("SPIFFS opened: " + result);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  File f = SPIFFS.open("/BSSIDs.txt", "r");

  delay(100);

  if (!f) {
    Serial.println("File doesn't exist yet. Creating it");

    // open the file in write mode
    File f = SPIFFS.open("/BSSIDs.txt", "a");
    if (!f) {
      Serial.println("file creation failed");
    }
  } else {
    Serial.println("Acabamos de abrir el archivo");
    // we could open the file
    while (f.available()) {
      //Lets read line by line from the file
      String line = f.readStringUntil('\n');
      Serial.println(line);
    }

  }
  f.close();

  Serial.println("Setup done");
}

void loop()
{
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
        Serial.print("     BSSID = ");
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
          Serial.print("Tamaño de array lastBSSIDs = ");
          Serial.println(arraySize);
          Serial.println(lastBSSIDs[arraySize]);
          cont ++;
        }
        if (cont == 20){
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
