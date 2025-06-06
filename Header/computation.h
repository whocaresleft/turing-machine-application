#ifndef COMPUTATION_H
#define COMPUTATION_H

#include "definitions.h"
#include "tape.h"
#include "alphabet.h"
#include "turing_machine.h"
#include <string>
#include <sstream>

namespace mdt {

    template <int K>
    class computation {

    private:
        alphabet alph;
        turing_machine<K> M;
        std::array<tape, K> tapes;

        state current{0};
        size_t transition_cnt{0};
        bool terminated{false};

    public:

        computation() = default;

        void use_alphabet(const alphabet& alph) {
            this->alph = alph;
        }

        void use_machine(const turing_machine<K>& M) {
            this-> M = M;
        }

        void use_tapes(std::array<tape, K> tapes) {
            this-> tapes = tapes;
        }

        void use_tape(tape t, int index) const {
            if (index >= K) return;
            tapes[index] = t;
        }

        bool step() {
            std::array<symbol, K> x;
            for (int i = 0; i < K; i++) {
                x[i] = tapes[i].read();
            }

            const std::optional out = M.get_transition(current, x);
            if (!out.has_value()) return false;
            x = out.value().second;
            for (int i = 0; i < K; i++) {
                if (x[i] == M.dx()) tapes[i].move_dx();
                else if (out.value().second[i] == M.sx()) tapes[i].move_sx();
                else tapes[i].write(x[i]);
            }
            current = out.value().first;
            return true;
        }

        void shift_head(const size_t position, int tape) {
            if (tape >= K || tape < 0) return;
            while (tapes[tape].move_sx()) {}
            for (int i = 0; i < position; i++)
                tapes[tape].move_dx();
        }

        void input_string(const std::string &w) {
            for (int i = 0; i < K; i++) shift_head(0, i);
            for (char c : w) {
                symbol opt = alph.get_symbol(c).value_or(blank);
                tapes[0].write(opt);
                for (int i = 1; i < K; i++) tapes[i].write(blank);
                for (int i = 0; i < K; i++) tapes[i].move_dx();
            }
            for (int i = 0; i < K; i++) shift_head(0, i);
        }

        void start() {
            for (int i = 0; i < K; i++)
                shift_head(0, i);
            transition_cnt = 0;
            while (step())
                transition_cnt++;
            terminated = true;
        }

        [[nodiscard]] bool has_terminated() const {
            return terminated;
        }

        [[nodiscard]] bool has_accepted() const {
            return terminated ? M.is_final_state(current) : false;
        }

        [[nodiscard]] size_t transition_count() const {
            return transition_cnt;
        }

        [[nodiscard]] std::string output(int tape) {
            if (tape >= K || tape < 0) return "";
            std::vector<char> v;
            v.reserve(tapes[tape].size() + 3);
            for (symbol s : tapes[tape].get_content())
                v.push_back(alph.get_representation(s).value_or(alphabet::blank_char));
            v.push_back('.');
            v.push_back('.');
            v.push_back('.');
            return std::string{v.begin(), v.end() };
        }

        [[nodiscard]] std::string output() {
            std::stringstream ss;
            for (int i = K - 1; i >= 0; --i) {
                ss << i << ": "<< output(i) << std::endl;
            }
            return ss.str();
        }
    };


}

#endif