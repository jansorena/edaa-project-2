#ifndef SDSL_SUFFIX_ARRAY
#define SDSL_SUFFIX_ARRAY

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/util.hpp>
#include <string>
#include <vector>
#include <fstream>

class sdsl_suffix_array {
private:
    sdsl::csa_wt<> csa;  // Compressed suffix array
    std::string t;    // Original text
    std::string _t;   // Text with ETX
public:
    sdsl_suffix_array(const std::string& text) {
        char ETX = 3;
        _t = text + ETX;
        t = _t;
        
        sdsl::construct_im(csa, t, 1); // 1 indica construcción en memoria
    }

    // Contar ocurrencias de un patrón
    size_t count(const std::string& pattern) const {
        return sdsl::count(csa, pattern.begin(), pattern.end());
    }

    // Obtener el tamaño en bytes de la estructura
    size_t size_in_bytes() const {
        return sdsl::size_in_bytes(csa);
    }

};

#endif