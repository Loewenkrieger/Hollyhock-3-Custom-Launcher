# Build und Tests

Der Launcher-Code liegt unter `src/`. Das unveränderte HH3-SDK ist als Submodule
unter `meta/hollyhock-3` auf Commit `1e483e395391d50d9bd2d21cee4beecc7589fb1c`
festgesetzt. Die Makefile liest `versions/v02.01.7002.0001/addresses.json` direkt
aus diesem Submodule; eine zweite Kopie oder ein Windows-Symlink ist nicht nötig.
Der Build benötigt weder Pillow noch uGFX.

## Quellen holen

```sh
git clone --recurse-submodules https://github.com/Loewenkrieger/Hollyhock-3-Custom-Launcher.git
cd Hollyhock-3-Custom-Launcher
```

Bei einem bestehenden Checkout genügt `git submodule update --init`.
Das vollständige Releasepaket enthält die benötigten SDK-Quellen bereits unter
`source/meta/hollyhock-3`; dort zuvor nach `source/` wechseln. Die automatisch von
GitHub angehängten Source-Archive enthalten keine ausgefüllten Submodules.

## Hosttests

Benötigt werden Linux, Python 3 und g++ mit C++23 sowie AddressSanitizer und
UndefinedBehaviorSanitizer:

```sh
make test
```

Die sechs Hosttests unter `tests/` lassen sich auch einzeln starten.
Sie prüfen Oberfläche, Eingaben, HH3-App-Suche, Metadaten, Verlauf, Startablauf
und den Rückgabeweg. Die Ausgabe wird zusätzlich in `dist/tests-host.json`
gespeichert. Messmethode und Ergebnisse stehen unter [PERFORMANCE.md](PERFORMANCE.md).

## Calculator-Build

`sh4a_nofpueb-elf-g++` 15.2.0 und die zugehörigen Werkzeuge müssen im PATH liegen.
Die Pipeline verwendet das festgesetzte Image aus `tools/release.json`.
In diesem Container oder mit derselben lokal verfügbaren Toolchain:

```sh
make -j2 launcher smoke
make target-test
make package
```

Das SDK wird automatisch gebaut. `SDK_DIR` kann bei Bedarf überschrieben werden;
die Pipeline setzt es ausdrücklich auf `meta/hollyhock-3/sdk`.

Ergebnisse: `dist/run.7002.bin`, `dist/run.7002.elf` und
`smoke/hh3/launcher-test.hh3`. Auf dem Taschenrechner muss die Launcher-Datei
`run.bin` heißen. `make target-test` prüft den kompilierten Rückgabeadapter sowie
die [MCS-Typausgabe](MCS-ABI.md) und schreibt `dist/tests-target.json`.

`make package` setzt erfolgreiche Host- und Zielprüfungen für die aktuellen
Quellen und das aktuelle Binary voraus. Unter `dist/release/` entstehen
`run.bin`, das Komplettpaket mit SDK-Quellen, `BUILD.json` und `SHA256SUMS.txt`.
Die Metadaten enthalten Quellrevisionen, Compiler, Prüfsummen und Teststatus.
`make clean` entfernt Launcher- und Test-App-Builddateien; das SDK lässt sich
mit `make -C meta/hollyhock-3/sdk clean` separat bereinigen.

## GitHub Actions und Releases

`.github/workflows/build.yml` läuft bei Pushes auf `main`, Pull Requests und
manuellem Start. Nach Hosttests, SH-Build, Zielprüfungen und Paketprüfung stehen
die Dateien als Actions-Artefakt bereit. Bei einem Versionstag wie `v0.5.0`
werden sie zusätzlich als GitHub Release veröffentlicht. Der Tag muss dem
Eintrag `tag` in `tools/release.json` entsprechen. Nur dieser Release-Job hat
Schreibrechte; Pull Requests veröffentlichen nichts.

## Schrift neu erzeugen

Nur zum Neuerzeugen der Schrift wird Pillow benötigt:

```sh
python3 generate_classic_font.py assets/DejaVuSans.ttf src/classic/font.hpp
```

## Verlaufsformat

Der Launcher verwendet ausschließlich den MCS-Ordner `hhlaunch` und die
String-Variable `recent`. Der auf 512 Bytes begrenzte Eintrag beginnt mit
`HH3REC1`, enthält höchstens fünf Dateipfade und endet mit einem NUL-Byte.
Fehler beim Lesen oder Schreiben beeinflussen den App-Start nicht. Geschrieben
wird einmal nach erfolgreichem Laden einer App, vor deren Ausführung.
