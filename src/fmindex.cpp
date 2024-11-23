#ifndef FMINDEX
#define FMINDEX

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <cassert>

class FMIndex {
private:
    static const char EOS = '\0';
    std::string data;
    std::map<size_t, size_t> offset;
    std::map<char, size_t> occ;
    std::vector<std::map<char, size_t>> checkpoints;
    size_t step;

    // Auxiliary functions for BWT
    std::string bwt_transform(const std::string& s) {
        assert(s.find(EOS) == std::string::npos);
        std::string text = s + EOS;
        std::vector<std::string> rotations;

        for (size_t i = 0; i < text.length(); ++i) {
            rotations.push_back(text.substr(i) + text.substr(0, i));
        }

        std::sort(rotations.begin(), rotations.end());
        std::string result;

        for (const auto& rotation : rotations) {
            result += rotation.back();
        }

        return result;
    }


    std::string bwt_inverse(const std::string& bwt) {
    size_t n = bwt.size();
    std::vector<size_t> count(256, 0);
    std::vector<size_t> rank(n);

    // Calcular las posiciones de los caracteres en el arreglo ordenado (primer columna)
    for (size_t i = 0; i < n; ++i) {
        rank[i] = count[bwt[i]]++;
    }

    std::vector<size_t> first_occ(256, 0);
    for (size_t i = 1; i < 256; ++i) {
        first_occ[i] = first_occ[i - 1] + count[i - 1];
    }

    // Reconstrucción inversa
    std::string result(n - 1, ' ');
    size_t pos = 0;

    for (size_t i = n - 1; i-- > 0;) {
        result[i] = bwt[pos];
        pos = first_occ[bwt[pos]] + rank[pos];
    }

    return result;
    }


    void calc_checkpoints() {
        std::map<char, size_t> counts;
        checkpoints.clear();
        
        for (size_t i = 0; i < data.length(); ++i) {
            if (i % step == 0) {
                checkpoints.push_back(counts);
            }
            counts[data[i]]++;
        }
    }

    size_t count_upto(size_t idx, char c) {
        size_t checkpoint_idx = (idx / step);
        size_t count = 0;
        
        if (checkpoint_idx > 0 && !checkpoints.empty()) {
            count = checkpoints[checkpoint_idx - 1][c];
        }
        
        for (size_t i = checkpoint_idx * step; i < idx; ++i) {
            if (data[i] == c) count++;
        }
        
        return count;
    }

    size_t get_first_occ(char c) const {
        auto it = occ.find(c);
        return (it != occ.end()) ? it->second : 0;
    }

    size_t lf_mapping(size_t idx, char c) {
        return get_first_occ(c) + count_upto(idx, c);
    }

    size_t get_offset(size_t idx) {
        size_t r = 0;
        size_t i = idx;
        
        while (data[i] != EOS) {
            auto it = offset.find(i);
            if (it != offset.end()) {
                r += it->second;
                break;
            }
            r++;
            i = lf_mapping(i, data[i]);
        }
        
        if (offset.find(idx) == offset.end()) {
            offset[idx] = r;
        }
        
        return r;
    }

public:
    FMIndex(size_t checkpoint_step = 50) : step(checkpoint_step) {}

    // Build FM-Index from input text
    void build(const std::string& text) {
        // Transform text using BWT
        data = bwt_transform(text);
        
        // Calculate first occurrences
        std::map<char, size_t> char_count;
        for (char c : data) {
            char_count[c]++;
        }
        
        size_t cumsum = 0;
        for (auto& pair : char_count) {
            size_t count = pair.second;
            occ[pair.first] = cumsum;
            cumsum += count;
        }
        
        // Calculate checkpoints
        calc_checkpoints();
    }

    // Search for pattern in text
    std::vector<size_t> search(const std::string& pattern) {
        if (pattern.empty()) return {};
        
        // Find pattern bounds
        size_t top = 0;
        size_t bottom = data.length();
        
        // Traverse pattern from right to left
        for (auto it = pattern.rbegin(); it != pattern.rend(); ++it) {
            char c = *it;
            top = lf_mapping(top, c);
            bottom = lf_mapping(bottom, c);
            
            if (top >= bottom) return {};
        }
        
        // Collect all occurrences
        std::vector<size_t> positions;
        for (size_t i = top; i < bottom; ++i) {
            positions.push_back(get_offset(i));
        }
        
        std::sort(positions.begin(), positions.end());
        return positions;
    }

    // Count occurrences of pattern in text
    size_t count(const std::string& pattern) {
        if (pattern.empty()) return 0;
        
        size_t top = 0;
        size_t bottom = data.length();
        
        for (auto it = pattern.rbegin(); it != pattern.rend(); ++it) {
            char c = *it;
            top = lf_mapping(top, c);
            bottom = lf_mapping(bottom, c);
            
            if (top >= bottom) return 0;
        }
        
        return bottom - top;
    }
};

#endif