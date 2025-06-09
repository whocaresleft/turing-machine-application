//
// Created by Franc on 6/8/2025.
//

#ifndef GUI_APP_HELPERS_H
#define GUI_APP_HELPERS_H

#include <optional>
#include <algorithm>
#include <set>
#include "helper.h"
#include "alphabet.h"

using json = nlohmann::json;

namespace gui_app {


    struct StateId {
        int state_id;
        int in_id;
        int out_id;
    };

    struct State {
        std::string label;
        StateId id;
        mutable bool final;
        mutable ImVec2 position;
    };

    struct Transition {
        mutable char label_buffer[128];
        int id;
        int from_state;
        int to_state;
    };

    class FSM {

    private:
        static int next_node_id;
        static int next_link_id;
        static std::vector<Transition> tself_loops;


        static StateId get_next_node_ids() {
            StateId id;
            id.state_id = next_node_id;
            id.in_id = next_node_id + 1;
            id.out_id = next_node_id + 2;
            next_node_id += 3;
            return id;
        }

        static int get_next_link_id() {
            return next_link_id++;
        }

        static int determine_K(std::string transition) {
            int k1 = 0, k2 = 0;
            std::string::iterator i;
            for (i = transition.begin(); i != transition.end(); ++i) {
                if ((*i) == '/') break;
                k1++;
            }
            ++i;
            for (; i != transition.end(); ++i) {
                k2++;
            }
            return (k1 == k2) ? k1 : -1;
        }

    public:

        static void init() {
            next_node_id = 1;
            next_link_id = 20000;
        }

        static char content[500];
        static std::vector<State> states;
        static std::vector<Transition> transitions;
        static std::vector<Transition> self_loops;


        static void add_state(const ImVec2 position) {

            State s;
            s.id = get_next_node_ids();

            std::stringstream ss;
            ss << "q" << ((s.id.state_id - 1) / 3);

            s.label = ss.str();
            s.position = position;
            s.final = false;
            states.push_back(s);
        }

        static void add_transition(const int from_state, const int to_state) {

            Transition t{};
            constexpr char a = '\0';
            strncpy(t.label_buffer, &a, 128);
            t.id = get_next_link_id();
            t.from_state = from_state;
            t.to_state = to_state;
            transitions.push_back(t);
        }

        static std::optional<State> from_pin(int pin_id) {
            for (State s : states) {
                if (s.id.in_id == pin_id) return std::optional{s};
                if (s.id.out_id == pin_id) return std::optional{s};
            }
            return std::nullopt;
        }

        static void remove_state(int state_id) {
            State error{"e",{-1, -1, -1}};

            // First remove all transitions with that state, than the state
            self_loops.erase(
                std::remove_if(self_loops.begin(), self_loops.end(),
                    [&state_id, error](const Transition& t) {
                        return t.id == state_id;
                    }), self_loops.end());

            transitions.erase(
                std::remove_if(transitions.begin(), transitions.end(),
                    [&state_id, error](const Transition& t) {
                        return from_pin(t.from_state).value_or(error).id.state_id == state_id ||
                            from_pin(t.to_state).value_or(error).id.state_id == state_id;
                    }), transitions.end());

            states.erase(
                std::remove_if(states.begin(), states.end(),
                    [&state_id](const State& s) { return s.id.state_id == state_id; }), states.end());

        }

        static void remove_transition(const int transition_id) {
            transitions.erase(
                std::remove_if(transitions.begin(), transitions.end(),
                    [&transition_id](const Transition& t) { return t.id == transition_id; }), transitions.end());
        }

        static void add_self_loop(const int state_id) {

            Transition t{};
            constexpr char a = '\0';
            strncpy(t.label_buffer, &a, 128);
            t.id = state_id;
            t.from_state = state_id + 2;
            t.to_state = state_id + 1;
            self_loops.push_back(t);
        }

        static bool has_self_loop(const int state_id) {
            for (const Transition t: self_loops) {
                if (t.id == state_id)return true;
            }
            return false;
        }

        static void swap_final(const int state_id) {
            for (State& s: states) {
                if (s.id.state_id == state_id) {
                    s.final = !s.final;
                    break;
                }
            }
        }

        static void remove_self_loop(const int state_id) {
            self_loops.erase(
                std::remove_if(self_loops.begin(), self_loops.end(),[&state_id](const Transition& t) {return t.id == state_id;}), self_loops.end());
        }

        static std::optional<mdt::alphabet> alphabet() {
            std::vector<Transition> temp = transitions;
            if (temp.begin() == temp.end())
                temp = self_loops;
            if (temp.begin() == temp.end()) return std::optional<mdt::alphabet>();

            std::vector<Transition>::iterator it = temp.begin();
            int first_K = determine_K(std::string{it->label_buffer});

            mdt::alphabet A;
            for (it = transitions.begin(); it != transitions.end(); ++it) {
                // For each transition, check that it has a consistent number of symbols, if it does, initialize the alphabet, otherwise cancel
                if (determine_K(std::string{it->label_buffer}) != first_K) return std::nullopt;
                for (char c : it->label_buffer) {
                    if (c == '\0') break;
                    if (c == '/' || c == '*' || c == 'R' || c == 'L') continue;
                    A.add_symbol(c);
                }
            }

            for (it = self_loops.begin() ; it != self_loops.end(); ++it) {
                // For each transition, check that it has a consistent number of symbols, if it does, initialize the alphabet, otherwise cancel
                if (determine_K(std::string{it->label_buffer}) != first_K) return std::nullopt;
                for (char c : it->label_buffer) {
                    if (c == '\0') break;
                    if (c == '/' || c == '*' || c == 'R' || c == 'L') continue;
                    A.add_symbol(c);
                }
            }
            return A;
        }

        static void save_all_to_file(const std::string &file_path) {
            std::optional<mdt::alphabet> alph = FSM::alphabet();
            if (!alph.has_value()) return;

            int K;
            if (transitions.begin() != transitions.end())
                K = determine_K(std::string{transitions.begin()->label_buffer});
            else
                if (self_loops.begin() != self_loops.end())
                    K = determine_K(std::string{self_loops.begin()->label_buffer});
                else
                    return;

            int counter = 0;
            std::set<int> F;
            std::unordered_map<int, int> states_numbers;
            for (const State& s : states) {
                if (s.final) F.insert(counter);
                states_numbers.insert({s.id.state_id, counter++});
            }
            size_t symbols = alph.value().symbol_count();
            // Ok so here we know the transitions are correct, we just need to translate them, we also know the character count and state count
            json mdt;
            mdt["#Tapes"] = K;
            mdt["#States"] = FSM::states.size();
            mdt["#Symbols"] = symbols;
            mdt["FStates"] = F;

            json trans = json::array();

            // Translate the transitions
            std::vector<Transition>::iterator it;
            for (it = self_loops.begin(); it != self_loops.end(); ++it) {
                std::vector<int> x;

                std::string::iterator i = std::string{(it->label_buffer)}.begin();
                for (int j = 0; j < K; ++j) {
                    if (*i == '*') x.push_back(mdt::blank);
                    else x.push_back(alph->get_symbol(*i).value());
                    ++i;
                }
                ++i;
                std::vector<int> a;
                for (int j = 0; j < K; ++j) {
                    if (*i == '*') a.push_back(mdt::blank);
                    else if (*i == 'R') a.push_back( symbols);
                    else if (*i == 'L') a.push_back( symbols + 1);
                    else a.push_back(alph->get_symbol(*i).value());
                    ++i;
                }

                int q = states_numbers[it->id];
                json tmp = {
                    {"q", q},
                    {"x", x},
                    {"a", a},
                    {"t", q}
                };
                trans.push_back(tmp);
            }

            for (it = transitions.begin(); it != transitions.end(); ++it) {

                std::vector<int> x;

                std::string::iterator i = std::string{(it->label_buffer)}.begin();
                for (int j = 0; j < K; ++j) {
                    if (*i == '*') x.push_back(mdt::blank);
                    else x.push_back(alph->get_symbol(*i).value());
                    ++i;
                }
                ++i;
                std::vector<int> a;
                for (int j = 0; j < K; ++j) {
                    if (*i == '*') a.push_back(mdt::blank);
                    else if (*i == 'R') a.push_back( symbols);
                    else if (*i == 'L') a.push_back( symbols + 1);
                    else a.push_back(alph->get_symbol(*i).value());
                    ++i;
                }

                int q_in = states_numbers[from_pin(it->from_state).value().id.state_id];
                int q_out = states_numbers[from_pin(it->to_state).value().id.state_id];
                json tmp = {
                    {"q", q_in},
                    {"x", x},
                    {"a", a},
                    {"t", q_out}
                };
                trans.push_back(tmp);
            }

            mdt["Transitions"] = trans;

            // Tape
            mdt::tape t{strlen(content)};
            for (char x : content) {
                if (x == '\0') break;
                t.write(alph->get_symbol(x).value_or(mdt::blank));
                t.move_dx();
            }
            while (t.move_sx()) {}

            json j_alph = mdt::serialize_alphabet(alph.value());
            json j_tpe = mdt::serialize_tape(t);

            mdt::json_to_file(mdt, file_path);
            std::string alph_path = file_path.substr(0, file_path.find(".json")) + "_alph.json";
            mdt::json_to_file(j_alph, alph_path);
            std::string tpe_path = file_path.substr(0, file_path.find(".json")) + "_tpe.json";
            mdt::json_to_file(j_tpe, tpe_path);
        }

        static void load_all_from_file(std::string file_path) {

            const std::string alphabet_file = file_path.substr(0, file_path.find(".json")) + "_alph.json";
            const std::string tape_path = file_path.substr(0, file_path.find(".json")) + "_tpe.json";

            json j_TM;
            mdt::json_from_file(j_TM, file_path);

            json j_alph;
            mdt::json_from_file(j_alph, alphabet_file);

            json j_tpe;
            mdt::json_from_file(j_tpe, tape_path);

            std::optional<mdt::tape> opt_t = mdt::deserialize_tape(j_tpe);
            if (!opt_t.has_value()) return;
            mdt::tape t = opt_t.value();

            std::optional<mdt::alphabet> opt_a = mdt::deserialize_alphabet(j_alph);
            if (!opt_a.has_value()) return;
            mdt::alphabet a = opt_a.value();

            constexpr char rep = '\0';
            strncpy(content, &rep, 500);
            mdt::symbol read;
            for (int i = 0; i < 500; i++) {
                read = t.read();
                FSM::content[i] = a.get_representation(read).value_or('\0');
                t.move_dx();
            }

            states.clear();
            FSM::init();
            for (int i = 0; i < j_TM["#States"].get<int>(); i++) {
                FSM::add_state(ImVec2(i * 30, 0));
            }

            std::set<mdt::state> finals = j_TM["FStates"].get<std::set<mdt::state>>();
            for (mdt::state s : finals) {
                FSM::states[ s ].final = true;
            }
            self_loops.clear();
            transitions.clear();

            json trans = j_TM["Transitions"].get<json>();
            std::vector<mdt::symbol> x, y;
            for (auto w = trans.begin(); w != trans.end(); ++w) {
                mdt::state q = (*w)["q"].get<mdt::state>();
                x = (*w)["x"].get<std::vector<mdt::symbol>>();
                mdt::state e = (*w)["t"].get<mdt::state>();
                y = (*w)["a"].get<std::vector<mdt::symbol>>();

                write_transition(q, x, y, e, a);
            }

        }

    private:
        static void write_transition(int q, std::vector<int> x, std::vector<int> y, int e, mdt::alphabet a) {
            std::stringstream ss;
            for (int x_i : x) {
                ss << a.get_representation(x_i).value_or('*');
            }
            ss << '/';
            for (int y_i : y) {
                if (y_i == a.symbol_count()) ss << 'R';
                else if (y_i == a.symbol_count() + 1) ss << 'L';
                else ss << a.get_representation(y_i).value_or('*');
            }

            const std::string label = ss.str();

            // self loop
            if (q == e) {
                FSM::add_self_loop(q * 3 + 1);
                strncpy(FSM::self_loops.rbegin()->label_buffer, label.c_str(), 128);
            } else {
                FSM::add_transition((q * 3 + 3), (e * 3 + 2));
                strncpy(FSM::transitions.rbegin()->label_buffer, label.c_str(), 128);
            }
        }
    };


}

#endif //GUI_APP_HELPERS_H
