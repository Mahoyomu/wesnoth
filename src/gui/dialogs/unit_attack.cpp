/*
	Copyright (C) 2010 - 2024
	by Mark de Wever <koraq@xs4all.nl>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/


#include "gui/dialogs/unit_attack.hpp"

#include "font/text_formatting.hpp"
#include "gui/auxiliary/find_widget.hpp"
#include "gui/dialogs/attack_predictions.hpp"
#include "gui/widgets/button.hpp"
#include "gui/widgets/listbox.hpp"
#include "gui/widgets/unit_preview_pane.hpp"
#include "gui/widgets/window.hpp"
#include "game_config.hpp"
#include "gettext.hpp"
#include "language.hpp"
#include "color.hpp"
#include "units/unit.hpp"

#include <functional>

namespace gui2::dialogs
{

REGISTER_DIALOG(unit_attack)

unit_attack::unit_attack(const unit_map::iterator& attacker_itor,
						   const unit_map::iterator& defender_itor,
						   std::vector<battle_context>&& weapons,
						   const int best_weapon,
						   std::vector<gui2::widget_data>& data_vector)
	: modal_dialog(window_id())
	, selected_weapon_(-1)
	, attacker_itor_(attacker_itor)
	, defender_itor_(defender_itor)
	, weapons_(std::move(weapons))
	, best_weapon_(best_weapon)
	, data_vector_(data_vector)
{
}

void unit_attack::damage_calc_callback()
{
	const std::size_t index = find_widget<listbox>(get_window(), "weapon_list", false).get_selected_row();
	attack_predictions::display(weapons_[index], attacker_itor_.get_shared_ptr(), defender_itor_.get_shared_ptr());
}

void unit_attack::pre_show(window& window)
{
	connect_signal_mouse_left_click(
			find_widget<button>(&window, "damage_calculation", false),
			std::bind(&unit_attack::damage_calc_callback, this));

	find_widget<unit_preview_pane>(&window, "attacker_pane", false)
		.set_displayed_unit(*attacker_itor_);

	find_widget<unit_preview_pane>(&window, "defender_pane", false)
		.set_displayed_unit(*defender_itor_);

	selected_weapon_ = -1;

	listbox& weapon_list = find_widget<listbox>(&window, "weapon_list", false);
	window.keyboard_capture(&weapon_list);

	// Possible TODO: If a "blank weapon" is generally useful, add it as a static member in attack_type.
	for (widget_data& data : data_vector_)
		weapon_list.add_row(data);

	// If these two aren't the same size, we can't use list selection incides
	// to access to weapons list!
	assert(weapons_.size() == weapon_list.get_item_count());

	weapon_list.select_row(best_weapon_);
}

void unit_attack::post_show(window& window)
{
	if(get_retval() == retval::OK) {
		selected_weapon_ = find_widget<listbox>(&window, "weapon_list", false).get_selected_row();
	}
}


} // namespace dialogs
