---
title: DiplomStudArb
permalink: /DiplomStudArb/
---

In the following, you may find ideas for student work - Bachelor,
Master, or Diploma theses. Please note that being a German institution,
we can only offer assistance for work done on German Universities /
Schools. That's why the work topics are listed in German.

## Allgemeines

Die meisten der folgenden Themen lassen sich sowohl im Rahmen einer
Master- oder Diplomarbeit als auch in reduziertem Umfang in einer
Studien- oder Bachelorarbeit behandeln. Alle stehen mehr oder weniger
direkt im Zusammenhang mit [SUMO](http://sumo.dlr.de/).

Alle Themen sind ausdrücklich nur als Vorschläge gedacht. Falls jemand
nach Kennenlernen der Software und der Anwendungsgebiete eigene
Vorschläge zur Verbesserung und Erweiterung hat, die er/sie selbst
umsetzen möchte, sind diese sehr gern gesehen.

## Themen mit Bezug zur Verkehrsforschung

<em>Voraussetzung: Kenntnisse in C++ oder Python</em>

### Schadstoffemission

#### Implementierung und Vergleich von Emissionsmodellen

Es existiert eine Vielzahl unterschiedlicher Modelle für den Ausstoß von
Schadstoffen, die sich in ihrer Granularität sowie der Menge
abgebildeter Fahrzeugarten unterscheiden. Das Ziel dieser Arbeit ist die
Umsetzung eines Teils dieser Modelle in SUMO und der Vergleich dieser.
Die Arbeit würde auf bereits vorhandenen Ergebnissen aufbauen.

### Elektromobilität - Simulation der Auswirkung von Verkehrsmanagementstrategien

```
Gegenwärtig hält eine neue Entwicklung mit all ihren Chancen und Herausforderungen Einzug in den Verkehrsraum – die Elektromobilität. Sie hat den Vorteil 
nur geringe Lärm- und Schadstoff-emissionen zu verursachen. Doch die Elektromobilität wird die bisherigen Probleme im Verkehrsmanagement nicht automatisch lösen 
Ganz im Gegenteil – im Zuge der Elektrifizierung des Verkehrs kommen neue Herausforderungen auf das Verkehrsmanagement zu 
Die begrenzte Reichweite und langen Ladevorgänge der bisherigen Elektrofahrzeuge bedingen eine neuartige Lade- und Verkehrsinfrastruktur 
Hinzu kommen unklare Rahmenbedingungen wie Kaufbereitschaft und Marktpreise. Die Konsequenzen für das Verkehrsmanagement sind daher kaum absehbar.
```

Wir möchten zusammen mit einem Studenten/einer Studentin
Verkehrsmanagementstrategien unter Beachtung verschiedener Markt- und
Nutzungsszenarien simulieren. Damit sollen Änderungen im
Verkehrsmanagement virtuell erprobt und deren Auswirkungen bspw. auf
deren Beitrag zum Klimaschutz untersucht werden. Mögliche
Aufgabengebiete umfassen dabei: Recherche und Aufbereitung von Markt-
und Nutzungsszenarien, Ableitung von Verkehrsmanagementstrategien /
Recherche bisheriger Anstze, Emissionsprofil für verschiedenen
Fahrzeugtypen (PHEV, REEV, BEV, FCEV etc.) implementieren, Simulation
verschiedener Steuerungs- und Beeinflussungsmaßnahmen, Auswertung der
Ergebnisse / Ziehen von Schlussfolgerungen.

### Lichtsignalanlagen

#### Methoden für eine Schätzung einer "grünen Welle" in komplexen Straßennetzen

Basierend auf der Auswertung bereits existierender Ansätze ist ein
Verfahren auszuwählen und zu implementieren, welches für ein gegebenes
Straßennetz und eine gegebene Nachfrage möglichst optimale, also z.B.
die Wartezeiten vor den Lichtsignalanlagen minimierende, Programme der
Lichtsignalanlagen erzeugt.

#### Vergleich verschiedener Ansätze zur Steuerung von Lichtsignalanlagen

Ziel der Arbeit ist es, das modellbasierte Steuerverfahren OPAC mit
einer Festzeitsteuerung und einer regelbasierten Steuerung zu
vergleichen. Dazu ist zunächst zu untersuchen, mit welchem Aufwand sich
die Steuerverfahren in der Simulation umsetzen lassen. Anschließend
sollen möglichst alle Steuerverfahren in konkreten Simulationsszenarien
umgesetzt und auf Basis der von SUMO generierten Ausgabedateien
verglichen werden.

### Routenwahl und Nachfragemodellierung

#### Auswertung von one-shot-Ansätzen zur Verkehrsumlegung

Die Verteilung von Routen innerhalb eines belasteten Straßennetzes
gehört zu den Standardschritten bei der Erstellung eines
Verkehrssimulationsszenarios. Normalerweise benutzte Methoden sind dabei
iterativ und benötigen daher viel Rechenzeit. Innerhalb dieser Arbeit
soll ein bereits entwickelter one-shot - Ansatz, der ohne Iterationen
auskommt im Vergleich zu herkömmlichen Verfahren evaluiert werden.

#### Alternative Algorithmen für Kürzeste-Wege-Suche

Innerhalb dieser Arbeit sollen Alternativen zu dem in SUMO benutzten
Kürzeste-Wege-Algorithmus von Dijkstra implementiert und verglichen
werden.

#### Ad-hoc - Nachfragegenerierung

Während mit OpenStreetMap reale Straßennetze frei zur Verfügung stehen,
gibt es kaum Quellen für die von der Simulation benötigte
Verkehrsnachfrage. Innerhalb dieser Arbeit sollen Heuristiken zur
Bestimmung einer Nachfrage für ein gegebenes Straßennetz entwickelt,
implementiert und ausgewertet werden.

#### Nachfragegenerierung aus Induktionsschleifendaten

Die nahezu vollständige Abdeckung aller Zu- und Abfahrten an Autobahnen
erlaubt die Modellierung einer Nachfrage für ein so beschriebenes
Autobahnnetz. Innerhalb dieser Arbeit soll ein bereits entwickeltes
Werkzeug auf ein solides theoretisches Fundament gestellt werden. Zudem
sollen die vorhandenen, realen Daten dazu benutzt werden, die
Möglichkeiten und Probleme bei der Benutzung dieses Werkzeugs auf zu
decken.

#### Anpassung einer Verkehrsnachfrage an Schleifendaten

Es soll ein Werkezeug erstellt und beschrieben werden, das eine für ein
Straßennetz gegebene Nachfrage an die innerhalb dieses Netzes erhobenen
Daten anpassen können soll. Vorarbeiten hierzu liegen vor.

#### Evaluierung des Routings nach Abbiegehäufigkeiten

Der existierende jtrrouter erstellt mittels einens einfachen
Wahrscheinlichkeitsmodells Routen aus Abbiegehäufigkeiten an Kreuzungen.
Dieses Modell sollte evaluiert und verfeinert werden.

### Kurzfristige Verkehrsprognose

#### Vorhersage von Ganglinien

Es sollen eigene Ideen entwickelt werden, wie Vorhersagen zur
Fahrzeugzahl und -geschwindigkeit in einem Straßennetz aufgrund
historischer Daten und dem Einsatz maschineller Lernverfahren gemacht
werden können.

## Themen mit Bezug zu Softwareentwicklung

### Schwerpunkt Algorithmik / theoretischer Informatik

<em>Voraussetzungen: keine Angst vor Beweisen, Grundkenntnisse der
Graphentheorie (z.B. aus einer Vorlesung zu Graphen und
Algorithmen)</em>

#### Analyse und Implementation eines Netzwerkflussalgorithmus zur Routenfindung

Im Rahmen der Arbeit sollte ein schon weitgehend entwickelter
Algorithmus zur Routenfindung effizient implementiert, seine Laufzeit
theoretisch und praktisch untersucht und seine Lösungen verbessert
werden. Ziel der Arbeit ist einen Routenfindungsalgorithmus zu
erstellen, der in der Praxis schnell und trotzdem theoretisch fundiert
vorgegebene Verkehrszählungen annähert.

#### Schnelle Flüsse

Es gibt in der kombinatorischen Optimierung das Konzept der Quickest
flows zur Abbildung der zeitlichen Komponente bei Netzwerkflüssen. Es
soll evaluiert werden, inwieweit diese zur Routenfindung eingesetzt
werden können.

#### Matching von Straßennetzen

Oft liegen für eine Region mehrere Beschreibungen in unterschiedlichen
Formaten und unterschiedlicher Qualität vor. Ziel dieser Arbeit ist die
Auswertung verschiedener Ansätze, die eine Projektion von Straßennetzen
unterschiedlicher Genauigkeit aufeinander realisieren sowie die
Implementierung eines diese Funktion realisierenden Verfahrens.

### Schwerpunkt Software Engineering

<em>Voraussetzung: Kenntnisse in C++</em>

#### Checkstyle für C++

Während es für Java und Python bereits recht ausgereifte Tools zur
statischen Überprüfung des Quellcodes gibt, gibt es für C++ nur wenig
(bezahlbares). Es soll überprüft werden inwieweit bestehende Tools für
Java sich auch bei C++ anwenden lassen.

#### Fixed point vs. Floating point

Gibt es bei modernen Prozessorarchitekturen noch einen deutlichen
Performanceunterschied zwischen Fest- und Fließkommaberechnungen?
Rechtfertigt dieser einen unter Umständen deutlich erhöhten
Implementationsaufwand? Und ändern sich die Antworten auf diese Fragen
für (stark) nebenläufige Programme?

#### Multi-core Parallelisierung

Bislang nutzen die Anwendungen des Softwarepaketes SUMO keine
Parallelisierung. Ziel dieser Arbeit ist die Implementierung einer
Parallelisierung für eine multi-core Architektur und die Messung des so
erhaltenen Geschwindigkeitsgewinns.

### Schwerpunkt XML

<em>Voraussetzung: Erfahrung mit der Parsierung / Erzeugung von XML</em>

#### Effizienter Import und Export von XML mittels schemabasierter Kompression

SUMO erzeugt und importiert zum Teil riesige XML-Dateien, welche die
Simulation durch ihre schiere Größe ausbremsen. Effiziente Kompression
mittels so genanntem binärem XML (z.B. XSBC) soll als Lösungsansatz
evaluiert und implementiert werden.

#### Modulares XML

Die Wiederverwendung von Konfigurationen wird durch die begrenzte
Modularisierbarkeit von "reinem" XML eingeschränkt. Daher sollen in
dieser Arbeit verschiedene (standardisierte) Modularisierungsansätze,
wie z.B. XInclude, untersucht und implementiert werden.

## Sonstiges

#### OpenSource Marketing und Community building

Welche Eigenschaften muss ein Softwareprojekt haben, damit es die
Anwender nutzen und zur Verbesserung beitragen? Spielt die Codequalität
eine Rolle, liegt es vor allem an der Dokumentation? Diesen Fragen
sollen studiert und Verbesserungsvorschläge gerne auch direkt
ausprobiert werden.