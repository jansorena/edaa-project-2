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
    std::vector<std::int64_t> rank;
    std::vector<std::int64_t> LCP;

public:
    suffix_array_lcp(const std::string &text)
    {
        // Agregar carácter mínimo lexicográfico al final del texto
        char ETX = 3;
        _t = text + ETX;
        t = _t;

        std::int64_t n = t.length();
        std::int64_t sigma = 256; // Tamaño del alfabeto
        std::int64_t one = 1;
        std::int64_t i, j, k;

        SA.resize(n);
        std::vector<std::int64_t> count(std::max(sigma, n), 0);
        std::vector<std::int64_t> p(n); // Índices desplazados
        std::vector<std::int64_t> q(n); // Ayuda para rango
        std::vector<std::int64_t> r(n); // Rangos

        // Ordenación por conteo de subcadenas de longitud 1
        for (i = 0; i < n; i++)
            count[t[i]]++;
        for (i = 1; i < sigma; i++)
            count[i] += count[i - 1];
        for (i = n - 1; i >= 0; i--)
            SA[--count[t[i]]] = i;

        // Configurar rangos
        r[SA[0]] = 0;
        j = 0;
        for (i = 1; i < n; i++) {
            if (t[SA[i - 1]] != t[SA[i]])
                j++;
            r[SA[i]] = j;
        }

        for (k = 0; (one << k) < n; k++) {
            // Encontrar índices cíclicamente desplazados
            for (i = 0; i < n; i++) {
                p[i] = SA[i] - (one << k);
                if (p[i] < 0)
                    p[i] += n;
            }

            // Ordenar de nuevo usando radix sort
            for (i = 0; i <= j; i++)
                count[i] = 0;
            for (i = 0; i < n; i++)
                count[r[p[i]]]++;
            for (i = 1; i <= j; i++)
                count[i] += count[i - 1];
            for (i = n - 1; i >= 0; i--)
                SA[--count[r[p[i]]]] = p[i];

            // Recalcular rangos
            q[SA[0]] = 0;
            j = 0;
            for (i = 1; i < n; i++) {
                if (r[SA[i - 1]] != r[SA[i]] ||
                    r[(SA[i - 1] + (one << k)) % n] != r[(SA[i] + (one << k)) % n])
                    j++;

                q[SA[i]] = j;
            }

            r.swap(q);
        }

        // Guardar el arreglo de rangos
        rank = r;

        // Calcular el arreglo LCP
        compute_LCP();
    }

    void compute_LCP()
    {
        std::int64_t n = t.length();
        LCP.resize(n);
        std::int64_t h = 0;
        for (std::int64_t i = 0; i < n; i++) {
            if (rank[i] > 0) {
                std::int64_t j = SA[rank[i] - 1];
                while (i + h < n && j + h < n && t[i + h] == t[j + h])
                    h++;
                LCP[rank[i]] = h;
                if (h > 0)
                    h--;
            } else {
                LCP[rank[i]] = 0; // LCP[0] = 0
            }
        }
    }

    std::pair<std::int64_t, std::int64_t> find(const std::string_view s)
    {
        std::int64_t n = t.length();
        std::int64_t m = s.length();

        std::int64_t left = -1, right = -1;

        // Buscar límite inferior
        std::int64_t l = 0, r = n - 1;
        while (l <= r) {
            std::int64_t mid = l + (r - l) / 2;
            std::string_view suffix = t.substr(SA[mid], m);
            if (suffix.compare(s) < 0) {
                l = mid + 1;
            } else {
                r = mid - 1;
            }
        }
        left = l;

        // Buscar límite superior
        r = n - 1;
        while (l <= r) {
            std::int64_t mid = l + (r - l) / 2;
            std::string_view suffix = t.substr(SA[mid], m);
            if (suffix.compare(s) <= 0) {
                l = mid + 1;
            } else {
                r = mid - 1;
            }
        }
        right = l;

        return {left, right};
    }

    std::int64_t count(const std::string_view s)
    {
        auto [left, right] = find(s);
        return right - left;
    }

    std::int64_t& operator[](std::int64_t i)
    {
        return SA[i];
    }

    std::int64_t memory_usage() const
    {
        std::int64_t total_memory = 0;

        // Tamaño de la cadena _t
        total_memory += sizeof(char) * _t.size();

        // Tamaño de la vista de cadena t (punteros y tamaño)
        total_memory += sizeof(std::string_view);

        // Tamaño del vector SA
        total_memory += sizeof(std::int64_t) * SA.size();

        // Tamaño del vector rank
        total_memory += sizeof(std::int64_t) * rank.size();

        // Tamaño del vector LCP
        total_memory += sizeof(std::int64_t) * LCP.size();

        return total_memory;
    }
};

#endif