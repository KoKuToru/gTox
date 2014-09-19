#include <iostream>
#include <fstream>
#include <algorithm>
#include <cctype>

std::string to_upper(std::string input) {
    for(size_t i = 0; i < input.size(); ++i) {
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
        std::cout << "Usage: " << argv[0] << " <namespace> <output> <input1> <input2> <inputN>" << std::endl;
        return -1;
    }

    std::ofstream header(argv[2] + std::string(".h"));
    std::ofstream source(argv[2] + std::string(".cpp"));

    if (!header.is_open()) {
        std::cerr << "Couldn't open " << argv[1] << ".h" << std::endl;
        return -3;
    }

    if (!source.is_open()) {
        std::cerr << "Couldn't open " << argv[1] << ".cpp" << std::endl;
        return -4;
    }

    header << "#ifndef " << to_upper(argv[1]) << "_H" << std::endl;
    header << "#define " << to_upper(argv[1]) << "_H" << std::endl << std::endl;
    header << "#include <string>" << std::endl << std::endl;
    header << "namespace " << argv[1] << " {" << std::endl;

    source << "#include \"" << basename(argv[2]) << ".h\"" << std::endl << std::endl;

    for(int i = 3; i < argc; ++i) {
        std::ifstream input(argv[i]);
        if (!input.is_open()) {
            std::cerr << "Couldn't open " << argv[i] << std::endl;
            return -2;
        }

        header << "    extern std::string " << remove_ext(basename(argv[i])) << ";" << std::endl;

        source << "std::string " << argv[1] << "::" << remove_ext(basename(argv[i])) << " = " << std::endl
               << "R\"rawstring(";

        std::string line;
        while(std::getline(input, line)) {
            source << line << std::endl; //not 100% correct last line might have no newline
        }

        source << ")rawstring\";" << std::endl << std::endl;
    }

    header << "}" << std::endl;
    header << "#endif" << std::endl;
}
