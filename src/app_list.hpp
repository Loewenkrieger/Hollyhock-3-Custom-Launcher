// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "loaders/interface.hpp"

std::forward_list<std::unique_ptr<Executable>> discover_apps();
