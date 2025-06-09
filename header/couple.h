/*
 * Copyright (c) 2025 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef COUPLE_H
#define COUPLE_H

#include <functional>

/**
 * @brief Couple that holds two elements of any two types. Hashing has been defined for couples so they can be used as keys in maps
 *
 * @tparam T type of the first element
 * @tparam R type of the second element
 */
template<typename T, typename R>
class couple {
public:
    T first;
    R second;
    couple() = default;
    couple(const T& first, const R& second) : first(first), second(second) {}
    bool operator==(const couple<T, R>& o) const { return first == o.first && second == o.second; }
};

namespace std {
    template<typename T, typename R>
    struct hash<couple<T, R>> {
        std::size_t operator()(const couple<T, R>& c) const {
            std::size_t h1 = std::hash<T>{}(c.first);
            std::size_t h2 = std::hash<R>{}(c.second);
            return h1 ^ (h2 << 1);
        }
    };

    template <typename T, std::size_t N>
    struct hash<std::array<T, N>> {
        std::size_t operator()(const std::array<T, N>& arr) const {
            std::size_t seed = 0;
            for (const T& val : arr) {
                seed ^= std::hash<T>{}(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            }
            return seed;
        }
    };
}

#endif