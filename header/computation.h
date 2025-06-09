/*
 * Copyright (c) 2025 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef COMPUTATION_H
#define COMPUTATION_H

#include "definitions.h"
#include "tape.h"
#include "alphabet.h"
#include "turing_machine.h"
#include <string>
#include <sstream>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <future>
#include <stop_token>

namespace mdt {

    /**
     * @brief Representation of a computation of a Turing Machine on an input string
     *
     * @tparam K Number of tapes used by the machine
     *
     * @details This class is used for the execution of the Turing Machine for a particular input string
     * We specify the Turing Machine to use and, either use tapes created beforehand, or create tapes on the go.
     * Optionally, we can specify an Alphabet, just for the mapping of logical symbols to readable symbols. With an Alphabet,
     * we can also give a std::string as the input (instead of an already created tape), and the alphabet is used to translate
     * each character to a symbol, so that it can be written on the input tape
     *
     * During our classes, we usually distinguish between input and working tapes:
     *  The input tape is usually tape 0
     *  And the working tapes go from 1 to K-1
     *
     * To start the execution, use the start method. The stop method can be used to forcefully interrupt the execution,
     * since we can decide to use both Final-State Accepting and Terminating Accepting Turing Machines. In case the Machine
     * doesn't accept a string, it can enter a loop, with stop(), we can terminate it externally
     * To perform a single transition, use step()
     */
    template <int K>
    class computation {

        /// Alphabet used both by the Turing Machine and Tapes
        alphabet alph;

        /// K-tapes Turing Machine
        turing_machine<K> M;

        /// The K tapes the machine will use
        std::array<tape, K> tapes;

        /// The state the machine is, during each step of the computation
        state current{0};

        /// Number of transitions performed
        size_t transition_cnt{0};

        std::mutex mtx;

        std::condition_variable cv;

        /// Is the computation currently suspended? It can be resumed later
        std::atomic<bool> paused{false};

        /// Was the machine stopped from outside? This is used to determine if a computation was forcefully interrupted
        std::atomic<bool> stopped{false};

        /// Has the machine stopped on its own? Used both for Final-State accepting and Termination accepting machines
        std::atomic<bool> terminated{false};

        /// Thread that handles the execution of the computation
        std::jthread worker;
        std::promise<void> done_promise;
        std::future<void> done_future;

        /// String that will be written on the input tape at the start of the computation. This allows for multiple computations
        std::string w;

    public:

        /// Default constructor
        computation() : done_future{done_promise.get_future()} {};

        /**
         * @brief Specifies the alphabet to use for translating symbols to characters and vice versa
         *
         * @param alph Alphabet to set
         */
        void use_alphabet(const alphabet& alph) {
            this->alph = alph;
        }

        /**
         * @brief Specifies the Turing Machine that will be executed for the computation
         *
         * @param M Turing Machine that will be executed
         */
        void use_machine(const turing_machine<K>& M) {
            this-> M = M;
        }

        /**
         * @brief Sets the tapes that the machine will have access to for the computation
         *
         * @param tapes Tapes used by the machine
         */
        void use_tapes(std::array<tape, K> tapes) {
            this-> tapes = tapes;
        }

        /**
         * @brief Specifies a tape that the machine will have access to for the computation
         *
         * @param t Tape that will be used by the machine
         * @param index Number of the tape to set
         */
        void use_tape(tape t, int index) {
            if (index >= K) return;
            tapes[index] = t;
        }

        /**
         * @brief Performs a single transition
         *
         * @details Based on the current configuration, reads the current symbols
         * Tries to retrieve a transition that has the first two arguments, <current state, read symbols>
         * If the transition is defined, then it is performed: State is changed and either symbols are written and/or the heads are shifted
         *
         * @return True if a transition was found (and performed), False otherwise
         */
        bool step() {
            std::array<symbol, K> x;
            for (int i = 0; i < K; i++) {
                x[i] = tapes[i].read();
            }

            const std::optional out = M.get_transition(current, x);
            if (!out.has_value()) {
                terminated = true;
                return false;
            }
            x = out.value().second;
            for (int i = 0; i < K; i++) {
                if (x[i] == M.dx()) tapes[i].move_dx();
                else if (out.value().second[i] == M.sx()) tapes[i].move_sx();
                else tapes[i].write(x[i]);
            }
            current = out.value().first;
            return true;
        }

        /**
         * @brief Moves the head on the given tape, to the given position
         *
         * @param position Where to move the head
         *
         * @param tape Whose tape to move the head
         */
        void shift_head(const size_t position, int tape) {
            if (tape >= K || tape < 0) return;
            while (tapes[tape].move_sx()) {}
            for (int i = 0; i < position; i++)
                tapes[tape].move_dx();
        }

        /**
         * @brief Specifies the input string for this computation
         *
         * @details This is the string that will be present on the input tape at the start of the computaion
         *
         * @note For this, an alphabet needs to be specified! It is needed to translate character to symbols
         *
         * @param w Input string for the computation
         */
        void input_string(const std::string &w) {
            if (alph.symbol_count() == 0) return;
            this->w = w;

        }

        /**
         * @brief Starts the computation
         *
         * @details Calls the step() method to perform a single transition, until we get to a configuration that has no
         * possible transitions, counting how many transitions are performed. The execution is handled by a thread
         *
         * @note If a Turing Machine loops indefinitely, use stop to force terminate it
         */
        void start() {
            if (!w.empty()) write_input_string();
            worker = std::jthread([this](std::stop_token st) {
                for (int i = 0; i < K; i++)
                    shift_head(0, i);
                transition_cnt = 0;

               while (!stopped && !st.stop_requested() && !terminated) {
                   {
                       std::unique_lock<std::mutex> lock(mtx);
                       cv.wait(lock, [this, &st] {
                          return !paused || st.stop_requested();
                       });
                   }
                   if (stopped || st.stop_requested()) break;
                   step();
                   transition_cnt++;
               }
                done_promise.set_value();
            });
        }

        /**
         * @bried Paused the execution of this Turing Machine
         */
        void pause() {
            if (!terminated && !stopped) paused = true;
        }

        /**
         * @brief Waits for the termination of the computation
         */
        void wait_for_termination() const {
            done_future.wait();
        }

        /**
         * @brief Resumes the execution of a previously paused Turing Machine
         */
        void resume() {
            if (!paused || stopped || terminated) return;
            {
                std::lock_guard<std::mutex> lock(mtx);
                paused = false;
            }
            cv.notify_all();
        }

        /**
         * @brief Stops the computation
         *
         * @details This method is used to forcefully stop the thread handling the computation
         *
         * @note The 'terminated' flag will not be set to true, but 'stopped' will
         */
        void stop() {
            stopped = true;
            cv.notify_all();
        }

        /**
         * @brief Tells whether the computation has stopped or not, naturally
         *
         * @note Invoking stop forcefully interrupts the computation, but doesn't flag the computation as terminated
         *
         * @return True if the Turing Machine has terminated on the given input string, False otherwise
         */
        [[nodiscard]] bool is_terminated() {
            return terminated;
        }

        /**
         * @brief Tells whether the Turing Machine has accepted the input string or not
         *
         * @note This only makes sense for a Final State accepting machine
         *
         * @return True if the Turing Machine has terminated on the given input string, on a final state, False otherwise
         */
        [[nodiscard]] bool has_accepted() const {
            return terminated ? M.is_final_state(current) : false;
        }

        /**
         * @brief Tells whether the Turing Machine is currently paused or not
         *
         * @return True if the Turing Machine is mid-computation, and paused, False otherwise
         */
        [[nodiscard]] bool is_paused() {
            return paused;
        }

        /**
         * @brief Tells whether the Turing Machine was forcefully stopped or not
         */
        [[nodiscard]] bool is_stopped() {
            return stopped;
        }

        /**
         * @brief Returns the number of transitions performed up until the point of invocation
         *
         * @return Number of transitions performed
         */
        [[nodiscard]] size_t transition_count() const {
            return transition_cnt;
        }

        /**
         * @brief Returns the content of the specified tape
         *
         * @details Returns a string containing the character associated with each symbol of the given tape, using the
         * alphabet to translate logical symbols to readable ones
         *
         * @param index Number of the tape we want the content of
         *
         * @note The last three characters (...) are printed to express the concept of infinite tape, since those positions would be empty anyway
         *
         * @return String with the content of the given tape
         */
        [[nodiscard]] std::string output(int index) {
            if (index >= K || index < 0) return "";
            std::vector<char> v;
            v.reserve(tapes[index].size() + 3);
            for (symbol s : tapes[index].get_content())
                v.push_back(alph.get_representation(s).value_or(alphabet::blank_char));
            v.push_back('.');
            v.push_back('.');
            v.push_back('.');
            return std::string{v.begin(), v.end() };
        }

        /**
         * @brief Actually writes the input string on the input tape
         *
         * @details Writes each symbol of the string to the input tape, using the specified Alphabet
         *
         * @note The alphabet needs to be set, as it is needed to translate che characters of the string to symbols
         * that the Turing Machine can actually 'understand'
        */
        void write_input_string() {
            for (int i = 0; i < K; i++) shift_head(0, i);
            for (char c : w) {
                symbol opt = alph.get_symbol(c).value_or(blank);
                tapes[0].write(opt);
                for (int i = 1; i < K; i++) tapes[i].write(blank);
                for (int i = 0; i < K; i++) tapes[i].move_dx();
            }
            for (int i = 0; i < K; i++) shift_head(0, i);
        }

        /**
         * @brief Returns the content of all tapes, from the last to the first
         *
         * @details Returns a string containing the content of all the tapes used for the computation
         *
         * @note This method just calls output(i) on i = K-1, ..., 0
         *
         * @return String with the content all tapes
         */
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