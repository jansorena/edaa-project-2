#ifndef FMINDEX
#define FMINDEX

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/util.hpp>

class fmindex {
private:
    sdsl::csa_wt<sdsl::wt_huff<sdsl::rrr_vector<127> >, 512, 1024> fm_index;
    std::string t;
    std::string _t;
public:
    fmindex(const std::string& text) {
        char ETX = 3;
        _t = text + ETX;
        t = _t;
        sdsl::construct_im(fm_index, t, 1);
    }

    // Contar ocurrencias de un patrón
    size_t count(const std::string& pattern) const {
        size_t occs = sdsl::count(fm_index, pattern.begin(), pattern.end());
    }

    // Obtener el tamaño en bytes de la estructura
    size_t size_in_bytes() const {
        return sdsl::size_in_bytes(fm_index);
    }

};

#endif