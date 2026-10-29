# Hollyhock 3 Custom Launcher — Public

Klassischer Launcher für den **Casio ClassPad II**, mit der Oberfläche im HHK2-Stil und ausschließlich **HH3-Apps (`.hh3`)**.

Version **0.5 Public**, für bereits installiertes Hollyhock 3 auf **OS 02.01.7002.0001**.

## Download und Installation

- [Installationspaket 0.5](downloads/v0.5/hhk3-public-launcher-0.5-os7002.zip)
- [run.bin](downloads/v0.5/run.bin)
- [Deutsche Anleitung](docs/README_DE.md)
- [Prüfbericht](docs/TEST-REPORT.txt)

Die vorhandene `run.bin` sichern und die neue Datei ins Hauptverzeichnis des Taschenrechner-Laufwerks kopieren. Apps ebenfalls direkt dort als `.hh3` ablegen. Sicher auswerfen, USB trennen und den Launcher wie bisher öffnen. **Kein Firmwareupdate erforderlich.**

## Funktionen

- Klassische Oberfläche mit Dropdown, Run, Close und Programminformationen.
- Direkte Touch-Ereignisverarbeitung sowie Pfeil-, EXE- und Clear-Tasten.
- HH3-ELF-Loader; `.hhk`- und alte `.bin`-Apps erscheinen nicht in der Auswahl.
- Rückkehr zur vorherigen Casio-Ansicht beim normalen Beenden über den geprüften OS-7002-Aufrufer. Bei verändertem Aufrufer bleibt der native Abschlussdialog erhalten.

![Vorschau des Renderers](docs/preview/classic-closed.png)

## Varianten

Die **Public-Version 0.5** enthält ausschließlich den HH3-Loader. HHK2-Loader, altes SDK, Adressübersetzung, alte Test-Apps und frühere Dual-Binärdateien gehören nicht zu diesem Repository oder den Downloadpaketen.

Die bisherige **Private-Version 0.4** mit HH3- und experimenteller HHK2-Unterstützung wird separat aufbewahrt. Das bestehende private Repository bleibt privat. [Details zur Trennung](docs/VARIANTS.md).

## Prüfstand

**36 automatisierte Prüfungen bestehen:** 25 für Oberfläche/Eingaben, 3 für die HH3-App-Suche und 8 für den Rückgabeweg einschließlich einer eng begrenzten SuperH-Instruktionssimulation. Der SuperH-Build und die mitgelieferte HH3-Test-App lassen sich kompilieren.

**Version 0.5 wurde hier nicht am echten Taschenrechner getestet.** Die Tests führen keine gestarteten Apps auf Calculator-Hardware aus. Die Vorschauen stammen vom PC-Renderer.

## Entwicklung

Quellcode liegt unter `src/`, das HH3-SDK unter `sdk/`. Der normale Build verwendet die mitgelieferte Bitmap-Schrift.

Hosttests benötigen Python 3 und g++ mit C++23:

```sh
python3 tests/test_classic.py
python3 tests/test_discovery.py
python3 tests/test_exit.py
```

Calculator-Build: `sh4a_nofpueb-elf-g++` 15.2.0 im PATH, `SDK_DIR` auf das gebaute HH3-SDK setzen:

```sh
sh build.sh
sh build-smoke.sh
```

Ergebnisse: `dist/run.7002.bin` und `smoke/hh3/launcher-test.hh3`. Für den Taschenrechner die Launcher-Datei in `run.bin` umbenennen. Das genaue Toolchain-Image und die Quellrevisionen stehen in [BUILD.json](downloads/v0.5/BUILD.json).

Das enthaltene SDK kann mit derselben Toolchain über `make -C sdk` gebaut werden; anschließend `SDK_DIR` auf den absoluten Pfad dieses Ordners setzen.

Prüfung des kompilierten Rückgabeadapters:

```sh
python3 tests/test_exit.py --target-elf dist/run.7002.elf --target-bin dist/run.7002.bin
```

## Grundlagen und Lizenz

Basierend auf [ClasspadDev/yal](https://github.com/ClasspadDev/yal) und dem [Hollyhock-3-SDK](https://github.com/ClasspadDev/hollyhock-3). Ursprüngliche Hinweise und Quellrevisionen bleiben erhalten.

Code: [GPL-3.0](LICENSE). Für DejaVu Sans gilt die beiliegende [Schriftlizenz](src/classic/FONT-LICENSE.txt). Dieses Repository enthält keine Casio-Firmware und keinen Firmware-Updater.
