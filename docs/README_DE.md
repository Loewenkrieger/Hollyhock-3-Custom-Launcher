# Hollyhock 3 Custom Launcher 0.5 — Public

Für ClassPad II mit bereits installiertem Hollyhock 3 auf **OS 02.01.7002.0001**. Diese Variante unterstützt ausschließlich **HH3-Apps (`.hh3`)**.

## Installation

1. Die bisherige `run.bin` auf dem PC sichern.
2. Das Installationspaket entpacken. Die `run.bin` aus dessen oberster Ebene ins Hauptverzeichnis des Taschenrechner-Laufwerks kopieren und dort ersetzen. Im Repository liegt dieselbe Datei unter `downloads/v0.5/run.bin`.
3. HH3-Apps ebenfalls direkt ins Hauptverzeichnis kopieren. Optional die Test-App `smoke/launcher-test.hh3` aus dem Installationspaket verwenden.
4. Sicher auswerfen, USB trennen und den Launcher wie bisher über das System-Menü öffnen.

In der Fußzeile steht **0.5**. Zuerst ohne App-Start mit Close, X oder Clear schließen und erneut öffnen. Danach kann **Launcher test: HH3** gestartet werden; die Test-App wartet auf eine Taste und kehrt zurück.

**Es wird nur `run.bin` ausgetauscht. Kein Firmwareupdate erforderlich.**

## Bedienung

| Eingabe | Funktion |
| --- | --- |
| Programmauswahl antippen | Dropdown öffnen oder schließen |
| Eintrag im Dropdown antippen | App wählen und Dropdown schließen |
| Im langen Dropdown ziehen / Pfeile antippen | Liste scrollen |
| Run | Gewählte App starten |
| Close oder X oben rechts | Launcher schließen |
| Pfeil hoch / runter | Vorherige / nächste App wählen |
| EXE bei geschlossenem Dropdown | Gewählte App starten; bei Fokus auf Close schließen |
| EXE bei geöffnetem Dropdown | Auswahl bestätigen, ohne die App sofort zu starten |
| Pfeil links / rechts bei geschlossenem Dropdown | Fokus zwischen Auswahl, Run und Close wechseln |
| Pfeil links / rechts bei geöffnetem Dropdown | Eine Seite zurück / weiter |
| Clear oder Backspace | Erst Dropdown schließen, sonst Launcher schließen |

Die klassische Anordnung und die Eingabeverarbeitung bleiben erhalten. Kurze Touch-Eingaben werden anhand des Berührungsbeginns erfasst; ungültige Koordinaten beim Loslassen verwerfen sie nicht. Die Schrift basiert auf DejaVu Sans.

## Apps und Rückkehr zu Casio

Dateien werden direkt in `\fls0\` gesucht, nicht in Unterordnern. HHK2-Apps (`.hhk`) und alte `.bin`-Apps werden nicht aufgelistet. Eine Umbenennung in `.hh3` konvertiert eine alte App nicht. Die Eingaben innerhalb gestarteter Apps werden von den Apps selbst verarbeitet.

Der HH3-ELF-Loader bleibt gegenüber der bisherigen Version unverändert. Der HHK2-Loader samt Adressübersetzung und altem SDK ist vollständig entfernt.

Der Rückgabeadapter aus Version 0.4 bleibt erhalten: Nach erfolgreichem Beenden liefert er für den geprüften OS-7002-Aufrufer einen gültigen Abschlusswert. Dadurch kann Casio ohne zusätzliches Abschlussfenster zur vorherigen Ansicht zurückkehren, normalerweise System. Die Main-Anwendung wird nicht automatisch geöffnet.

Die Aufrufstelle wird anhand ihrer Maschinenbefehle geprüft. Bei einem anderen Aufrufer bleibt der native Abschlussdialog erhalten; Fehlercodes werden weiterhin angezeigt. Bildschirmwiederherstellung und Laufzeitbereinigung bleiben erhalten.

## Prüfstand und Zurückwechseln

**36 automatisierte Prüfungen und der SuperH-Build sind bestanden. Diese Version wurde hier nicht am echten Taschenrechner ausgeführt.** Die Tests prüfen UI, App-Suche und Rückgabeadapter mit simulierten Schnittstellen beziehungsweise einem begrenzten Instruktionsinterpreter. Details stehen in `TEST-REPORT.txt`.

Zum Zurückwechseln die zuvor gesicherte `run.bin` zurückkopieren. Das öffentliche Paket enthält ausschließlich die neue HH3-Variante.

## Paketinhalt und Quellcode

Das Installationspaket enthält `run.bin`, diese Anleitung, Prüfbericht, `BUILD.json`, SHA256-Prüfsummen, eine HH3-Test-App, Renderer-Vorschauen, die passende Debug-ELF-Datei und vollständigen Quellcode unter `source/`.

Im Repository liegt der Quellcode direkt unter `src/`. Hosttests:

```sh
python3 tests/test_classic.py
python3 tests/test_discovery.py
python3 tests/test_exit.py
```

Für den Calculator-Build `sh4a_nofpueb-elf-g++` 15.2.0 im PATH bereitstellen und `SDK_DIR` auf das gebaute HH3-SDK setzen. Das in `BUILD.json` genannte Toolchain-Image enthält diese Werkzeuge und ein gebautes SDK.

```sh
sh build.sh
sh build-smoke.sh
```

Im Installationspaket zuvor nach `source/` wechseln. Ergebnis: `dist/run.7002.bin`. Auf dem Taschenrechner muss die Datei `run.bin` heißen.

Der normale Build benötigt weder Pillow noch uGFX. Schrift neu erzeugen, falls gewünscht:

```sh
python3 generate_classic_font.py assets/DejaVuSans.ttf src/classic/font.hpp
```

Code unter GPL-3.0; Schriftlizenz unter `src/classic/FONT-LICENSE.txt` beziehungsweise `source/src/classic/FONT-LICENSE.txt` im Installationspaket. Grundlagen: ClasspadDev/yal und ClasspadDev/hollyhock-3.
