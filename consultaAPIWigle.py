"""
DEPRECADO EN EL DISPOSITIVO.

La consulta a Wigle ya no se realiza localmente en este repositorio.
Ahora el flujo esperado es:
1) El ESP8266 envía BSSIDs detectadas a un servidor remoto.
2) El servidor remoto consulta la API de Wigle.
3) El servidor remoto persiste y/o publica resultados.

Este archivo se mantiene únicamente como aviso de migración.
"""

print("Este script está deprecado. Mueve la lógica de Wigle al backend remoto.")
