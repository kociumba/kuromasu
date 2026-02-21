#ifndef INPUT_H
#define INPUT_H

#include "common.h"

ImVec2 get_relative_mouse(const state_t& s, ImVec2 window_origin);
ktl::pos2_size get_cell_under_mouse(const state_t& s, ImVec2 window_origin);
void undo_action(state_t& s);
void redo_action(state_t& s);

bool is_ctrl_down();
bool is_shift_down();

void mouse_input(state_t& s, ImVec2 window_origin);
void keyboard_input(state_t& s);

#endif /* INPUT_H */
