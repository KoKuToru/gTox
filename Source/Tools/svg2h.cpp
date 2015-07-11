/**
    gTox a GTK-based tox-client - https://github.com/KoKuToru/gTox.git

    Copyright (C) 2014  Luca Béla Palkovics
    Copyright (C) 2014  Maurice Mohlek

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>
**/
#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>

std::string to_upper(std::string input) {
  for (size_t i = 0; i < input.size(); ++i) {
    input[i] = std::toupper(input[i]);
  }
  return input;
}

std::string basename(std::string input) {
  size_t start = input.find_last_of("/\\");
  if (start == std::string::npos) {
    start = 0;
  } else {
    start += 1;
  }
  return input.substr(start);
}

std::string remove_ext(std::string input) {
  return input.substr(0, input.find_last_of('.'));
}

int main(int argc, const char* argv[]) {
  if (argc < 4) {
    std::cout << "Usage: " << argv[0]
              << " <namespace> <output> <input1> <input2> <inputN>\n";
    return -1;
  }

  std::ofstream header(argv[2] + std::string(".h"));
  std::ofstream source(argv[2] + std::string(".cpp"));

  if (!header.is_open()) {
    std::cerr << "Couldn't open " << argv[1] << ".h\n";
    return -3;
  }

  if (!source.is_open()) {
    std::cerr << "Couldn't open " << argv[1] << ".cpp\n";
    return -4;
  }

  header << "#ifndef " << to_upper(argv[1]) << "_H\n";
  header << "#define " << to_upper(argv[1]) << "_H\n\n";
  header << "#include <string>\n\n";
  header << "#include <map>\n\n";
  header << "#include <gtkmm.h>\n\n";
  header << "namespace " << argv[1] << " {\n";
  header << "    Glib::RefPtr<Gdk::Pixbuf> load_icon(const std::string& data);"
         << "\n";

  source << "#include \"" << basename(argv[2]) << ".h\"\n\n";
// clang-format off
  source << R"rawstring(
      Glib::RefPtr<Gdk::Pixbuf> ICON::load_icon(const std::string & data) {
        static std::map<const std::string*, Glib::RefPtr<Gdk::Pixbuf> > cache;
        auto cache_iter = cache.find(&data);

        if (cache_iter != cache.end()) {
          return cache_iter->second;
        }

        auto tmp = Gio::MemoryInputStream::create();
        tmp->add_data(data);
        return cache.insert({&data, Gdk::Pixbuf::create_from_stream(tmp)})
            .first->second;
      })rawstring";
// clang-format on
    source << "\n\n";

  for (int i = 3; i < argc; ++i) {
    std::ifstream input(argv[i]);
    if (!input.is_open()) {
      std::cerr << "Couldn't open " << argv[i] << "\n";
      return -2;
    }

    header << "    extern std::string " << remove_ext(basename(argv[i]))
           << ";\n";

    source << "std::string " << argv[1] << "::" << remove_ext(basename(argv[i]))
           << " = \nR\"rawstring(";

    std::string line;
    while (std::getline(input, line)) {
      source << line
             << "\n";   // not 100% correct last line might have no newline
    }

    source << ")rawstring\";\n\n";
  }

  header << "}\n";
  header << "#endif\n";
}
