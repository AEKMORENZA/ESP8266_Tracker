import requests
import json
import time
from lxml import etree
from pykml.factory import KML_ElementMaker as KML

filepath = 'BSSIDs.txt'
lista = set([])
with open(filepath) as fp:
	
	line = fp.readline()
	
	while line:
			
		line = fp.readline()
		try:
			if line[2] == ':':
				BSSID = line
				lista.add(BSSID)
		except:
			pass
	fp.close()

	#TBW abrir el documento KML
	fld = KML.Folder()
 	
	for element in lista:
		URL = 'https://api.wigle.net/api/v2/network/detail?netid='
		BSSID = element.strip('\n')
		print "BSSID: " + BSSID
		encodedBSSID = BSSID.replace(':','%3A')
		URL += encodedBSSID
				
		r = requests.get(URL, auth=('Wigle API Name','Wigle API Token'))
		print r
		j = r.json()
				
		if (j['success'] == True):
			latLong = str(j['results'][0]['trilong']) + ',' + str(j['results'][0]['trilat']) 
			print latLong
			#TBW escribir como placemark al documento KML
			pm = KML.Placemark(KML.name(str(element)),KML.Point(KML.coordinates(latLong)))
			fld.append(pm)
		else:
			print "error:" + str(j['message'])
				
		print "\n"
	
	timestr = time.strftime("%Y%m%d")
	filename = timestr + ".kml"
	#TBW cerrar el documento KML y escribir su contenido a un fichero .kml
	print etree.tostring(etree.ElementTree(fld),pretty_print=True)	
	kmlFile = etree.tostring(etree.ElementTree(fld),pretty_print=True)
	k = open(filename,"w")
	k.write(kmlFile)
				
