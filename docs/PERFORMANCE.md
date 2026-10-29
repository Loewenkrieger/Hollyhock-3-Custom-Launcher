# Optimierungen der Version 0.5

## Weniger Arbeit beim Öffnen

- Die Sortierung vergleicht vorhandene Pfade, statt bei jedem Vergleich zwei neue Pfadkopien anzulegen.
- Für die App-Liste werden alle Namen eingelesen. Version, Autor und Beschreibung werden erst für die ausgewählte App kopiert und danach wiederverwendet.
- Die Listen reservieren ihren Speicher einmal passend zur Anzahl der Apps.
- ELF-Notizsegmente werden in einem Stück gelesen. Namen und Beschreibungen verweisen auf diesen Puffer; eigene Speicherblöcke pro Notiz entfallen.

## Vergleich der Dateizugriffe

Gemessen mit `tests/test_loader.py`: dieselbe native ELF32-Testdatei mit vier Notizen für Name, Version, Autor und Beschreibung. Der Test verwendet den echten Metadatenparser und zählt dessen Aufrufe von `fopen`, `fread` und `fseek` über Linker-Wrapper. Die Vergleichsbasis ist Commit `11ebaaa`.

| Aufruf beim ersten Einlesen | Vorher | Jetzt |
| --- | ---: | ---: |
| Datei öffnen (`fopen`) | 1 | 1 |
| Lesen (`fread`) | 14 | 3 |
| Positionieren (`fseek`) | 11 | 4 |
| Weitere Dateioperationen beim erneuten Abfragen | 0 | 0 |

Reproduktion mit einem vorhandenen Checkout der Vergleichsbasis:

```sh
python3 tests/test_loader.py --source /path/to/baseline --measure-only
python3 tests/test_loader.py --measure-only
```

Ein zusätzlicher Test der echten Oberfläche öffnet eine Liste mit **1.000 Apps**. Alle Namen bleiben verfügbar; beim Öffnen werden nur die drei Detailfelder der ersten ausgewählten App abgefragt. Beim erneuten Auswählen bereits angezeigter Apps werden diese Felder wiederverwendet.

Das sind Zählungen von Dateioperationen und Metadatenabfragen auf dem PC. Sie belegen weniger Arbeit, aber noch keine bestimmte Beschleunigung in Sekunden am ClassPad. Der Lader für App-Code und die Cache-Behandlung beim Programmstart bleiben erhalten.

## Verlauf

Höchstens fünf Einträge werden vor die übrige Liste gestellt. Die Zuordnung verwendet die Dateipfade, sodass gleichnamige Apps nicht verwechselt werden. Es gibt keine Schreibzugriffe beim Navigieren; ein Verlaufseintrag wird nach erfolgreichem Laden einer App gespeichert. Fehlende Apps und fehlerhafte Verlaufsdaten verhindern die normale Nutzung nicht.
