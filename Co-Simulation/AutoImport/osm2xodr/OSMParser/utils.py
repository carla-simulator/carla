__all__ = ['crs_4326', 'crs_25832', 'transformer', 'giveHeading', 'checkDistance', 'drehen', 'rotateToXAxis',
           'getXYPositionFromLineLength', 'convertTopoMap', 'setHeights', 'referenceLat', 'referenceLon',
           'maximumheight', 'minimumheight', 'giveHeight', 'giveMaxMinLongLat', 'convertLongitudeLatitude', 'distance',
           'schnittpunkt', 'createVirtualLastPointForJunctionRoads', 'getPositiveHeading', 'getDeltaHdg', 'giveReferences']

#Cell
import numpy as np
from pyproj import CRS, Transformer
from PIL import Image
from osmread import parse_file, Way, Node


crs_4326  = CRS.from_epsg(4326) # epsg 4326 is wgs84
#crs_25832  = CRS.from_epsg(25832) # epsg 25832 is etrs89
crs_3395 = CRS.from_epsg(3395) #epsg 3395 is pseudo Mercator for tiled Maps (Streetview, GoogleMaps etc.)
global transformer
transformer = None#Transformer.from_crs(crs_4326, crs_3395)


#Cell
def giveHeading(x1,y1,x2,y2):
        assert not (x1==x2 and y1==y2), "Can't give heading without a direction"
        x = [x1,x2]
        y = [y1,y2]
        x_arr=np.array(x)-x[0]
        y_arr=np.array(y)-y[0]
        #rotate to initial approximately 0
        #umrechnen in polarkoordinaten des ersten abstandes
        if x_arr[1] > 0:
                    phi = np.arctan(y_arr[1]/x_arr[1])
        elif x_arr[1] == 0:
                    if y_arr[1] > 0:
                            phi = np.pi/2
                    else:
                            phi = -np.pi/2
        else:
                    if y_arr[1] >= 0:
                            phi = np.arctan(y_arr[1]/x_arr[1])+np.pi
                    else:
                            phi = np.arctan(y_arr[1]/x_arr[1])-np.pi
        return getPositiveHeading(phi)


#Cell
def checkDistance(x,y,x2,y2):
    x_m = (x+x2)/2.0
    y_m = (y+y2)/2.0
    r = ((x_m-x)**2+(y_m-y)**2)**0.5
    [_,_],phi,_,_ = rotateToXAxis([x_m,x2],[y_m,y2])
    return x_m, y_m, r, phi


#Cell
def drehen(x,y,phi, drehpunkt = [0,0], offset = False):
        x = np.array(x)-drehpunkt[0]
        y = np.array(y)-drehpunkt[1]
        dmat =  np.array([[np.cos(phi),-np.sin(phi)],[np.sin(phi),np.cos(phi)]])
        x_new = []
        y_new = []
        for i in range(len(x)):
            points = np.matmul(dmat,np.hstack((x[i],y[i])))
            x_new.append(points[0])
            y_new.append(points[1])
        x_new = np.array(x_new)
        y_new = np.array(y_new)
        if offset:
            x_new = x_new+drehpunkt[0]
            y_new = y_new+drehpunkt[1]
        return x_new, y_new

#Cell
def rotateToXAxis(x,y):
        x_arr=np.array(x)-x[0]
        y_arr=np.array(y)-y[0]
        #rotate to initial approximately 0
        #umrechnen in polarkoordinaten des ersten abstandes
        if x_arr[1] > 0:
                    phi = np.arctan(y_arr[1]/x_arr[1])
        elif x_arr[1] == 0:
                    if y_arr[1] > 0:
                            phi = np.pi/2
                    else:
                            phi = -np.pi/2
        else:
                    if y_arr[1] >= 0:
                            phi = np.arctan(y_arr[1]/x_arr[1])+np.pi
                    else:
                            phi = np.arctan(y_arr[1]/x_arr[1])-np.pi
        phi = -phi
        x_arr,y_arr = drehen(x_arr,y_arr,phi)
        return [x[0],y[0]],phi,x_arr,y_arr

#Cell

def getXYPositionFromLineLength(xstart, ystart, heading, length):
    xend = xstart + np.cos(heading) * length
    yend = ystart + np.sin(heading) * length
    return xend, yend


#Cell
global referenceLat
global referenceLon
referenceLat = None
referenceLon = None


global topoParameter
global topomap
topomap = None
topoParameter = None
def convertTopoMap(topomappath, osmpath):
        global topomap
        global topoParameter
        if topomappath is not None:
                try:
                        topomap =  np.array(Image.open(topomappath))[:,:,0] #y,x,rgba
                except:
                        topomap =  np.array(Image.open(topomappath))[:,:] #y,x,rgba
                topomap=np.rot90(topomap)
                topomap=np.rot90(topomap)
        topoParameter = giveMaxMinLongLat(osmpath)
        return topoParameter

global maximumheight, minimumheight
maximumheight = 0.0
minimumheight = 0.0
def setHeights(minimum, maximum):
        global maximumheight, minimumheight
        minimumheight = minimum
        maximumheight  = maximum


#Cell
def giveHeight(x,y,minRemoved = True):
                global topoParameter
                global topomap
                global maximumheight, minimumheight
                if topomap is not None:
                        if not minRemoved:
                                x_lookup= int(topomap.shape[1]*(x-topoParameter[0])/(topoParameter[1]-topoParameter[0]))
                                y_lookup = int(topomap.shape[0]*(1.0-(y-topoParameter[2])/(topoParameter[3]-topoParameter[2])))
                        else:
                                x_lookup= int(topomap.shape[1]*x/(topoParameter[1]-topoParameter[0]))
                                y_lookup = int(topomap.shape[0]*(1.0-(y/(topoParameter[3]-topoParameter[2]))))
                        x_lookup = min(max(topomap.shape[1]-x_lookup-1,0),topomap.shape[1]-1)
                        y_lookup = min(max(topomap.shape[0]-1-y_lookup,0),topomap.shape[0]-1)
                        height = topomap[y_lookup,x_lookup]-np.min(topomap)
                        height = height/np.max(topomap)
                        height = height * (maximumheight-minimumheight) + minimumheight
                else:
                        height = 0.0
                return height

#Cell
def giveMaxMinLongLat(osmpath, trustOSMHeaderMinMax = False):
        global referenceLat
        global referenceLon
        global transformer
        minlat = 999999.0
        maxlat = -999999.0
        minlon = 999999.0
        maxlon = -999999.0
        for entity in parse_file(osmpath):
                if isinstance(entity, Node):
                        if minlat > entity.lat:
                                minlat = entity.lat
                        if maxlat < entity.lat:
                                maxlat = entity.lat
                        if minlon > entity.lon:
                                minlon = entity.lon
                        if maxlon < entity.lon:
                                maxlon = entity.lon

        if trustOSMHeaderMinMax:
                with open(osmpath, "r") as f:
                        for line in f:
                                if "minlat='" in line:
                                        minlat = float(line.split("minlat='")[1].split("'")[0])
                                if "maxlat='" in line:
                                        maxlat = float(line.split("maxlat='")[1].split("'")[0])
                                if "maxlon='" in line:
                                        maxlon = float(line.split("maxlon='")[1].split("'")[0])
                                if "minlon='" in line:
                                        minlon = float(line.split("minlon='")[1].split("'")[0])
                                if 'minlat="' in line:
                                        minlat = float(line.split('minlat="')[1].split('"')[0])
                                if 'maxlat="' in line:
                                        maxlat = float(line.split('maxlat="')[1].split('"')[0])
                                if 'maxlon="' in line:
                                        maxlon = float(line.split('maxlon="')[1].split('"')[0])
                                if 'minlon="' in line:
                                        minlon = float(line.split('minlon="')[1].split('"')[0])
        print("minlon = {},minlat= {},maxlon = {},maxlat = {}".format(minlon,minlat,maxlon,maxlat))
        if referenceLat is None:
            referenceLat =  minlat
            referenceLon = minlon
        #initialize the projectionTransformer with the found referenceprojections
        uproj = CRS.from_proj4("+proj=tmerc +lat_0={0} +lon_0={1} +x_0=0 +y_0=0 +ellps=GRS80 +units=m".format(referenceLat, referenceLon))
        transformer = Transformer.from_crs(crs_4326, uproj)
        
        xmin,ymin = convertLongitudeLatitude(minlon,minlat)
        xmax,ymax = convertLongitudeLatitude(maxlon,maxlat)
        return xmin, xmax, ymin, ymax


#Cell
def convertLongitudeLatitude(longitude,latitude):
        #return longitude, latitude
        x,y = next(transformer.itransform([(latitude,longitude)]))
        #x = latitude
        #y = longitude
        return x,y

def giveReferences():
        global referenceLat
        global referenceLon
        global topoParameter
        return referenceLon, referenceLat, topoParameter

#Cell
def distance(x1,y1,x2,y2):
    return ((x1-x2)**2 + (y1-y2)**2)**0.5

#Cell
def schnittpunkt(x1,y1,hdg1,x2,y2,hdg2):
    #x1 + np.cos(hdg1) * r1 = x2 + np.cos(hdg2) * r2 = x_s
    #y1 + np.sin(hdg1) * r1 = y2 + np.sin(hdg2) * r2 = y_s

    #r1 =   (x2-x2 + np.cos(hdg2) * r2) /(np.cos(hdg1))
    #alt r1 = (y2-y1 + np.sin(hdg2) * r2) /(np.sin(hdg1))

    #---> r2= (x1-x2 + np.cos(hdg1) * r1)/np.cos(hdg2)
    #---> alt: r2= (y1 -y2+ np.sin(hdg1) * r1 )/np.sin(hdg2)
    #r2 ersetzen
    #y1 + np.sin(hdg1) * r1 = y2 + np.sin(hdg2) * (x1-x2 + np.cos(hdg1) * r1)/np.cos(hdg2)
    #alt: x1 + np.cos(hdg1) * r1 = x2 + np.cos(hdg2) * (y1 -y2+ np.sin(hdg1) * r1 )/np.sin(hdg2)
    #r1 ersetzen
    #y1 + np.sin(hdg1) * (x2-x2 + np.cos(hdg2) * r2) /(np.cos(hdg1)) = y2 + np.sin(hdg2) * r2
    if abs(np.sin(hdg1) * np.cos(hdg2) - np.sin(hdg2) *np.cos(hdg1)) < 0.02:
        r2 = ( y1*np.cos(hdg1) + np.sin(hdg1) * (x2-x2)-y2*np.cos(hdg1)) /((np.sin(hdg2)*np.cos(hdg1) - np.sin(hdg1) *np.cos(hdg2) ))
        if abs(abs(hdg1) -np.pi/2.0) < 0.2:
            r1 = (y2-y1 + np.sin(hdg2) * r2) /(np.sin(hdg1))
        else:
            r1 =   (x2-x2 + np.cos(hdg2) * r2) /(np.cos(hdg1))
    else:
        r1 = (-y1*np.cos(hdg2)+ y2*np.cos(hdg2) + np.sin(hdg2) *x1-np.sin(hdg2) *x2 )/(np.sin(hdg1) * np.cos(hdg2) - np.sin(hdg2) *np.cos(hdg1))
        if abs(abs(hdg2) -np.pi/2.0) < 0.2:
            r2 = (y1 -y2+ np.sin(hdg1) * r1 )/np.sin(hdg2)
        else:
            r2 = (x1-x2 + np.cos(hdg1) * r1)/np.cos(hdg2)
    x_s = x1 + np.cos(hdg1) * r1
    y_s = y1 + np.sin(hdg1) * r1
    return x_s,y_s,r1,r2

#Cell
def createVirtualLastPointForJunctionRoads(jx,jy,jrxs,jrys,radius = 3.0):
    lastPoints = []
    for i in range(len(jrxs)):
        x = jrxs[i]
        y = jrys[i]
        hdg1 = giveHeading(jx,jy,x,y)
        #get the 4 points to either side of the road
        hdg1_90 = hdg1-np.pi/2.0
        x1t1 = x + radius*np.cos(hdg1_90)
        y1t1 = y + radius*np.sin(hdg1_90)
        x1t2 = x - radius*np.cos(hdg1_90)
        y1t2 = y - radius*np.sin(hdg1_90)
        relevantr = [min(distance(jx,jy,x,y)*0.9,radius)] #0.9 da es die vorletzten Punkte des Weges sind->
        #                                                  ansonsten sind die vorletzten und letzen punkte gleich
        for j in range(len(jrxs)):
            if i == j:
                continue
            x2 = jrxs[j]
            y2 = jrys[j]
            hdg2 = giveHeading(jx,jy,x2,y2)
            #get the 4 points besides this line
            hdg2_90 = hdg2-np.pi/2.0
            x2t1 = x2 + radius*np.cos(hdg2_90)
            y2t1 = y2 + radius*np.sin(hdg2_90)
            x2t2 = x2 - radius*np.cos(hdg2_90)
            y2t2 = y2 - radius*np.sin(hdg2_90)
            # get the 4 intersectionPoints
            ix1,iy1,_,_ = schnittpunkt(x1t1,y1t1,hdg1,x2t1,y2t1,hdg2)
            ix1,iy1,_,_ = schnittpunkt(ix1,iy1,hdg1_90,x,y,hdg1)
            ix2,iy2,_,_ = schnittpunkt(x1t2,y1t2,hdg1,x2t1,y2t1,hdg2)
            ix2,iy2,_,_ = schnittpunkt(ix2,iy2,hdg1_90,x,y,hdg1)
            ix3,iy3,_,_ = schnittpunkt(x1t1,y1t1,hdg1,x2t2,y2t2,hdg2)
            ix3,iy3,_,_ = schnittpunkt(ix3,iy3,hdg1_90,x,y,hdg1)
            ix4,iy4,_,_ = schnittpunkt(x1t2,y1t2,hdg1,x2t2,y2t2,hdg2)
            ix4,iy4,_,_ = schnittpunkt(ix4,iy4,hdg1_90,x,y,hdg1)
            ix = [ix1,ix2,ix3,ix4]
            iy = [iy1,iy2,iy3,iy4]
            #get the radius from relevant intersections
            for k in range(4):
                if min(x,jx)<ix[k]<max(x,jx) and min(y,jy)<iy[k]<max(y,jy): #point is on Line x,y to jx,jy! Relevant
                    relevantr.append(distance(ix[k],iy[k],jx,jy))
        lasty,lastx = getXYPositionFromLineLength(jx, jy, hdg1, max(relevantr))
        lastPoints.append([lasty,lastx])
    return lastPoints

#Cell
def getPositiveHeading(hdg):
    while hdg < 0.0:
         hdg+=2.0*np.pi
    return hdg%(np.pi*2.0)

def getDeltaHdg(hdg1,hdg2):
    hdg1 = getPositiveHeading(hdg1)
    hdg2 = getPositiveHeading(hdg2)
    deltaHdg = hdg1-hdg2
    if abs(deltaHdg)//np.pi == 1:
        deltaHdg = (abs(deltaHdg)%np.pi) * np.sign(deltaHdg)
        deltaHdg = np.pi+deltaHdg if deltaHdg < 0.0 else -np.pi + deltaHdg
    else:
        deltaHdg = (abs(deltaHdg)%np.pi) * np.sign(deltaHdg)
    return deltaHdg
