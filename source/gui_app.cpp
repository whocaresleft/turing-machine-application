/*
 * Copyright (c) 2025 Biribo' Francesco
 *
 * Permission to use, copy, modify, and distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>
#include <cmath>
#include <imnodes/imnodes.h>
#include "imnodes_internal.h"
#include <imguifiledialog/ImGuiFileDialog.h>
#include "../header/gui_app_helpers.h"

using namespace gui_app;
namespace gui_app {
    std::vector<State> FSM::states;
    std::vector<Transition> FSM::transitions;
    std::vector<Transition> FSM::self_loops;
    int FSM::next_node_id = 1;
    int FSM::next_link_id = 20000;
    char FSM::content[500];
}
void draw_self_loop(ImDrawList* draw_list, const ImVec2 node_pos, Transition t, const ImU32 color, const float thickness);
int main () {
    const char a = '\0';
    strncpy(FSM::content, &a, 500);
    // Init GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    // Richiedi un contesto OpenGL 3.3 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Crea la finestra
    GLFWwindow* window = glfwCreateWindow(1280, 720, "Turing Machine Application", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // VSync

    // (GL loader qui, se usi glad/glew — saltato in questo esempio)


    // Init ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImNodes::CreateContext();
    ImNodes::GetStyle().Flags |= ImNodesStyleFlags_GridLines;
    ImNodes::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Loop that renders the actual canvas
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (FSM::states.empty()) {
            FSM::init();
        }

        /*-------------------------------Main Window------------------------------------------------------------------*/
        ImGui::Begin("Turing Machine Creator");
            /*-----First panel----------------------*/
            ImGui::BeginChild("first-panel", ImVec2(680, 520), true);
                /*-----Sub panel 1-----*/
                ImGui::BeginChild("buttons", ImVec2(660, 50), true);
                if (ImGui::Button("Add State"))
                    FSM::add_state(ImGui::GetMousePos());
                ImGui::SameLine();
                bool at_least_one_node_selected = ImNodes::NumSelectedNodes() > 0;
                ImGui::BeginDisabled(!at_least_one_node_selected);
                if (ImGui::Button("Remove state") && at_least_one_node_selected) {
                    int selected_node[ImNodes::NumSelectedNodes()];
                    ImNodes::GetSelectedNodes(selected_node);
                    for (const int selected : selected_node) {
                        FSM::remove_state(selected);
                    }
                }
                ImGui::EndDisabled();

                ImGui::SameLine();
                bool just_one_node_selected = ImNodes::NumSelectedNodes() == 1;
                ImGui::BeginDisabled(!just_one_node_selected);
                if (ImGui::Button("Self Loop") && just_one_node_selected) {
                    int selected_node;
                    ImNodes::GetSelectedNodes(&selected_node);
                    if (FSM::has_self_loop(selected_node))
                        FSM::remove_self_loop(selected_node);
                    else
                        FSM::add_self_loop(selected_node);
                }
                ImGui::EndDisabled();

                ImGui::SameLine();
                ImGui::BeginDisabled(!just_one_node_selected);
                if (ImGui::Button("Final") && just_one_node_selected) {
                    int selected_node;
                    ImNodes::GetSelectedNodes(&selected_node);
                    FSM::swap_final(selected_node);
                }
                ImGui::EndDisabled();

                ImGui::SameLine();
                if (ImGui::Button("Save")) {
                    ImGuiFileDialog::Instance() -> OpenDialog("save-tm", "Save", ".json", IGFD::FileDialogConfig{.path = ".",});
                }
                if (ImGuiFileDialog::Instance() -> Display("save-tm")) {
                    if (ImGuiFileDialog::Instance()->IsOk()) {
                        std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
                        FSM::save_all_to_file(file_path);
                    }
                    ImGuiFileDialog::Instance()->Close();
                }

                ImGui::SameLine();
                if (ImGui::Button("Load")) {
                    ImGuiFileDialog::Instance() -> OpenDialog("load-tm", "Load", ".json", IGFD::FileDialogConfig{.path = ".",});
                }
                if (ImGuiFileDialog::Instance() -> Display("load-tm")) {
                    if (ImGuiFileDialog::Instance()->IsOk()) {
                        std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();
                        FSM::load_all_from_file(file_path);
                    }
                    ImGuiFileDialog::Instance()->Close();
                }

                // Add one more transition
                ImGui::SameLine();
                bool one_selected = (ImNodes::NumSelectedLinks() == 1) ^ (ImNodes::NumSelectedNodes() == 1);
                ImGui::BeginDisabled(!one_selected);
                if (ImGui::Button("+") && one_selected) {
                    if (ImNodes::NumSelectedLinks() == 1) {
                        int link_id;
                        ImNodes::GetSelectedLinks(&link_id);
                        FSM::one_more_label(link_id, false);
                    } else {
                        int self_loop_id;
                        ImNodes::GetSelectedNodes(&self_loop_id);
                        FSM::one_more_label(self_loop_id, true);
                    }
                }
                ImGui::EndDisabled();

                // Remove one transition
                ImGui::SameLine();
                ImGui::BeginDisabled(!one_selected);
                if (ImGui::Button("-") && one_selected) {
                    if (ImNodes::NumSelectedLinks() == 1) {
                        int link_id;
                        ImNodes::GetSelectedLinks(&link_id);
                        FSM::one_less_label(link_id, false);
                    } else {
                        int self_loop_id;
                        ImNodes::GetSelectedNodes(&self_loop_id);
                        FSM::one_less_label(self_loop_id, true);
                    }
                }
                ImGui::EndDisabled();

                ImGui::EndChild();
                /*-------------------*/
                /*-----Sub panel 2-----*/
                ImGui::BeginChild("fsm-canvas", ImVec2(660, 450), true);

                // We first draw all the states that were created
                ImNodes::BeginNodeEditor();

                for (const State& s : FSM::states) {
                    if (s.final) {
                        ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(200, 0, 0, 255));
                        ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(255, 50, 50, 255));
                        ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(255, 100, 100, 255));
                    } else {
                        ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(50, 100, 200, 255));
                        ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(75, 125, 255, 255));
                        ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(100, 150, 255, 255));
                    }

                    ImNodes::BeginNode(s.id.state_id);

                    ImNodes::BeginNodeTitleBar();
                    ImGui::Text(s.label.c_str());
                    ImNodes::EndNodeTitleBar();

                    ImNodes::BeginInputAttribute(s.id.in_id);
                    ImGui::Text("< In");
                    ImNodes::EndInputAttribute();

                    ImNodes::BeginOutputAttribute(s.id.out_id);
                    ImGui::Text("Out >");
                    ImNodes::EndOutputAttribute();

                    s.position = ImNodes::GetNodeGridSpacePos(s.id.state_id);

                    ImNodes::EndNode();

                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();
                    ImNodes::PopColorStyle();
                }

                // Draw all the links
                for (const Transition& t : FSM::transitions) {
                    ImNodes::Link(t.id, t.from_state, t.to_state);
                    ImVec2 pos1 = ImNodes::GetNodeScreenSpacePos(FSM::from_pin(t.from_state).value().id.state_id);
                    ImVec2 pos2 = ImNodes::GetNodeScreenSpacePos(FSM::from_pin(t.to_state).value().id.state_id);
                    ImVec2 midpoint = ImVec2(
                    pos1.x + (pos2.x - pos1.x) * 0.5f,
                    pos1.y + (pos2.y - pos1.y) * 0.5f );

                    // Does the inverse exits?
                    bool ascending = FSM::from_pin(t.from_state).value().id.state_id < FSM::from_pin(t.to_state).value().id.state_id;

                    // Adjust for text size (centering)
                    const ImVec2 text_size = ImGui::CalcTextSize(t.labels[0].data());
                    ImVec2 text_pos = ascending ? ImVec2(
                        midpoint.x - text_size.x * 0.5f,
                        midpoint.y + text_size.y * 0.5f
                        ) : ImVec2(
                        midpoint.x - text_size.x * 0.5f,
                        midpoint.y + text_size.y * 5.0f // Meh kinda
                        );

                    // Draw the text label
                    for (int i = 0; i < t.labels.size(); i++) {
                        ImDrawList* draw_list = ImGui::GetWindowDrawList();
                        draw_list->AddText(
                            text_pos + ImVec2(0, -25 * i),
                            IM_COL32(220, 220, 220, 255),  // White color
                            t.labels[i].data()
                        );
                    }
                }

                ImNodes::EndNodeEditor();

                // Draw self loop links, I do it here so it draws a bit of the transition behind the state and not on top
                for (const Transition& t : FSM::self_loops) {
                    const ImVec2 pos = ImNodes::GetNodeScreenSpacePos(t.id);
                    draw_self_loop(ImGui::GetWindowDrawList(), pos, t,IM_COL32(55, 112, 185, 245), 2.0f);
                }

                // Handle link creation
                int from_pin, to_pin;
                if (ImNodes::IsLinkCreated(&from_pin, &to_pin)) {
                    FSM::add_transition(from_pin, to_pin);
                }

                // Handle link deletion
                if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
                    int selected;
                    if (ImNodes::NumSelectedLinks() == 1) {
                        ImNodes::GetSelectedLinks(&selected);
                        FSM::remove_transition(selected);
                    }
                }

                ImGui::EndChild();
                /*-------------------*/
            ImGui::EndChild();
            /*--------------------------------------*/
            ImGui::SameLine();
            /*-----Second panel---------------------*/
            ImGui::BeginChild("second-panel", ImVec2(300, 520), true);
                /*-----Sub panel 1-----*/
                ImGui::BeginChild("transition-list", ImVec2(290, 450), true);
                for (const auto & transition : FSM::transitions) {
                    ImGui::PushID(transition.id);
                    for (int i = 0; i < transition.labels.size(); i++) {
                        ImGui::PushID(i);
                        ImGui::Text("%s => %s", FSM::from_pin(transition.from_state).value().label.c_str() , FSM::from_pin(transition.to_state).value().label.c_str());
                        ImGui::SameLine();
                        ImGui::InputText("", transition.labels[i].data(), 128);
                        ImGui::PopID();
                    }
                    ImGui::Separator();
                    ImGui::PopID();
                }
                for(const auto & self : FSM::self_loops) {
                    ImGui::PushID(self.id);
                    for (int i = 0; i < self.labels.size(); i++) {
                        ImGui::PushID(i);
                        ImGui::Text("%s => %s", FSM::from_pin(self.from_state).value().label.c_str() , FSM::from_pin(self.from_state).value().label.c_str());
                        ImGui::SameLine();
                        ImGui::InputText("", self.labels[i].data(), 128);
                        ImGui::PopID();
                    }
                    ImGui::Separator();
                    ImGui::PopID();
                }
                ImGui::EndChild();
                /*-------------------*/
                /*-----Sub panel 2-----*/
                ImGui::BeginChild("input_string", ImVec2(290, 45), true);
                ImGui::PushID(99999);
                ImGui::Text("Input:");
                ImGui::SameLine();
                ImGui::InputText("", FSM::content, 500);
                ImGui::PopID();
                ImGui::EndChild();
                /*-------------------*/
            ImGui::EndChild();
            /*--------------------------------------*/
        ImGui::End();
        /*------------------------------------------------------------------------------------------------------------*/

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImNodes::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void draw_self_loop(ImDrawList* draw_list, const ImVec2 node_pos, Transition t, const ImU32 color, const float thickness) {
    const ImVec2 start = node_pos + ImVec2(20, 20);
    const ImVec2 end = node_pos + ImVec2(0, 20);
    const ImVec2 control1 = node_pos + ImVec2(50, -60);
    const ImVec2 control2 = node_pos + ImVec2(-50, -60);

    const ImVec2 dir = ImVec2(end.x - control2.x, end.y - control2.y);
    float len = sqrt(dir.x * dir.x + dir.y * dir.y);
    ImVec2 norm = ImVec2(dir.x / len, dir.y / len);

    ImVec2 p1 = end;
    ImVec2 p2 = end - ImVec2(norm.y, -norm.x) * 6.0f - norm * 10.0f;
    ImVec2 p3 = end + ImVec2(norm.y, -norm.x) * 6.0f - norm * 10.0f;

    // Draw the text label
    for (int i = 0; i < t.labels.size(); i++) {
        draw_list->AddText(
            node_pos + ImVec2(-20, -75) + ImVec2(0, -25 * i),
            IM_COL32(220, 220, 220, 255),
            t.labels[i].data()
        );
    }

    draw_list->AddBezierCubic(start, control1, control2, end, color, thickness);
    draw_list->AddTriangleFilled(p1, p2, p3, color);
}