#ifndef TURING_MACHINE_H
#define TURING_MACHINE_H

#include "definitions.h"
#include "couple.h"
#include <unordered_map>
#include <array>
#include <optional>
#include <set>
#include <sstream>

namespace mdt {

    /**
     * @brief Logical representation of a Turing Machine
     *
     * @tparam K Number of tapes used by this machine
     *
     * @details This class just represents the logic behind a Turing Machine, it does not hold any execution capabilities on its own.
     * We only specify how many states and symbols the machine has @see (definitions.h)
     * Moreover, out of all states, we can specify which are final states
     * Lastly, we get to the core of the logic, the transitions
     *
     * During our classes, we represented transitions in Turing Machines as Quadruples (q, x, a, t):
     *      q: State before the transition
     *      x: K-tuple of symbols read
     *      a: K-tuple of symbols written OR head movement (left or right)
     *      t: State after the transition
     *
     * This means that both left and right movement are treated as special symbols, not specified in an alphabet:
     * Each Turing Machine has its definition of left and right, the way we treated them in class was:
     *  Consider the alphabet of the Machine has r symbols, from a{0} to a{r-1}, then Right is a{r} and Left is a{r+1}
     *
     *  @note Each turing Machine has the initial state q0 (0)
     */
    template<int K>
    class turing_machine {
        state state_count{0};

        symbol symbol_count{0};

        std::set<state> final_states;

        std::unordered_map<
            couple<state, std::array<symbol, K>>,
            couple<state, std::array<symbol, K>>
        > transitions;

    public:

        /// Creates the simplest Turing Machine, having only 1 state (q0) and 1 symbol
        turing_machine() : turing_machine(1, 1) {};

        /**
         * @brief Creates a Turing Machine with the given symbols and states
         *
         * @param state_count Number of states, must be non-negative, otherwise it's set to 1
         *
         * @param symbol_count Number os symbols, must be non-negative, otherwise it's set to 1
         *
         * @note These can't be changed after instantiation, so please determine the number of states and symbols
         * before creating the machine
         */
        turing_machine(const state state_count, const symbol symbol_count) {
            this->state_count = (state_count) > 0 ? state_count : 1;
            this->symbol_count = (symbol_count) > 0 ? symbol_count : 1;
        }

        /**
         * @brief Adds a transition to this machine
         *
         * @details This method is based on the fact that deterministic Turing Machines have transitions that are functional
         * in the first two arguments, so when adding a transition (q x a t), if q and x are already present in another transition (q x b s)
         * then only the already present transition will remain, the new one will not be present
         *
         * @param state_in Starting state of the transition, q
         *
         * @param symbols_in K-tuple of symbols read, x
         *
         * @param symbols_out K-tuple of symbols written or movements, a
         *
         * @param state_out Ending state of the transition, q
         *
         * @note In case we need to overwrite a transition, remove it beforehand, and then add it back, with the new 'out' values
         */
        void add_transition(const state state_in, std::array<symbol, K> symbols_in, std::array<symbol, K> symbols_out, const state state_out) {
            if (state_in >= state_count || state_out >= state_count) return;
            for (const symbol s : symbols_in) if (s >= symbol_count) return;
            for (const symbol s : symbols_out) if (s >= symbol_count + 2) return;

            couple<state, std::array<symbol, K>> in (state_in, symbols_in);
            couple<state, std::array<symbol, K>> out (state_out, symbols_out);

            transitions.insert({in, out});
        }

        /**
         * @brief Removes a transition from this machine
         *
         * @details This method is based on the fact that deterministic Turing Machines have transitions that are functional
         * in the first two arguments, so we know for sure that, if at least a transition with q and x exists (q x ? ?), then it's unique
         *
         * @param q In state of the transition to remove
         *
         * @param x K-tuple of the read symbols of the transition to remove
         */
        void remove_transition(const state q, std::array<symbol, K> x) {
            transitions.erase(couple{q, x});
        }

        /**
         * @brief Attempts to get the second half of a transition given the first half
         *
         * @details In a deterministic Turing Machine, transitions are functional in the first 2 parameters, this method
         * given q ('from' state) and x (K-tuple of read symbols) returns t ('to' state) and a (K-tuple of written symbols or head movement)
         * IF the transition (q x a t) exists in this machine
         *
         * @param q In state of the transition
         *
         * @param x K-tuple of read symbols
         *
         * @return Couple (t, a) where t is the out state of the transition and a the K-tuple of written symbols or movements, if (q x t a) is an actual transition
         * null option otherwise
         */
        [[nodiscard]] std::optional<couple<state, std::array<symbol, K>>> get_transition(const state q, const std::array<symbol, K> x) {
            return (transitions.find(couple{q,x}) != transitions.end()) ? std::optional(transitions[couple{q,x}]) : std::nullopt;
        }

        /**
         * @brief Returns the definition of Left movement for this machine
         *
         * @return Left movement symbol (number of symbols + 1)
         */
        [[nodiscard]] symbol sx() const {
            return symbol_count + 1;
        }

        /**
         * @brief Returns the definition of Right movement for this machine
         *
         * @return Right movement symbol (number of symbols)
         */
        [[nodiscard]] symbol dx() const {
            return symbol_count;
        }

        /**
         * @brief Attempts to mark a state as final
         *
         * @details Adds a state in the set of final states, only if it doesn't exceed the state count, and is non-negative
         *
         * @param state State to mark as final
         */
        void add_final_state(const state state) {
            if (state >= state_count || state < 0) return;
            final_states.insert(state);
        }

        /**
         * @brief Attempts to mark a set of states as final
         *
         * @details Adds each state in the set of final states, for each, only if it doesn't exceed the state count, and is non-negative
         *
         * @param states States to mark as final
         *
         * @see add_final_state method
         */
        void add_final_states(const std::set<state>& states) {
            final_states.insert(states.begin(), states.end());
        }

        /**
         * @briefTells whether the given state is final or not
         *
         * @param state State to check
         *
         * @return True if the state is final, False otherwise
         */
        [[nodiscard]] bool is_final_state(const state state) const {
            return final_states.find(state) != final_states.end();
        }

        /**
         * @brief Returns a readable string summerizing the information about this machine, states, symbols, final states and all transitions
         *
         * @return A readable, pretty, string with all the information about this Turing Machine
         */
        [[nodiscard]] std::string to_string() const {
            std::stringstream ss;

            ss << "States Q = { ";
            bool first = true;
            for (int i = 0; i < state_count; i++) {
                if (!first) ss << ", ";
                ss << "q" << i;
                first = false;
            }
            ss << " }" << std::endl << "|Q| = " << state_count << std::endl << std::endl;

            ss << "Final States F = { ";
            first = true;
            for (const int q : final_states) {
                if (!first) ss << ", ";
                ss << "q" << q;
                first = false;
            }
            ss << " }" << std::endl << std::endl;

            ss << "Number of symbols |S| = " << symbol_count << std::endl;
            ss << "Right (R): " << symbol_count << std::endl;
            ss << "Left (L): " << symbol_count + 1 << std::endl;
            ss << std::endl;

            ss << "Number of tapes: " << K << std::endl  << std::endl;

            ss << "Transitions: " << std::endl;
            for (auto t : transitions) {
                ss << t.first.first << " (";
                first = true;
                for (const symbol s : t.first.second) {
                    if (!first) ss << ", ";
                    ss << s;
                    first = false;
                }
                ss << ") (";
                first = true;
                for (const symbol s : t.second.second) {
                    if (!first) ss << ", ";
                    ss << s;
                    first = false;
                }
                ss << ") " << t.second.first << std::endl;
            }
            return ss.str();
        }

        /**
         * @brief Returns the binary representation of this Turing Machine, with alphabet {0, 1}
         *
         * @details This representation is an extension of the one I've learnt during the course. Since each symbol and state
         * can be indexed, it can be represented, in binary, through its index, in unary. (n in base 10 is n+1 ones in base 1)
         *
         * The representation has this form: consider s states and r symbols and k tapes, and final states qx, qy and qz
         *  1^{k + 1}001^{s + 1}001^{r + 1}001^{x + 1}01^{y + 1}01^{z + 1}0000<transition{1}>000<transition{2}>000...000<transition{m}0000
         *
         *  Where each <transition{i}> is represented as follows:
         *   Suppose the transition is (q{i} (a{j1}, ..., a{jK}) (a{z1}, ..., a{zK}) q{t}), where K is the number of tapes
         *   Then 1^{i+1}001^{j1 + 1}0...01^{jK + 1}001^{z1 + 1}0...01^{zK + 1}001^{t + 1}
         *
         * @return A string with the specified representation of this Turing Machine
         */
        [[nodiscard]] std::string to_binary_representation() const {
            const size_t n = transitions.size();

            std::vector<char> binr;
            const size_t w = final_states.size();
            binr.reserve( (state_count + symbol_count + w * (state_count + 1) + 13) + n * (1 + 2 * state_count + 2 * K * (symbol_count + 1)));

            // Number of tapes
            for (int i = 0; i <= K; i++)
                binr.push_back('1');

            // Separator
            binr.push_back('0');
            binr.push_back('0');

            // Number of states
            for (int i  = 0; i < state_count + 1; i++)
                binr.push_back('1');

            // Separator
            binr.push_back('0');
            binr.push_back('0');

            // Number of symbols
            for (int i = 0; i < symbol_count + 1; i++)
                binr.push_back('1');

            // Separator
            binr.push_back('0');
            binr.push_back('0');

            // Final states
            for (const state f : final_states) {
                for (int i = 0; i <= f; i++)
                    binr.push_back('1');
                binr.push_back('0');
            }

            // List of transitions [Start]
            binr.push_back('0');
            binr.push_back('0');
            binr.push_back('0');

            for (const auto& t : transitions) {
                couple<state, std::array<symbol, K>> in = t.first;
                couple<state, std::array<symbol, K>> out = t.second;

                // State IN
                for (int i = 0; i < in.first + 1; i++)
                    binr.push_back('1');
                binr.push_back('0');
                binr.push_back('0');

                // Symbols IN
                for (const symbol s : in.second) {
                    for (int i = 0; i < s + 1; i++)
                        binr.push_back('1');
                    binr.push_back('0');
                }
                binr.push_back('0');

                // Symbols OUT
                for (const symbol s : out.second) {
                    for (int i = 0; i < s + 1; i++)
                        binr.push_back('1');
                    binr.push_back('0');
                }
                binr.push_back('0');

                // State OUT
                for (int i = 0; i < out.first + 1; i++)
                    binr.push_back('1');
                binr.push_back('0');
                binr.push_back('0');
                binr.push_back('0');
            }

            // List of transitions [End]
            binr.push_back('0');

            return std::string{binr.begin(), binr.end()};
        }

        /**
         * @brief Tries to initialize a Turing Machine from a given binary representation
         *
         * @details Tries to parse the binary string in order to recreate the machine it describes. In order for a successful
         * initialization, the string needs to be of the format specified in the 'to_binary_representation' method
         *
         * @see to_binary_representation
         */
        void from_binary_representation(std::string binary_rep) {
            std::string::iterator it = binary_rep.begin();

            // Check for the number of tapes
            int k = -1;
            while (*it == '1') {
                ++k;
                ++it;
            }
            if (k != K) return; // Wrong number of tapes. Would parse an incorrect machine
            ++it;
            ++it;

            // Number of states
            this->state_count = -1;
            while (*it == '1') {
                ++this->state_count;
                ++it;
            }

            ++it;
            ++it;

            // Number of symbols
            this->symbol_count = -1;
            while (*it == '1') {
                ++this->symbol_count;
                ++it;
            }

            ++it;
            ++it;

            // Final states
            state f = -1;
            while (true) {
                if (*it == '0') break;
                while (*it == '1') {
                    ++f;
                    ++it;
                }
                ++it;
                this->add_final_state(f);
            }

            ++it;
            ++it;
            ++it;

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
                this->add_transition(in.first, in.second, out.second, out.first);
            }
        }
    };
}

#endif