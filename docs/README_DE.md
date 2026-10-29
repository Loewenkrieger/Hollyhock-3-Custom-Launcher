# Hollyhock 3 Custom Launcher 0.5

Für den Casio ClassPad II mit bereits installiertem Hollyhock 3 auf **OS 02.01.7002.0001**. Der Launcher startet **HH3-Apps (`.hh3`)**.

## Installation

1. Die bisherige `run.bin` auf dem PC sichern.
2. Die neue `run.bin` ins Hauptverzeichnis des Taschenrechner-Laufwerks kopieren und dort ersetzen. Sie liegt im Installationspaket ganz oben, im Repository unter `downloads/v0.5/`.
3. Deine `.hh3`-Apps ebenfalls direkt ins Hauptverzeichnis kopieren. Zum Ausprobieren liegt im Installationspaket `smoke/launcher-test.hh3` bei.
4. Das Laufwerk sicher auswerfen, USB trennen und den Launcher über das System-Menü öffnen.

**Es wird nur `run.bin` ausgetauscht. Ein Firmwareupdate ist nicht nötig.**

In der Fußzeile steht **0.5**. Zuerst den Launcher mit Close, X oder Clear schließen und erneut öffnen. Danach die Test-App **Launcher test: HH3** starten; sie wartet auf eine Taste und kehrt zurück.

## Bedienung

| Eingabe | Funktion |
| --- | --- |
| Programmauswahl antippen | Dropdown öffnen oder schließen |
| Eintrag antippen | App auswählen |
| Im Dropdown ziehen oder Pfeile antippen | Liste scrollen |
| Run | Ausgewählte App starten |
| Close oder X | Launcher schließen |
| Pfeil hoch / runter | Vorherige / nächste App wählen |
| EXE bei geschlossenem Dropdown | App starten; bei Fokus auf Close schließen |
| EXE bei geöffnetem Dropdown | Auswahl bestätigen |
| Pfeil links / rechts bei geschlossenem Dropdown | Fokus zwischen Auswahl, Run und Close wechseln |
| Pfeil links / rechts bei geöffnetem Dropdown | Eine Seite zurück / weiter |
| Clear oder Backspace | Dropdown schließen; sonst Launcher schließen |

Apps werden direkt im Hauptverzeichnis (`\fls0\`) gesucht, nicht in Unterordnern. Die Eingaben innerhalb einer gestarteten App verarbeitet die App selbst.

Beim normalen Beenden kehrt der Launcher zur vorherigen Casio-Ansicht zurück, normalerweise zum System-Menü. Erkennt er einen anderen Systemaufrufer, verwendet er den Casio-Abschlussdialog. Fehler werden weiterhin angezeigt.

## Teststand und Zurückwechseln

Die automatisierten Prüfungen decken Oberfläche, Eingaben, App-Suche und den Rückgabeweg ab. **Ein abschließender Test am echten ClassPad steht noch aus.** Details stehen im Prüfbericht `TEST-REPORT.txt`.

Zum Zurückwechseln die zuvor gesicherte `run.bin` zurückkopieren.

## Paketinhalt

Das Installationspaket enthält `run.bin`, eine HH3-Test-App, diese Anleitung, Prüfbericht, Build-Angaben, SHA256-Prüfsummen, Vorschauen und die Debug-ELF-Datei. Der vollständige Quellcode liegt unter `source/`, die Build-Anleitung unter `source/docs/BUILD.md`.

Code unter GPL-3.0; Schriftlizenz unter `source/src/classic/FONT-LICENSE.txt`. Grundlagen: ClasspadDev/yal und ClasspadDev/hollyhock-3.
