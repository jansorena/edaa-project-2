#ifndef SUFFIX_ARRAY_LCP
#define SUFFIX_ARRAY_LCP

#include <algorithm>
#include <cstdint>
#include <string>
#include <vector>
#include <iostream>

class suffix_array_lcp
{
private:
    std::string _t;
    std::string_view t;
    std::vector<std::int64_t> SA;
    std::vector<std::int64_t> LCP;
    std::vector<std::int64_t> rank;

public:
    suffix_array_lcp(const std::string &text)
    {
        // Add lexicographically minimal char at end of text
        // Done to properly compare suffixes
        char ETX = 3;
        _t = text + ETX;
        t = _t;

        std::int64_t n, sigma, one, i, j, k;
        n = t.length();
        sigma = 256; // Size of alphabet, ASCII for now
                     // If changed, must implement key function that maps
                     // symbols to integers in range 0..sigma uniquely
        one = 1;

        SA.resize(n);
        std::vector<std::int64_t> count(std::max(sigma, n), 0);
        std::vector<std::int64_t> p(n); // For shifted indices
        std::vector<std::int64_t> q(n); // Helper for rank
        std::vector<std::int64_t> r(n); // For ranks

        // Counting sort substrings of length 1
        for (i = 0; i < n; i++)
            count[static_cast<unsigned char>(t[i])]++;
        for (i = 1; i < sigma; i++)
            count[i] += count[i - 1];
        for (i = n - 1; i >= 0; i--)
            SA[--count[static_cast<unsigned char>(t[i])]] = i;

        // Set up ranks by comparing pairs and increasing by one if different
        r[SA[0]] = 0;
        j = 0;
        for (i = 1; i < n; i++) {
            if (t[SA[i - 1]] != t[SA[i]])
                j++;
            r[SA[i]] = j;
        }

        for (k = 0; (one << k) < n; k++) {
            // Find cyclic shifted index
            for (i = 0; i < n; i++) {
                p[i] = SA[i] - (one << k);
                if (p[i] < 0)
                    p[i] += n;
            }

            // Sort again using radix sort
            // This is just a counting sort, but works as a faster
            // radix sort because of the shifting hack
            // We sort first with second half and then first half,
            // but only once, so it is faster
            for (i = 0; i <= j; i++)
                count[i] = 0;
            for (i = 0; i < n; i++)
                count[r[p[i]]]++;
            for (i = 1; i <= j; i++)
                count[i] += count[i - 1];
            for (i = n - 1; i >= 0; i--)
                SA[--count[r[p[i]]]] = p[i];

            // Recompute ranks
            q[SA[0]] = 0;
            j = 0;
            for (i = 1; i < n; i++) {
                // Check if first half or second half differ
                if (r[SA[i - 1]] != r[SA[i]] ||
                    r[(SA[i - 1] + (one << k)) % n] != r[(SA[i] + (one << k)) % n])
                    j++;

                q[SA[i]] = j;
            }

            for (i = 0; i < n; i++)
                r[i] = q[i];
        }

        // LCP construction using Kasai's algorithm
        LCP.resize(n);
        rank.resize(n);
        for (i = 0; i < n; i++)
            rank[SA[i]] = i;

        std::int64_t h = 0;
        for (i = 0; i < n; i++) {
            if (rank[i] > 0) {
                j = SA[rank[i] - 1];
                while (i + h < n && j + h < n && t[i + h] == t[j + h])
                    h++;
                LCP[rank[i]] = h;
                if (h > 0)
                    h--;
            } else {
                LCP[rank[i]] = 0;
            }
        }
    }

    std::int64_t count(const std::string_view s)
    {
        if (s.length() > t.length())
            return 0;

        std::int64_t n = t.length();
        std::int64_t lcp_lo = 0, lcp_hi = 0;
        
        // First binary search: find the first occurrence
        std::int64_t lo = 0, hi = n;
        while (lo < hi) {
            std::int64_t mi = lo + (hi - lo) / 2;
            std::string_view suffix = t.substr(SA[mi]);
            // Compare only up to pattern length for efficiency
            int res = suffix.compare(0, s.length(), s);
            
            if (res == 0) {
                // Found a match - now walk backwards through LCP array
                // to find the first occurrence where LCP becomes smaller than pattern
                lcp_lo = mi;
                while (lcp_lo > 0 && LCP[lcp_lo] >= s.length()) {
                    lcp_lo--;
                }
                break;
            } else if (res < 0) {
                lo = mi + 1;
            } else {
                hi = mi;
            }
        }
        
        // If we didn't find any exact match in the binary search
        std::int64_t first_occurrence = lcp_lo == 0 ? lo : lcp_lo;
        
        // Second binary search: start from where we left off
        // to find the last occurrence
        lo = first_occurrence;  // Start from where we found first match
        hi = n;
        while (lo < hi) {
            std::int64_t mi = lo + (hi - lo) / 2;
            std::string_view suffix = t.substr(SA[mi]);
            int res = suffix.compare(0, s.length(), s);
            
            if (res == 0) {
                // Found a match - now walk forward through LCP array
                // to find the last occurrence where LCP becomes smaller than pattern
                lcp_hi = mi;
                while (lcp_hi < n - 1 && LCP[lcp_hi + 1] >= s.length()) {
                    lcp_hi++;
                }
                break;
            } else if (res < 0) {
                lo = mi + 1;
            } else {
                hi = mi;
            }
        }
        
        // If we didn't find any exact match in the second binary search
        std::int64_t last_occurrence = lcp_hi == 0 ? hi : lcp_hi + 1;
        
        return last_occurrence - first_occurrence;
    }

    std::int64_t& operator[](std::int64_t i)
    {
        return SA[i];
    }

    
    std::int64_t memory_usage() const
    {
        std::int64_t total_memory = 0;

        // SA size
        total_memory += sizeof(std::int64_t) * SA.size();

        // LCP size
        total_memory += sizeof(std::int64_t) * LCP.size();


        return total_memory;
    }

    void print_lcp() {
        for (std::int64_t i = 0; i < LCP.size(); i++) {
            std::cout << LCP[i] << " ";
        }
        std::cout << std::endl;
    }
    
};

#endif
