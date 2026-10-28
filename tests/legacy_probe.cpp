#include "legacy/image.hpp"
#include "legacy/sdk_maps.hpp"
#include <fstream>
#include <iostream>
#include <iterator>
#include <vector>

int main(int argc, char **argv) {
  if (argc != 3) return 2;
  std::ifstream input(argv[2], std::ios::binary);
  std::vector<std::uint8_t> bytes((std::istreambuf_iterator<char>(input)), {});
  try {
    auto map = std::string(argv[1]) == "2000" ? std::span(legacy::map_2000) : std::span(legacy::map_7002);
    auto image = legacy::parse(bytes, map);
    std::cout << "OK " << image.name << " PATCHES " << image.patched_words << " SECTIONS " << image.sections.size() << '\n';
    for (auto &s : image.sections) {
      std::cout << std::hex << s.address << ':';
      for (auto byte : s.data) {
        constexpr char digits[] = "0123456789abcdef";
        std::cout << digits[byte >> 4] << digits[byte & 15];
      }
      std::cout << '\n';
    }
    return 0;
  } catch (const std::exception &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }
}
