#ifndef TAPE_H
#define TAPE_H

#include "definitions.h"
#include <utility>
#include <vector>

namespace mdt {
    class tape {
    private:
        static constexpr size_t max_size = 999999;

        std::vector<symbol> content;
        std::size_t head;

    public:
        tape() : tape(1) {}
        explicit tape(const size_t size) : content(size), head(0) {}
        explicit tape(std::vector<symbol> content) : content(std::move(content)), head(0) {}

        [[nodiscard]] symbol read() const {
            return content[head];
        }

        void write(const symbol x) {
            content[head] = x;
        }

        bool move_sx() {
            if (head <= 0) return false;
            head--;
            return true;
        }

        bool move_dx() {
            if (head == max_size) return false;
            content.push_back(blank);
            head++;
            return true;
        }

        std::vector<symbol> get_content() {
            return content;
        }

        size_t size() const {
            return content.size();
        }
    };
}

#endif