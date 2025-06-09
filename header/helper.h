/*
 * Copyright (c) 2025 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef HELPER_H
#define HELPER_H

#include <fstream>
#include "definitions.h"
#include "alphabet.h"
#include "turing_machine.h"
#include "tape.h"
#include "../third_party/nlohmann/json.hpp"
using json = nlohmann::json;

namespace mdt {
    /**
     * @brief Tries to translate a readable translation into a usable transition for a Turing Machine
     *
     * @definition The given Machine should be the target for this machine, as it will provide its definition of Left and Right, moreover,
     * @definition the transition has K symbols both as in and out, where is the same as the number of tapes of the Turing Machine.
     * @definition The alphabet is used to translate each character to its symbol for that alphabet
     *
     * @tparam K Number of symbols each transition has both for input and output
     *
     * @param S Alphabet, used to translate the characters of the readable transition into logical symbols
     *
     * @param M Machine for which we want to translate the transition, provide its definition of Left and Right
     *
     * @param transition Readable transition. Transition that has states and characters instead of states and symbols
     *
     * @return Either returns an equivalent, but usable, transition, or a null optional if there are any symbols out of the range of the Alphabet
     */
    template<int K>
    static std::optional<couple<
        couple<state, std::array<symbol, K>>,
        couple<state, std::array<symbol, K>>
    >> transition_from_readable(alphabet& S, turing_machine<K>& M, couple<
        couple<state, std::array<char, K>>,
        couple<state, std::array<char, K>>
    > transition) {
        couple<state, std::array<symbol, K>> in;
        in.first = transition.first.first;
        for (int i = 0; i < transition.first.second.size(); i++) {
            if (transition.first.second[i] == 'L')
                in.second[i] = M.sx();
            else if (transition.first.second[i] == 'R')
                in.second[i] = M.dx();
            else {
                if (const std::optional<symbol> x = S.get_symbol(transition.first.second[i]); x.has_value())
                    in.second[i] = x.value();
                else
                    return std::nullopt;
            }
        }

        couple<state, std::array<symbol, K>> out;
        out.first = transition.second.first;
        for (int i = 0; i < transition.second.second.size(); i++) {
            if (transition.second.second[i] == 'L')
                out.second[i] = M.sx();
            else if (transition.second.second[i] == 'R')
                out.second[i] = M.dx();
            else {
                if (const std::optional<symbol> x = S.get_symbol(transition.second.second[i]); x.has_value())
                    out.second[i] = x.value();
                else
                    return std::nullopt;
            }
        }

        return std::optional(couple {in, out});
    }

    /**
     * @brief Serializes the alphabet S into a json
     *
     * @note To serialize the alphabet, we just save the map (symbol -> char)
     *
     * @param S Alphabet to serialize
     *
     * @return Corresponding json object
     */
    static json serialize_alphabet(alphabet& S) {
        json j;
        for (symbol s = 0; s < S.symbol_count(); s++) {
            j[s] = {s, std::string(1, S.get_representation(s).value())};
        }
        return j;
    }

    /**
     * @brief Serializes the tape T into a json
     *
     * @note To serialize the tape, we just save the content of the tape, and the position of the head
     *
     * @param T Tape to serialize
     *
     * @return Corresponding json object
     */
    static json serialize_tape(tape& T) {
        json j{{"Content", T.get_content()}, {"Head", T.head_position()}};
        return j;
    }

    /**
     * @brief Serializes the Turing Machine M to a json
     *
     * @note To serialize the Turing Machine, we save the number of states, number of symbols, final states, and list of transitions
     *
     * @tparam K Number of tapes of the machine
     *
     * @param M Turing Machine to serialize
     *
     * @return Corresponding json object
     */
    template<int K>
    static json serialize_turing_machine(turing_machine<K>& M) {
        json j;

        std::string binr = M.to_binary_representation();
        std::string::iterator it = binr.begin();

        int k = -1;
        while (*it == '1') {
            ++k;
            ++it;
        }

        ++it;
        ++it;

        // Number of states
        int state_count = -1;
        while (*it == '1') {
            ++state_count;
            ++it;
        }

        ++it;
        ++it;

        // Number of symbols
        int symbol_count = -1;
        while (*it == '1') {
            ++symbol_count;
            ++it;
        }

        ++it;
        ++it;

        // Final states
        std::vector<state> F;
        state f = -1;
        while (true) {
            if (*it == '0') break;
            while (*it == '1') {
                ++f;
                ++it;
            }
            ++it;
            F.push_back(f);
        }

        ++it;
        ++it;
        ++it;

        std::unordered_map<couple<int, std::array<int, K>>, couple<int, std::array<int, K>>> t;
        couple<int, std::array<int, K>> in, out;
        while (true) {
            if (*it == '0') break;
            in.first = -1;
            while (*it == '1') {
                ++in.first;
                ++it;
            }

            ++it;
            ++it;

            in.second.fill(-1);
            for (int i = 0; i < K; i++) {
                while (*it == '1') {
                    ++in.second[i];
                    ++it;
                }
                ++it;
            }

            ++it;


            out.second.fill(-1);
            for (int i = 0; i < K; i++) {
                while (*it == '1') {
                    ++out.second[i];
                    ++it;
                }
                ++it;
            }

            ++it;

            out.first = -1;
            while (*it == '1') {
                ++out.first;
                ++it;
            }

            ++it;
            ++it;
            ++it;
            t.insert({in, out});
        }

        j["#Tapes"] = K;
        j["#States"] = state_count;
        j["#Symbols"] = symbol_count;
        j["FStates"] = F;

        json transitions = json::array();
        for (auto x : t) {
            json tmp = {
                {"q", x.first.first},
                {"x", x.first.second},
                {"a", x.second.second},
                {"t", x.second.first}
            };
            transitions.push_back(tmp);
        }

        j["Transitions"] = transitions;


        return j;
    }

    /**
     * @brief Deserializes an alphabet from the json J_ALPH
     *
     * @see serialize_alphabet
     *
     * @param j_alph Json containing the alphabet's information
     *
     * @return Corresponding alphabet
     */
    static alphabet deserialize_alphabet(json j_alph) {
        alphabet S;

        for (json::iterator it = j_alph.begin(); it != j_alph.end(); ++it) {
            bool first = true;
            for (auto w : (*it)[1].get<std::string>()) {
                if (!first) break;
                first = false;
                S.add_symbol(w);
            }
        }

        return S;
    }

    /**
     * @brief Deserializes a tape from the json J_TPE
     *
     * @see serialize_tape
     *
     * @param j_tpe Json containing the tape's info
     *
     * @return Corresponding tape
     */
    static tape deserialize_tape(json j_tpe) {
        const size_t head = j_tpe["Head"].get<size_t>();
        const std::vector<symbol> cnt = j_tpe["Content"].get<std::vector<symbol>>();

        tape T(cnt);
        while (T.move_sx()) {}
        for (size_t i = 0; i < head; i++)
            T.move_dx();

        return T;
    }

    /**
     * @brief Deserializes a Turing Machine from the json J_TM
     *
     * @see serialize_turing_machine
     *
     * @tparam K Number of tapes of the machine
     *
     * @param j_tm Json containing the Turing Machine's info
     *
     * @return Corresponding Turing Machine, or null optional if the number of tapes wasn't coherent
     */
    template <int K>
    static std::optional<turing_machine<K>> deserialize_turing_machine(json j_tm) {

        if (K != j_tm["#Tapes"].get<int>()) return std::nullopt;
        int states = j_tm["#States"].get<int>();
        int symbols = j_tm["#Symbols"].get<int>();
        turing_machine<K> M(states, symbols);
        std::set<state> finals = j_tm["FStates"].get<std::set<state>>();
        M.add_final_states(finals);

        json transitions = j_tm["Transitions"].get<json>();
        std::array<symbol, K> x, a;
        for (auto w = transitions.begin(); w != transitions.end(); ++w) {
            state q = (*w)["q"].get<state>();
            x = (*w)["x"].get<std::array<symbol, K>>();
            state t = (*w)["t"].get<state>();
            a = (*w)["a"].get<std::array<symbol, K>>();
            M.add_transition(q, x, a, t);
        }

        return M;
    }

    /**
     * @brief Saves the json object in a file that has the name given, without extension
     *
     * @param j JSON to write to file
     *
     * @param filename File name, no extension
     */
    static void json_to_file(const json& j, const std::string& filename) {
        std::ofstream file(filename);
        file << j.dump();
    }

    /**
     * @brief Retrieves a json object from the file that has the name given, without extension
     *
     * @param j JSON to read from file
     *
     * @param filename File name, no extension
     */
    static void json_from_file(json& j, const std::string& filename) {
        std::ifstream file(filename);
        file >> j;
    }

}

#endif //HELPER_H
