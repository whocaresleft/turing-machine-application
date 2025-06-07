#ifndef TAPE_H
#define TAPE_H

#include "definitions.h"
#include <utility>
#include <vector>

namespace mdt {

    /**
     * @brief Logical representation of a Turing Machine's Tape
     *
     * @details This class represents the Tapes that Turing Machine use during computation, to read and write symbols
     * as well as move through it.
     * The tape does not hold readable symbols (characters) but logical symbols @see (definitions.h)
     *
     * @note This is a Left Limited Tape, since all the actual Machines created during the course were Standard Machines (with Left Limited Tapes)
     * Moreover, the tape should be used uniquely for ONE Turing Machine, as it holds the position of the reading/writing head
     */
    class tape {

        /// It looks cool, I thought maybe I shouldn't allow everyone to just fill memory by just adding symbols
        static constexpr size_t max_size = 999999;

        /// Actual content of the tape
        std::vector<symbol> content;

        /// Position of the head (which cell the Turing Machine is on)
        std::size_t head;

    public:

        /// Creates the most symple tape, that can hold only 1 character
        tape() : tape(1) {}

        /// Creates a tape with the given size, initializing everything to the blank symbol (empty cell)
        explicit tape(const size_t size) : content(size), head(0) { for (int i = 0; i < size; i++) content[i] = blank; }

        /// Creates a tape copying the content of the given vector
        explicit tape(std::vector<symbol> content) : content(std::move(content)), head(0) {}

        /**
         * @brief Reads and returns the symbol in the cell the head is currently on
         *
         * @return The symbol contained in the cell corresponding to the head's position
         */
        [[nodiscard]] symbol read() const {
            return content[head];
        }

        /**
         * @brief Writes the given symbol in the cell the head is currently on
         *
         * @param x Symbol to write in the cell corresponding to the head's position
         */
        void write(const symbol x) {
            if (x < blank) return;
            content[head] = x;
        }

        /**
         * @brief Moves the head to the left, if it's not on the first position
         *
         * @note The head can be moved left when we are not on the first cell, as the tape is Left limited
         *
         * @return True if the head could be moved, False otherwise
         */
        bool move_sx() {
            if (head <= 0) return false;
            head--;
            return true;
        }

        /**
         * @brief Moves the head to the right, extending the tape if needed
         *
         * @note If we are on the last cell (based on the current size), we attempt to increase the vector by 1 cell, if that is possible @see max_size
         *
         * @return True if the head could be moved, so tape was increased, False otherwise, so tape could not be increased and head wasn't moved
         */
        bool move_dx() {
            if (head == max_size) return false;
            if (head == content.size()) content.push_back(blank);
            head++;
            return true;
        }

        /**
         * @bried Returns the position of the head, so the index of the cell the head is currenty placed on
         *
         * @return Head position on the tape
         */
        [[nodiscard]] size_t head_position() const {
            return head;
        }

        /**
         * @brief Returns the current size of the tape
         *
         * @details Yes, Turing Machine Tapes should be of infinite length. But realistically, they can't, so by size
         * I mean we return the number of actually used cells up until the invocation
         *
         * @return Size of the vector of the content of the tape
         */
        [[nodiscard]] size_t size() const {
            return content.size();
        }

        [[nodiscard]] std::vector<symbol> get_content() {
            return content;
        }
    };
}

#endif