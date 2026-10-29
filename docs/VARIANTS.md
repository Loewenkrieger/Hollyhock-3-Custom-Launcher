# Getrennte Varianten

| Variante | Stand | App-Formate | Zweck |
| --- | --- | --- | --- |
| Private | 0.4 | `.hh3` und experimentell `.hhk` | Bestehende persönliche Fassung |
| Public | 0.5 | ausschließlich `.hh3` | Separate Fassung zur Veröffentlichung |

Die Private-Version 0.4 wird durch diese Aufteilung nicht verändert. Ihr Repository und ihre bisherigen Downloadpakete bleiben separat und privat.

Die öffentliche Fassung wird als eigener Quellcodebestand ohne Git-Historie der privaten Fassung bereitgestellt. Für einen Upload ein separates Repository verwenden. Den Inhalt des Ordners `Hollyhock-3-Public-Launcher` aus dem Repository-ZIP dort hochladen und die Ordnerstruktur beibehalten.

Entfernt wurden der HHK2-Loader, der Legacy-ELF-Parser, SDK-Adresszuordnungen, das HHK2-SDK, der alte Binär-App-Loader, die OS-2000-Konfiguration und zugehörige Tests und Test-Apps. Die öffentlichen Archive enthalten keine früheren Dual-Launcher-Binärdateien.

Die HHK2-ähnliche Oberfläche, die Eingabekorrekturen und der Abschlussweg aus 0.4 bleiben Bestandteil der Public-Version. Die Aufteilung veröffentlicht das bestehende private Repository nicht automatisch.
