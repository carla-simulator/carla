---
title: Vehicle Type Parameter Defaults
permalink: /Vehicle_Type_Parameter_Defaults/
---

This page describes the vehicle parameter defaults depending on the
vehicle class. If no values are given the values for passenger are used.
It uses as default always the values in the first line for each vehicle
class. The other values are just as an information (to be used with own
vehicle types).

<style type="text/css">
.tg  {border-collapse:collapse;border-spacing:0;}
.tg td{font-family:Arial, sans-serif;font-size:14px;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;border-color:black;}
.tg th{font-family:Arial, sans-serif;font-size:14px;font-weight:normal;padding:10px 5px;border-style:solid;border-width:1px;overflow:hidden;word-break:normal;border-color:black;}
.tg .tg-cly1{text-align:left;vertical-align:middle}
.tg .tg-eaoh{font-weight:bold;background-color:#ffdead;text-align:center;vertical-align:top}
.tg .tg-dr3j{color: #495057; font-weight:bold;background-color:#ffdead;text-align:center;vertical-align:middle}
.tg .tg-ye8u{background-color:#ffdead;text-align:center;vertical-align:top}
.tg .tg-0lax{text-align:left;vertical-align:top}
.tg .tg-c882{color: #495057; font-weight:bold;background-color:#e0ffd0;text-align:center;vertical-align:top}
</style>
<table class="tg">
  <tr>
    <th class="tg-dr3j">vClass (SVC)</th>
    <th class="tg-dr3j">example</th>
    <th class="tg-dr3j">shape (SVS)</th>
    <th class="tg-dr3j">length<br>width<br>height</th>
    <th class="tg-dr3j">minGap</th>
    <th class="tg-dr3j">a<sub>max</sub><sup>(**)</sup>accel<br></th>
    <th class="tg-dr3j">b<sup>(**)</sup> decel</th>
    <th class="tg-dr3j">b<sub>e</sub>emergency decel</th>
    <th class="tg-dr3j">v<sub>max</sub> maxSpeed</th>
    <th class="tg-dr3j">seats</th>
    <th class="tg-dr3j"><a href="Models/Emissions/HBEFA3-based.html">emissionClass (HBEFA3)</a></th>
  </tr>
  <tr>
    <td class="tg-cly1">pedestrian</td>
    <td class="tg-cly1"><img src="images/Wikicommons_pedestrian.jpg" title="wikicommons_pedestrian.jpg" width="200" alt="" /></td>
    <td class="tg-cly1">pedestrian</td>
    <td class="tg-cly1">0.215m<sup>(1)</sup><br>0.478m<sup>(1)</sup><br>1.719m<sup>(1)</sup></td>
    <td class="tg-cly1">0.25m</td>
    <td class="tg-0lax">1.5m/s<sup>2(23)</sup></td>
    <td class="tg-0lax">2m/s<sup>2(23)</sup></td>
    <td class="tg-0lax">5m/s<sup>2</sup></td>
    <td class="tg-0lax">5.4km/h<sup>(23)</sup></td>
    <td class="tg-0lax">-</td>
    <td class="tg-0lax">zero</td>
  </tr>
  <tr>
    <td class="tg-c882" colspan="11">two-wheeled vehicle</td>
  </tr>
  <tr>
    <td class="tg-0lax">bicycle</td>
    <td class="tg-0lax"><img src="images/Wikicommons_bicycle.jpg" title="wikicommons_bicycle.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">bicycle</td>
    <td class="tg-0lax">1.6m<sup>(17)</sup><br>0.65m<sup>(17)</sup><br>1.7m<sup>(*)</sup></td>
    <td class="tg-0lax">0.5m</td>
    <td class="tg-0lax">1.2m/s<sup>2(19)</sup></td>
    <td class="tg-0lax">3m/s<sup>2(19)</sup></td>
    <td class="tg-0lax">7m/s<sup>2</sup></td>
    <td class="tg-0lax">20km/h<sup>(19)</sup></td>
    <td class="tg-0lax">1</td>
    <td class="tg-0lax">zero</td>
  </tr>
  <tr>
    <td class="tg-0lax">moped</td>
    <td class="tg-0lax"><img src="images/Wikicommons_moped.jpg" title="wikicommons_moped.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">moped</td>
    <td class="tg-0lax">2.1m<sup>(17)</sup><br>0.8m<sup>(17)</sup><br>1.7m<sup>(*)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">1.1m/s<sup>2(25)</sup></td>
    <td class="tg-0lax">7m/s<sup>2(26)</sup></td>
    <td class="tg-0lax">10m/s<sup>2</sup></td>
    <td class="tg-0lax">45km/h<sup>(24)</sup></td>
    <td class="tg-0lax">2</td>
    <td class="tg-0lax">zero</td>
  </tr>
  <tr>
    <td class="tg-0lax">motorcycle</td>
    <td class="tg-0lax"><img src="images/Wikicommons_motorcycle.jpg" title="wikicommons_motorcycle.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">motorcycle</td>
    <td class="tg-0lax">2.2m<sup>(28)</sup><br>0.9m<sup>(28)</sup><br>1.5m<sup>(28)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">6m/s<sup>2(19)</sup></td>
    <td class="tg-0lax">10m/s<sup>2(27)</sup></td>
    <td class="tg-0lax">10m/s<sup>2</sup></td>
    <td class="tg-0lax">200km/h<sup>(28)</sup></td>
    <td class="tg-0lax">2</td>
    <td class="tg-0lax">LDV_G_EU6</td>
  </tr>
  <tr>
    <td class="tg-c882" colspan="11">passenger car</td>
  </tr>
  <tr>
    <td class="tg-0lax" rowspan="5">passenger</td>
    <td class="tg-0lax"><img src="images/Wikicommons_passenger.jpg" title="wikicommons_passenger.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">passenger</td>
    <td class="tg-0lax">4.3m<sup>(29)</sup><br>1.8m<sup>(29)</sup><br>1.5m<sup>(29)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">2.9m/s<sup>2(29)</sup></td>
    <td class="tg-0lax">7.5m/s<sup>2(27)</sup></td>
    <td class="tg-0lax">9m/s<sup>2</sup></td>
    <td class="tg-0lax">180km/h<sup>(29)</sup></td>
    <td class="tg-0lax">5</td>
    <td class="tg-0lax">LDV_G_EU4</td>
  </tr>
  <tr>
    <td class="tg-0lax"><img src="images/Wikicommons_passenger_sedan.jpg" title="wikicommons_passenger_sedan.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">passenger/sedan</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">5</td>
    <td class="tg-0lax">LDV_G_EU4</td>
  </tr>
  <tr>
    <td class="tg-0lax"><img src="images/Wikicommons_passenger_hatchback.jpg" title="wikicommons_passenger_hatchback.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">passenger/hatchback</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">5</td>
    <td class="tg-0lax">LDV_G_EU4</td>
  </tr>
  <tr>
    <td class="tg-0lax"><img src="images/Wikicommons_passenger_wagon.jpg" title="wikicommons_passenger_wagon.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">passenger/wagon</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">5</td>
    <td class="tg-0lax">LDV_G_EU4</td>
  </tr>
  <tr>
    <td class="tg-0lax"><img src="images/Wikicommons_passenger_van.jpg" title="wikicommons_passenger_van.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">passenger/van</td>
    <td class="tg-0lax">4.7m<sup>(16)</sup><br>1.9m<sup>(16)</sup><br>1.73m<sup>(16)</sup></td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">6<br></td>
    <td class="tg-0lax">LDV_G_EU4</td>
  </tr>
  <tr>
    <td class="tg-0lax">emergency</td>
    <td class="tg-0lax"><img src="images/Wikicommons_emergency.jpg" title="wikicommons_emergency.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">delivery</td>
    <td class="tg-0lax">6.5m<sup>(37)</sup><br>2.16m<sup>(37)</sup><br>2.86m<sup>(37)</sup></td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">3</td>
    <td class="tg-0lax">LDV</td>
  </tr>
  <tr>
    <td class="tg-0lax">delivery</td>
    <td class="tg-0lax"><img src="images/Wikicommons_delivery.jpg" title="wikicommons_delivery.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">delivery</td>
    <td class="tg-0lax">6.5m<sup>(37)</sup><br>2.16m<sup>(37)</sup><br>2.86m<sup>(37)</sup></td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">3</td>
    <td class="tg-0lax">LDV</td>
  </tr>
  <tr>
    <td class="tg-c882" colspan="11">truck</td>
  </tr>
  <tr>
    <td class="tg-0lax">truck</td>
    <td class="tg-0lax"><img src="images/Wikicommons_transport.jpg" title="wikicommons_transport.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">truck</td>
    <td class="tg-0lax">7.1m<sup>(30)</sup><br>2.4m<sup>(30)</sup><br>2.4m<sup>(30)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">1.3m/s<sup>2(31)</sup></td>
    <td class="tg-0lax">4m/s2<br>(27)</td>
    <td class="tg-0lax">7m/s<sup>2</sup></td>
    <td class="tg-0lax">130km/h<sup>(35)</sup></td>
    <td class="tg-0lax">3</td>
    <td class="tg-0lax">HDV</td>
  </tr>
  <tr>
    <td class="tg-0lax" rowspan="2">trailer</td>
    <td class="tg-0lax"><img src="images/Wikicommons_transport_semitrailer.jpg" title="wikicommons_transport_semitrailer.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">truck/semitrailer</td>
    <td class="tg-0lax">16.5m<sup>(14)</sup><br>max. 2.55m<sup>(13)</sup><br>max. 4m<sup>(13)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">1.1m/s<sup>2(31)</sup></td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">3</td>
    <td class="tg-0lax">HDV</td>
  </tr>
  <tr>
    <td class="tg-0lax"><img src="images/Wikicommons_transport_trailer.jpg" title="wikicommons_transport_trailer.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">truck/trailer</td>
    <td class="tg-0lax">max. 18.75m<sup>(13)</sup><br>max. 2.55m<sup>(13)</sup><br>max. 4m<sup>(13)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">1m/s<sup>2(31)</sup></td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">3</td>
    <td class="tg-0lax">HDV</td>
  </tr>
  <tr>
    <td class="tg-c882" colspan="11">bus</td>
  </tr>
  <tr>
    <td class="tg-0lax" rowspan="2">bus</td>
    <td class="tg-0lax"><img src="images/Wikicommons_bus_city.jpg" title="wikicommons_bus_city.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">bus</td>
    <td class="tg-0lax">12m<sup>(3)</sup><br>2.5m<sup>(3)</sup><br>3.4m<sup>(3)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">1.2m/<sup>s2(18)</sup></td>
    <td class="tg-0lax">4m/s<sup>2(27)</sup></td>
    <td class="tg-0lax">7m/s<sup>2</sup></td>
    <td class="tg-0lax">85km/h<sup>(3)</sup></td>
    <td class="tg-0lax">~85</td>
    <td class="tg-0lax">Bus</td>
  </tr>
  <tr>
    <td class="tg-0lax"><img src="images/Wikicommons_bus_flexible.jpg" title="wikicommons_bus_flexible.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">bus/flexible</td>
    <td class="tg-0lax">17.9m<sup>(3)</sup><br>2.5m<sup>(3)</sup><br>3.0m<sup>(3)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">1.2m/s<sup>2(18)</sup></td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">~150</td>
    <td class="tg-0lax">Bus</td>
  </tr>
  <tr>
    <td class="tg-0lax">coach</td>
    <td class="tg-0lax"><img src="images/Wikicommons_overland.jpg" title="wikicommons_overland.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">bus/coach</td>
    <td class="tg-0lax">14m<sup>(10)</sup><br>2.6m<sup>(10)</sup><br>4.m<sup>(10)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">2.0m/s<sup>2(*)</sup></td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">see above</td>
    <td class="tg-0lax">100km/h<sup>(32)</sup></td>
    <td class="tg-0lax">70<sup>(9)</sup></td>
    <td class="tg-0lax">Coach</td>
  </tr>
  <tr>
    <td class="tg-c882" colspan="11">rail</td>
  </tr>
  <tr>
    <td class="tg-0lax">tram</td>
    <td class="tg-0lax"><img src="images/Wikicommons_lightrail.jpg" title="wikicommons_lightrail.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">rail/railcar</td>
    <td class="tg-0lax">~22m<sup>(7)</sup><br>~2.4m<sup>(7)</sup><br>~3.2m<sup>(7)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">1.0m/s<sup>2(18)</sup></td>
    <td class="tg-0lax">3.0m/s<sup>2(27)</sup></td>
    <td class="tg-0lax">7m/s<sup>2</sup></td>
    <td class="tg-0lax">80km/h<sup>(36)</sup></td>
    <td class="tg-0lax">~120<sup>(7, *)</sup></td>
    <td class="tg-0lax">zero</td>
  </tr>
  <tr>
    <td class="tg-0lax">rail_urban</td>
    <td class="tg-0lax"><img src="images/Wikicommons_rail_city.jpg" title="wikicommons_rail_city.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">rail/railcar</td>
    <td class="tg-0lax">36.5m(*2/*4)<sup>(4)</sup><br>3.0m<sup>(4)</sup><br>3.6m<sup>(4)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">1.0m/s<sup>2(4)</sup></td>
    <td class="tg-0lax">3.0m/s<sup>2(27)</sup></td>
    <td class="tg-0lax">7m/s<sup>2</sup></td>
    <td class="tg-0lax">100km/h<sup>(4)</sup></td>
    <td class="tg-0lax">~300(*2/*4)</td>
    <td class="tg-0lax">zero</td>
  </tr>
  <tr>
    <td class="tg-0lax" rowspan="2">rail</td>
    <td class="tg-0lax"><img src="images/Wikicommons_rail_slow.jpg" title="wikicommons_rail_slow.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">rail</td>
    <td class="tg-0lax">67.5m(*2)<sup>(11)</sup><br>2.84m<sup>(11)</sup><br>3.75m<sup>(11)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">0.25m/s<sup>2(34)</sup></td>
    <td class="tg-0lax">1.3m/s<sup>2(33)</sup></td>
    <td class="tg-0lax">5m/s<sup>2</sup></td>
    <td class="tg-0lax">160km/h<sup>(11)</sup></td>
    <td class="tg-0lax">~434(*2)<sup>(11)</sup></td>
    <td class="tg-0lax">HDV_D_EU0</td>
  </tr>
  <tr>
    <td class="tg-0lax"><img src="images/Wikicommons_rail_cargo.jpg" title="wikicommons_rail_cargo.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">rail/cargo</td>
    <td class="tg-0lax">750m (~18.9m + x*~16m)<sup>(5, 6, 8)</sup><br>~4.4m(6)<br>~3.0m(6)</td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">0.25m/s<sup>2(34)</sup></td>
    <td class="tg-0lax">1.3m/s<sup>2(33)</sup></td>
    <td class="tg-0lax">5m/s<sup>2</sup></td>
    <td class="tg-0lax">120km/h<sup>(5)</sup></td>
    <td class="tg-0lax">1</td>
    <td class="tg-0lax">HDV_D_EU0</td>
  </tr>
  <tr>
    <td class="tg-0lax">rail_electric</td>
    <td class="tg-0lax"><img src="images/Wikicommons_rail_fast.jpg" title="wikicommons_rail_fast.jpg" width="200" alt="" /></td>
    <td class="tg-0lax">rail</td>
    <td class="tg-0lax">~25m(*8)<sup>(12)</sup><br>2.95m<sup>(12)</sup><br>3.89m<sup>(12)</sup></td>
    <td class="tg-0lax">2.5m</td>
    <td class="tg-0lax">0.5m/s<sup>2(33)</sup></td>
    <td class="tg-0lax">1.3m/s<sup>2(33)</sup></td>
    <td class="tg-0lax">5m/s<sup>2</sup></td>
    <td class="tg-0lax">330km/h<sup>(12)</sup></td>
    <td class="tg-0lax">425<sup>(12)</sup></td>
    <td class="tg-0lax">zero</td>
  </tr>
  <tr>
    <td class="tg-c882" colspan="11">electric vehicle</td>
  </tr>
  <tr>
    <td class="tg-0lax">evehicle</td>
    <td class="tg-0lax"></td>
    <td class="tg-0lax">evehicle</td>
    <td class="tg-0lax">see passenger</td>
    <td class="tg-0lax">see passenger</td>
    <td class="tg-0lax">see passenger</td>
    <td class="tg-0lax">see passenger</td>
    <td class="tg-0lax">see passenger</td>
    <td class="tg-0lax">see passenger</td>
    <td class="tg-0lax">see passenger</td>
    <td class="tg-0lax">zero</td>
  </tr>
  <tr>
    <td class="tg-c882" colspan="11">ships</td>
  </tr>
  <tr>
    <td class="tg-0lax">ship</td>
    <td class="tg-0lax"></td>
    <td class="tg-0lax">ship</td>
    <td class="tg-0lax">17m<br>4m<br>4m</td>
    <td class="tg-0lax">2.5</td>
    <td class="tg-0lax">0.1m/s<sup>2</sup></td>
    <td class="tg-0lax">0.1m/s<sup>2</sup></td>
    <td class="tg-0lax">1m/s<sup>2</sup></td>
    <td class="tg-0lax">4.12 (8 Knots)</td>
    <td class="tg-0lax"></td>
    <td class="tg-0lax">HDV_D_EU0</td>
  </tr>
</table>


- (1):
  <http://www.baua.de/de/Publikationen/AWE/Band3/AWE108.pdf?__blob=publicationFile&v=4>;
- (2): <http://www.motorrad-leuschner-d.de/motorraeder.html>
- (3): www.bvg.de/index.php/de/binaries/asset/download/470184/file/1-1
- (4): <http://de.wikipedia.org/wiki/S-Bahn_Berlin> und folgende
- (5): <http://de.wikipedia.org/wiki/Schieneng%C3%BCterverkehr>
- (6): <http://de.wikipedia.org/wiki/Bombardier_Traxx>
- (7): <http://de.wikipedia.org/wiki/GTxN/M/S>
- (8): <http://de.wikipedia.org/wiki/Flachwagen>
- (9): <http://de.wikipedia.org/wiki/Reisebus>
- (10):
  <http://www.neoplan-bus.com/cms/media/de/content/downloads/skyliner/NEOPLAN_SKYLINER.pdf>
- (11): <http://de.wikipedia.org/wiki/DBAG-Baureihe_425>
- (12): <http://de.wikipedia.org/wiki/ICE_3>
- (14):
  <http://www.mantruckandbus.com/man/media/migrated/doc/master_1/Transportloesungen__de_.pdf>
- (15): <http://de.wikipedia.org/wiki/Opel_Vivaro>
- (16): <http://de.wikipedia.org/wiki/Renault_Espace>
- (17): Fußgeher- und Fahrradverkehr: Planungsprinzipien; Hermann
  Knoflacher; Böhlau Verlag Wien, 1995
- (18): RiLSA 2010
- (19): <http://www.gutax.de/Diplomarbeit_Manfred_Bulla.pdf>
- (20):
  <http://tuprints.ulb.tu-darmstadt.de/905/1/Schroeder_Frank_FZD_Diss.pdf>
- (21): <http://www.unfallrekonstruktion.de/pdf/nickel.pdf>
- (22): <http://www.trb.org/publications/nchrp/nchrp_rpt_505.pdf>
  (p48)
- (23): <http://jml2012.indexcopernicus.com/fulltxt.php?ICID=1022800>
  (pdf p.229-230)
- (24): <http://de.wikipedia.org/wiki/Kleinkraftrad>
- (25): <http://de.wikipedia.org/wiki/Vespa_LX>
- (26):
  <http://www.colliseum.net/wiki/Bremsversuche_mit_dem_Kleinkraftrad_S_50_/_S_51>
- (27):
  <http://www.unfallaufnahme.info/uebersichten-listen-und-tabellen/geschwindigkeiten-und-bremswege/index.html>
- (28): <http://www.bmw-motorrad.de/>
- (29):
  <http://www.volkswagen.de/de/models/golf_7/trimlevel_overview.s9_trimlevel_detail.suffix.html/der-golf-cup~2Fcup.html#/tab=2cf06ac5c643bf892618c835c90c32ac>
- (30): <https://www.buchbinder.de/de/fahrzeuge.html>
- (31):
  <http://www.colliseum.net/wiki/Lkw-Anfahrbeschleunigungswerte_f%C3%BCr_die_Praxis>
- (32):
  <http://de.wikipedia.org/wiki/Zul%C3%A4ssige_H%C3%B6chstgeschwindigkeit_im_Stra%C3%9Fenverkehr_(Deutschland)>
- (33):
  <http://www.ice-fansite.com/index.php?option=com_content&view=article&id=195:ice1-techdat&catid=50:icet&Itemid=69>
- (34):
  <http://en.wikipedia.org/wiki/Orders_of_magnitude_(acceleration)>
- (35):
  <https://de.answers.yahoo.com/question/index?qid=20061122162708AAtzoPP>
- (36): <http://ka.stadtwiki.net/H%C3%B6chstgeschwindigkeit_(Bahn)>
- (37):
  <http://www.rettungsdienst.brk.de/technik/rtw/rtw_by_2010/daten>
- (\*): estimated
- (\*\*): need to be revisited to fit to model behavior