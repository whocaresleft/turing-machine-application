#ifndef ALPHABET_H
#define ALPHABET_H

#include <optional>
#include <unordered_map>
#include "definitions.h"

namespace mdt {

    /**
     * @brief Alphabet for Turing Machines
     *
     * @details Alphabets can be used to 'help' in the context of a computation of a Turing Machine on a certain input string
     * They are used to map each 'readable' symbol (character) into a 'logical' symbol  @see (definitions.h)
     * So we can have a pretty, readable, way to give input and read output from a Turing Machine
     */
    class alphabet {

        /// Maps a character to a logical symbol
        std::unordered_map<char, symbol> map;

        /// Inverse map, from each logical symbol to its character
        std::unordered_map<symbol, char> inverse;

        int count{0};

    public:

        /// During our classes, we used this character to represent an empty cell
        static constexpr char blank_char = '*';

        alphabet() = default;

        /**
         * @brief Adds a character to this alphabet, assigning a logical symbol to it
         *
         * @param symbol Character to add to this alphabet
         */
        void add_symbol(const char& symbol) {
            if (map.contains(symbol)) return;
            inverse[count] = symbol;
            map[symbol] = count;
            count++;
        }

        /**
         * @brief Adds a set of characters to this alphabet
         *
         * @param symbols Characters to add to this alphabet
         *
         * @see add_symbol method
         */
        void add_symbols(const std::set<char>& symbols) {
            for (const char s : symbols) {
                add_symbol(s);
            }
        }

        /**
         * @brief Tries to retrieve the logical symbol associated with the given character
         *
         * @param symbol Character to find
         *
         * @return The logical symbol mapped to the character if the symbol is in the alphabet, or a null optional
         */
        std::optional<symbol> get_symbol(const char& symbol) {
            if (symbol == blank_char) return {blank};
            return map.contains(symbol) ? std::optional(map[symbol]) : std::nullopt;
        }

        /**
         * @brief Tries to retrieve the character associated with the given logical symbol
         *
         * @param symbol Logical symbol to find
         *
         * @return The readable symbol (character) mapped to the logical symbol if the symbol is in the alphabet, or a null option
         */
        std::optional<char> get_representation(const symbol& symbol) {
            if (symbol == blank) return {blank_char};
            return inverse.contains(symbol) ? std::optional(inverse[symbol]) : std::nullopt;
        }

        /**
         * @brief Returns the symbol count
         *
         * @return The number of symbols in this alphabet
         */
        size_t symbol_count() const {
            return count;
        }

    };
}

#endif