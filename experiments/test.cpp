#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "../src/suffix_array.cpp"
#include "../src/suffix_array_lcp.cpp"
#include "../src/fmindex.cpp"

std::string load_text(const std::string& filename) {
    std::ifstream infile(filename);
    std::stringstream buffer;
    buffer << infile.rdbuf();
    return buffer.str();
}

int main() {
    // Suffix array
    std::string text = load_text("dna.50MB");

    //std::string text = "alaba_a_la_alabarda_abalaba_la_alabarda";

    //suffix_array sa(text);

    
    //std::cout << sa.count("a") << std::endl;
    
    //suffix_array_lcp salcp(text);
    //std::cout << salcp.count("a") << std::endl;

    FMIndex fmi(50);
    fmi.build(text);
     std::cout << fmi.count("ACGT") << std::endl;
    // g++ -o fmindex main.cpp fmindex.cpp bwt.cpp suffixtree.cpp -std=c++17
    return 0;
}