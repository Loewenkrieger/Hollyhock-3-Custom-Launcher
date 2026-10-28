# Hollyhock Dual Launcher 0.4 - HHK2-Ansicht auf HHK3

Fuer ClassPad II mit bereits installiertem Hollyhock 3 auf
**OS 02.01.7002.0001**. Es wird nur **run.bin** ausgetauscht.
**Kein Firmwareupdate erforderlich.**

## Beenden korrigiert

Version 0.3 gab beim normalen Beenden nullptr an den Casio-Aufrufer zurueck.
Der Aufrufer liest danach unmittelbar ein Feld dieses Rueckgabeobjekts. Das
war der Fehler in 0.3 und erklaert den Absturz/Neustart beim Schliessen.

**0.4 gibt fuer die gepruefte OS-7002-Aufrufstelle einen gueltigen
Rueckgabeadapter zurueck.** Dessen Abschlussfunktion liefert sofort
"Abbrechen" zurueck. Der normale Casio-Code kann dadurch weiterlaufen,
ohne einen Dialog anzuzeigen oder eine Systemeinstellung zu aendern.
Es wird kein Neustart angefordert. Vorher werden wie bisher die Laufzeit
beendet und die gesicherte Bildschirmansicht wiederhergestellt.

Das gilt fuer Close, X und Clear sowie die normale Rueckkehr aus einer App,
die ueber denselben Abschlussweg laeuft. Gemeint ist die Rueckkehr zur
vorherigen Casio-Ansicht, normalerweise System; es wird nicht automatisch
die Main-Anwendung geoeffnet.

Die Aufrufstelle wird vor dem stillen Ruecksprung anhand ihrer
Maschinenbefehle geprueft. Bei einem anderen OS oder veraenderten Aufrufer
verwendet der Launcher weiterhin den bisherigen Casio-Abschlussdialog.
Fehlercodes werden ebenfalls weiter angezeigt.

HHK2-Layout, Dropdown, Touch- und Tastensteuerung sowie beide App-Lader
bleiben erhalten. Gegenueber 0.3 aendert sich in der Oberflaeche nur die
Versionsnummer in der Fusszeile. Die Schrift ist eine frei lizenzierte
Nachbildung mit DejaVu Sans.

## Installation

1. Die bisherige run.bin auf dem PC sichern.
2. Das ZIP entpacken. Die **run.bin direkt neben dieser Anleitung** in das
   Hauptverzeichnis des Taschenrechner-Laufwerks kopieren und dort ersetzen.
   Nicht einen der Dateien aus fallback/ oder debug/ verwenden.
3. Optional die beiden Test-Apps aus smoke/ ebenfalls direkt ins
   Hauptverzeichnis kopieren.
4. Das Laufwerk sicher auswerfen, USB trennen und den Hollyhock Launcher
   wie bisher ueber das System-Menue oeffnen.

In der Fusszeile steht **0.4**. Zuerst ohne App-Start mit **Close** schliessen. Die normale Casio-Ansicht
sollte wieder erscheinen. Danach erneut oeffnen und X sowie Clear pruefen.
Anschliessend zunaechst **Dual test: HH3**, anschliessend **Dual test: HHK2** starten.
Die Test-Apps zeigen eine Meldung und warten auf eine Taste.

Die Firmware bleibt unveraendert. Das Paket enthaelt keinen Updater.

## Bedienung

| Eingabe | Funktion |
| --- | --- |
| Programmauswahl antippen | Dropdown oeffnen oder schliessen |
| Eintrag im Dropdown antippen | App waehlen und Dropdown schliessen |
| Im langen Dropdown ziehen / Pfeile antippen | Liste scrollen |
| Run | Gewaehlte App starten |
| Close oder X oben rechts | Launcher schliessen |
| Pfeil hoch / runter | Vorherige / naechste App waehlen |
| EXE bei geschlossenem Dropdown | Gewaehlte App starten; bei Fokus auf Close schliessen |
| EXE bei geoeffnetem Dropdown | Auswahl bestaetigen, ohne die App sofort zu starten |
| Pfeil links / rechts bei geschlossenem Dropdown | Fokus zwischen Auswahl, Run und Close wechseln |
| Pfeil links / rechts bei geoeffnetem Dropdown | Eine Seite zurueck / weiter |
| Clear oder Backspace | Erst Dropdown schliessen, sonst Launcher schliessen |

## Unterstuetzte Apps

- **.hh3:** der vorhandene HH3-ELF-Lader bleibt erhalten.
- **.hhk:** die bisherige experimentelle HHK2-Kompatibilitaet bleibt erhalten.
  Der Parser und die Adressuebersetzung wurden fuer diese Version nicht
  veraendert. Einige Apps laufen laut deinem Test mit 0.2 bereits; das
  garantiert keine vollstaendige Kompatibilitaet aller alten Apps.
- Dateien werden direkt in **\\fls0\\** gesucht, nicht in Unterordnern.
- Rohe alte .bin-Apps werden unter OS 7002 nicht unterstuetzt.

HHK2-Dateien brauchen ihre ELF-Symboltabelle und das klassische Speicherlayout
0x8CFF0000 bis 0x8CFFFFFF. Bekannte SDK-Adressen werden in der geladenen
RAM-Kopie uebersetzt. Die App-Datei wird nicht umgeschrieben. Andere
Speicherlayouts oder erkennbare unbekannte ROM-Aufrufe werden abgewiesen.
Eigene Hardwarezugriffe und abweichende Aufrufkonventionen einer alten App
koennen weiterhin inkompatibel sein. Das Frontend aendert die Eingaben
innerhalb einer gestarteten App nicht.

## Pruefstand

Der SuperH-Build und **50 Pruefungen** sind bestanden:

- 21 fuer UI-Zustandsautomat und Renderer, 4 fuer die echte gui.cpp mit
  simulierten SDK-Eingaben, 17 fuer den HHK2-Parser.
- 6 fuer den echten Abschlussweg samt OS-Pruefung und Rueckfallpfad.
- 2 fuer die Maschinenbefehle des Casio-Aufrufers und den kompilierten
  SuperH-Rueckgabeadapter: Der neue Adapter erreicht die normale Fortsetzung;
  die alte Null-Rueckgabe erzeugt den erwarteten ungueltigen Zugriff bei +0x4c.

Die letzten beiden Pruefungen verwenden einen eng begrenzten
SuperH-Interpreter. Sie simulieren nicht den gesamten Taschenrechner.
Details und genaue Grenzen stehen in TEST-REPORT.txt.

**Diese 0.4-Fassung wurde hier nicht am echten Taschenrechner ausgefuehrt.**
Ein erfolgreicher Hardwaretest von Schliessen, Wiedereroeffnen und
App-Rueckkehr steht noch aus. preview/ zeigt die Ausgabe desselben Renderers
am PC, keine Aufnahme eines Taschenrechners.

## Zurueckwechseln

Unter fallback/ liegen:

- **dual-0.2-run.bin:** die zuvor gelieferte gemeinsame HHK2/HH3-Version.
- **yal-original-run.bin:** der urspruengliche YAL fuer OS 7002.

Version 0.3 mit dem bekannten Fehler ist nicht als Rueckfallversion enthalten.
Zum Zurueckwechseln die gewuenschte Datei auf dem PC in **run.bin** umbenennen
und ins Hauptverzeichnis kopieren. Alternativ die selbst gesicherte Datei
verwenden. Wenn der Launcher haengt, und kein Update oder Datentransfer
laeuft, kann ein kurzer Druck auf RESTART hinten den Rechner neu starten;
ungespeicherte RAM-Daten koennen dabei verloren gehen.

## Quellcode und Build

source/ enthaelt das Frontend, beide App-Lader, Tests, die zugehoerigen
SDK-Quellen und die Lizenz der verwendeten Schrift. debug/ enthaelt die zum
gelieferten Build gehoerende ELF-Datei fuer eine spaetere PC-Fehlerzuordnung.

Hosttests brauchen Python 3 und g++ mit C++23:

    cd source
    python3 tests/test_classic.py
    python3 tests/test_legacy.py
    python3 tests/test_exit.py

Fuer den Calculator-Build muss sh4a_nofpueb-elf-g++ 15.2.0 im PATH liegen und
SDK_DIR auf das gebaute HHK3-SDK zeigen. Das in BUILD.json mit Digest
genannte Toolchain-Image stellt diese Werkzeuge und /sdk bereit:

    sh build.sh

Das Ergebnis ist dist/run.7002.bin. Im Taschenrechner muss es run.bin
heissen. Der normale Build verwendet die enthaltene Bitmap-Schrift und
braucht weder Pillow noch uGFX. Zum Regenerieren der Schrift mit Pillow:

    python3 generate_classic_font.py assets/DejaVuSans.ttf src/classic/font.hpp

Grundlagen:
https://github.com/ClasspadDev/yal
https://github.com/ClasspadDev/hollyhock-3
https://github.com/SnailMath/hollyhock-2

Code unter GPL-3.0, siehe LICENSE-GPL-3.0.txt und die jeweiligen Quellen.
Fuer die Schrift gilt source/src/classic/FONT-LICENSE.txt.
Es werden keine Casio-Firmwaredateien verteilt.
