# ESP8266_Tracker
Dispositivo de seguimiento con ESP8266 para detección de BSSIDs y envío a servidor remoto.

## Flujo actual
1. El ESP8266 escanea redes WiFi cercanas y almacena BSSIDs detectadas localmente en SPIFFS (`/BSSIDs.txt`).
2. Cuando hay conectividad, el dispositivo intenta conectarse a:
   - Un SSID preferido configurado previamente, o
   - Cualquier red WiFi abierta disponible.
3. El dispositivo envía las BSSIDs pendientes por HTTP POST a un servidor remoto.
4. El servidor remoto es quien debe consultar la API de Wigle y gestionar el resto del pipeline.

## Configuración principal del firmware
Editar en `ESP8266_Scanner/ESP8266_Scanner.ino`:
- `PREFERRED_SSID` y `PREFERRED_PASSWORD`
- `SERVER_URL`
- `DEVICE_ID`
- Umbrales `SCAN_RSSI_THRESHOLD` y `CONNECT_RSSI_THRESHOLD`

## Comandos serie
- `D`: dump del archivo `/BSSIDs.txt`
- `R`: limpia `/BSSIDs.txt`
- `H`: ayuda

## Tareas pendientes
- Definir autenticación robusta con el servidor remoto (token/HMAC/TLS).
- Añadir reintentos con backoff y cola persistente más avanzada.
- Definir formato final de payload y contrato de API del backend.
