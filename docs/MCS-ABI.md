# MCS-Typausgabe unter OS 7002

Das SDK ist für den Taschenrechner entwickelt. Der direkte Aufruf von
`MCS_GetVariableU` in `recent.cpp` umgeht eine konkrete Typabweichung des
gepinnten Wrappers; er ist keine Anpassung an den PC.

Geprüfte Kombination:

- SDK: `ClasspadDev/hollyhock-3`, Commit `1e483e395391d50d9bd2d21cee4beecc7589fb1c`.
- Zielcompiler: `sh4a_nofpueb-elf-g++` 15.2.0, Big Endian, normale Enum-Größe 4 Bytes.
- OS: `02.01.7002.0001`, Guard `2023.0209.1500`.

In `sdk/include/sdk/os/mcs.h` nimmt `MCS_GetVariableU` einen `uint8_t *` entgegen.
Der Inline-Wrapper `MCS_GetVariable` wandelt dagegen einen
`MCS_VariableType *` in diesen Byte-Zeiger um. Die Firmware schreibt genau ein
Byte. Beim zuvor auf null gesetzten vier Byte großen Enum entsteht dadurch
`0x05000000` statt `VARTYPE_STR == 0x05`. Ein `uint8_t` liefert den richtigen Wert.

Der Firmwarebefund stammt aus der lokalen Originaldatei mit SHA256
`62c95f96df2c238282a4b208b9a970dbfbfd53c4aa70b938ae5cbb9c4cd06973`.
`MCS_GetVariable` beginnt bei `0x800776be`; bei `0x8007773e` schreibt
`mov.b r1,@r7` den Typ an den übergebenen Zeiger. Auch die Initialisierung
bei `0x800776c8` verwendet einen Byte-Schreibzugriff.

`make target-test` kompiliert `tests/mcs_abi_probe.cpp` mit dem Zielcompiler.
Die Prüfung liest die tatsächlich erzeugten SH-Rückgabewerte für den
synthetischen Byte-Schreibzugriff auf Enum und Byte. Sie benötigt keine
Firmwaredatei und führt keine vollständige OS- oder Hardware-Emulation aus.
Die Hosttests prüfen zusätzlich den produktiven Verlaufscode mit MCS-Stubs.

Bei einem SDK- oder Toolchain-Update ist dieser Befund neu zu prüfen. Der
Upstream-Code bleibt unverändert; die Byte-Deklaration stammt bereits aus dem SDK.
