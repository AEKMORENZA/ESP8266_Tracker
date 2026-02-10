# ESP8266_Tracker
Dispositivo de seguimiento con ESP8266 para deteccion de BSSIDs y envio a servidor remoto.

## Flujo actual
1. El ESP8266 escanea redes WiFi cercanas y almacena BSSIDs detectadas localmente en SPIFFS (`/BSSIDs.txt`).
2. Cuando hay conectividad, el dispositivo intenta conectarse a:
   - Un SSID preferido configurado previamente, o
   - Cualquier red WiFi abierta disponible.
3. Si la red es abierta, se verifica conectividad a Internet abriendo un socket TCP a `8.8.8.8:53`. Si falla, se desconecta y reintenta en el siguiente ciclo.
4. Se intenta sincronizar hora por NTP de forma no bloqueante.
5. El dispositivo envia las BSSIDs pendientes por HTTPS POST a un servidor remoto.
6. El servidor remoto es quien debe consultar la API de Wigle y gestionar el resto del pipeline.

## Formato de datos
- Archivo local `BSSIDs.txt`: `bssid,ts,ntp`
  - `ts` es `epoch` en segundos si NTP esta disponible, si no es `millis()`.
  - `ntp` es `1` si el timestamp proviene de NTP, `0` si es `millis()`.
- Payload JSON:
  - `device_id` y `bssids` con objetos `{ "bssid": "...", "ts": 123, "ntp": 1 }`.

## Comportamiento de duplicados
- En una misma ejecucion, una BSSID solo se registra una vez (cache en RAM).

## Configuracion principal del firmware
Editar en `ESP8266_Scanner/ESP8266_Scanner.ino`:
- `PREFERRED_SSID` y `PREFERRED_PASSWORD`
- `SERVER_URL`
- `DEVICE_ID`
- Umbrales `SCAN_RSSI_THRESHOLD` y `CONNECT_RSSI_THRESHOLD`
- Parametros NTP: `NTP_RETRY_INTERVAL_MS`, `NTP_WAIT_MS` y servidores en `configTime(...)`

## Comandos serie
- `D`: dump del archivo `/BSSIDs.txt`
- `R`: limpia `/BSSIDs.txt`
- `H`: ayuda

## Tareas pendientes
- Definir autenticacion robusta con el servidor remoto (token/HMAC/TLS).
- Anadir reintentos con backoff y cola persistente mas avanzada.
- Definir formato final de payload y contrato de API del backend.

