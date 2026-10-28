#pragma once
#include "../interface.hpp"
#include "legacy/image.hpp"
#include <optional>

class LegacyLoader : public FileBasedExecutable<".hhk"> {
  bool inspected = false;
  std::string name, author, version, description;
  std::optional<legacy::Image> image;
  legacy::Image read_image();
  void inspect();
public:
  using FileBasedExecutable::FileBasedExecutable;
  void load() override;
  void unload() override { image.reset(); }
  int execute() override;
  std::unique_ptr<char[]> getName() override;
  std::unique_ptr<char[]> getAuthor() override;
  std::unique_ptr<char[]> getVersion() override;
  std::unique_ptr<char[]> getDescription() override;
};
