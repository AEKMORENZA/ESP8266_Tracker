import json
import time
import requests
from lxml import etree
from pykml.factory import KML_ElementMaker as KML


def cargar_bssids_desde_json(filepath):
    with open(filepath, "r", encoding="utf-8", errors="ignore") as fp:
        data = json.load(fp)
    bssids = set()
    for item in data.get("bssids", []):
        bssid = item.get("bssid")
        if isinstance(bssid, str) and len(bssid) >= 3 and bssid[2] == ":":
            bssids.add(bssid)
    return bssids


def main():
    filepath = "BSSIDs.txt"
    lista = cargar_bssids_desde_json(filepath)

    # TBW abrir el documento KML
    fld = KML.Folder()

    for element in lista:
        url = "https://api.wigle.net/api/v2/network/detail?netid="
        bssid = element.strip()
        print("BSSID: " + bssid)
        encoded_bssid = bssid.replace(":", "%3A")
        url += encoded_bssid

        r = requests.get(url, auth=("API_NAME", "API_TOKEN"))
        print(r)
        if not r.ok:
            print("HTTP error: {} {}".format(r.status_code, r.text.strip()))
            print()
            continue
        try:
            j = r.json()
        except ValueError:
            print("Response was not JSON.")
            print(r.text.strip())
            print()
            continue

        if j.get("success") is True:
            lat_long = str(j["results"][0]["trilong"]) + "," + str(j["results"][0]["trilat"])
            print(lat_long)
            # TBW escribir como placemark al documento KML
            pm = KML.Placemark(KML.name(str(element)), KML.Point(KML.coordinates(lat_long)))
            fld.append(pm)
        else:
            print("error:" + str(j.get("message")))

        print()

    timestr = time.strftime("%Y%m%d")
    filename = timestr + ".kml"
    # TBW cerrar el documento KML y escribir su contenido a un fichero .kml
    kml_bytes = etree.tostring(etree.ElementTree(fld), pretty_print=True)
    print(kml_bytes.decode("utf-8", errors="ignore"))
    with open(filename, "wb") as k:
        k.write(kml_bytes)


if __name__ == "__main__":
    main()
				
