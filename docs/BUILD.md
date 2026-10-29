# Build und Tests

Der Launcher-Code liegt unter `src/`, das HH3-SDK unter `sdk/`. Der Build verwendet die mitgelieferte Bitmap-Schrift und benötigt weder Pillow noch uGFX.

## Hosttests

Benötigt werden Linux, Python 3 und g++ mit C++23 sowie AddressSanitizer und UndefinedBehaviorSanitizer:

```sh
python3 tests/test_classic.py
python3 tests/test_discovery.py
python3 tests/test_loader.py
python3 tests/test_recent.py
python3 tests/test_launch.py
python3 tests/test_exit.py
```

Die Tests prüfen die Oberfläche, Eingaben, HH3-App-Suche, Metadaten, Verlauf, Startablauf und den Rückgabeweg. Sie ersetzen keinen Test am Taschenrechner. Messmethode und Ergebnisse stehen unter [PERFORMANCE.md](PERFORMANCE.md).

## Calculator-Build

`sh4a_nofpueb-elf-g++` 15.2.0 und die zugehörigen Werkzeuge müssen im PATH liegen. Toolchain-Image und Quellrevisionen stehen in `downloads/v0.5/BUILD.json`, im Installationspaket in der obersten Ebene.

```sh
make -C sdk
export SDK_DIR="$(pwd)/sdk"
sh build.sh
sh build-smoke.sh
```

Im Installationspaket zuvor nach `source/` wechseln. Ergebnisse: `dist/run.7002.bin` und `smoke/hh3/launcher-test.hh3`. Die Launcher-Datei muss auf dem Taschenrechner `run.bin` heißen.

Zusätzliche Prüfung des kompilierten Rückgabeadapters:

```sh
python3 tests/test_exit.py --target-elf dist/run.7002.elf --target-bin dist/run.7002.bin
```

## Schrift neu erzeugen

Nur zum Neuerzeugen der Schrift wird Pillow benötigt:

```sh
python3 generate_classic_font.py assets/DejaVuSans.ttf src/classic/font.hpp
```

## Verlaufsformat

Der Launcher verwendet ausschließlich den MCS-Ordner `hhlaunch` und die String-Variable `recent`. Der auf 512 Bytes begrenzte Eintrag beginnt mit `HH3REC1`, enthält höchstens fünf Dateipfade und endet mit einem NUL-Byte. Fehler beim Lesen oder Schreiben beeinflussen den App-Start nicht. Geschrieben wird einmal nach erfolgreichem Laden einer App, vor deren Ausführung.

Die Implementierung nutzt die Byte-Typdeklaration `MCS_GetVariableU`, um die Typausgabe auch auf dem Big-Endian-System korrekt auszulesen. Grundlage: [MCS-SDK-Dokumentation](https://classpad.dev/wiki/hh3/reference/os/mcs/).
