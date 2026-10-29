// SPDX-License-Identifier: GPL-3.0-or-later
#include "loaders/elf/loader.hpp"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <iomanip>
#include <iostream>

namespace { int opens = 0, reads = 0, seeks = 0; }
extern "C" {
std::FILE *__real_fopen(const char *, const char *);
std::size_t __real_fread(void *, std::size_t, std::size_t, std::FILE *);
int __real_fseek(std::FILE *, long, int);
std::FILE *__wrap_fopen(const char *path, const char *mode) {
  ++opens; return __real_fopen(path, mode);
}
std::size_t __wrap_fread(void *out, std::size_t size, std::size_t count, std::FILE *file) {
  ++reads; return __real_fread(out, size, count, file);
}
int __wrap_fseek(std::FILE *file, long offset, int origin) {
  ++seeks; return __real_fseek(file, offset, origin);
}
}
// Metadata parsing is real. SH cache instructions and executable hand-off do
// not run in a host process; those methods are not called by this probe.
void ELFLoader::load() { assert(false); }
void ELFLoader::unload() { assert(false); }
int ELFLoader::execute() { assert(false); return 0; }

int main(int argc, char **argv) {
  assert(argc == 3);
  ELFLoader app(argv[1]);
  if (std::strcmp(argv[2], "invalid") == 0) {
    for (int i = 0; i < 2; ++i) {
      bool failed = false;
      try { app.getName(); } catch (const std::exception &) { failed = true; }
      assert(failed);
    }
    return 0;
  }
  auto name = app.getName(), version = app.getVersion();
  auto author = app.getAuthor(), description = app.getDescription();
  const auto initial_reads = reads, initial_seeks = seeks, initial_opens = opens;
  app.getName(); app.getVersion(); app.getAuthor(); app.getDescription();
  assert(reads == initial_reads && seeks == initial_seeks && opens == initial_opens);
  const auto text = [](const auto &value) { return value ? value.get() : ""; };
  std::cout << "{\"name\":" << std::quoted(text(name))
            << ",\"version\":" << std::quoted(text(version))
            << ",\"author\":" << std::quoted(text(author))
            << ",\"description\":" << std::quoted(text(description))
            << ",\"opens\":" << opens << ",\"reads\":" << reads << ",\"seeks\":" << seeks << "}\n";
}
