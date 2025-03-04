/*
	Copyright (C) 2003 - 2025
	by David White <dave@whitevine.net>
	Part of the Battle for Wesnoth Project https://www.wesnoth.org/

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY.

	See the COPYING file for more details.
*/

#pragma once

#include "config.hpp"
#include "game_end_exceptions.hpp"
#include "map/location.hpp"
#include "mt_rng.hpp"
#include "variable_info.hpp"

class scoped_wml_variable;

class game_data  : public variable_set  {
public:
	explicit game_data(const config& level);
	game_data(const game_data& data);

	std::vector<scoped_wml_variable*> scoped_variables;

	const config& get_variables() const { return variables_; }
	/** throws invalid_variablename_exception if varname is no valid variable name. */
	config::attribute_value &get_variable(const std::string &varname);
	/** returns a blank attribute value if varname is no valid variable name. */
	virtual config::attribute_value get_variable_const(const std::string& varname) const;
	/** throws invalid_variablename_exception if varname is no valid variable name. */
	config& get_variable_cfg(const std::string& varname);
	/** does nothing if varname is no valid variable name. */
	void set_variable(const std::string& varname, const t_string& value);
	/** throws invalid_variablename_exception if varname is no valid variable name. */
	config& add_variable_cfg(const std::string& varname, const config& value=config());
	/** returns a variable_access that cannot be used to change the game variables */
	variable_access_const get_variable_access_read(const std::string& varname) const
	{
		activate_scope_variable(varname);
		return variable_access_const(varname, variables_);
	}
	/** returns a variable_access that can be used to change the game variables */
	variable_access_create get_variable_access_write(const std::string& varname)
	{
		activate_scope_variable(varname);
		return variable_access_create(varname, variables_);
	}
	/**
	 * Clears attributes config children
	 * does nothing if varname is no valid variable name.
	 */
	void clear_variable(const std::string& varname);
	/**
	 * Clears only the config children
	 * does nothing if varname is no valid variable name.
	 */
	void clear_variable_cfg(const std::string& varname);

	const randomness::mt_rng& rng() const { return rng_; }
	randomness::mt_rng& rng() { return rng_; }

	enum PHASE {
		/// creating intitial [unit]s, executing toplevel [lua] etc.
		/// next phase: PRELOAD
		INITIAL,
		/// the preload [event] is fired
		/// next phase: PRESTART (normal game), TURN_STARTING_WAITING (reloaded game), TURN_PLAYING (reloaded game) or GAME_ENDED (reloadedgame)
		PRELOAD,
		/// the prestart [event] is fired
		/// next phase: START (default), GAME_ENDING
		PRESTART,
		/// the start [event] is fired
		/// next phase: TURN_STARTING_WAITING (default), GAME_ENDING
		START,
		/// we are waiting for the turn to start.
		/// The game can be saved here.
		/// next phase: TURN_STARTING
		TURN_STARTING_WAITING,
		/// the turn, side turn etc. [event]s are being fired
		/// next phase: TURN_PLAYING (default), GAME_ENDING
		TURN_STARTING,
		/// The User is controlling the game and invoking actions
		/// The game can be saved here.
		/// next phase: TURN_PLAYING (default), GAME_ENDING
		TURN_PLAYING,
		/// The turn_end, side_turn_end etc [events] are fired
		/// next phase: TURN_STARTING_WAITING (default), GAME_ENDING
		TURN_ENDED,
		/// The victory etc. [event]s are fired.
		/// next phase: GAME_ENDED
		GAME_ENDING,
		/// The game has ended and the user is observing the final state "lingering"
		/// The game can be saved here.
		GAME_ENDED,
	};

	PHASE phase() const { return phase_; }
	void set_phase(PHASE phase) { phase_ = phase; }
	/// returns where there is currently a well defiend "current player",
	/// that is for example not the case during start events or during linger mode.
	bool has_current_player() const;
	bool is_before_screen() const;
	bool is_after_start() const;

	static PHASE read_phase(const config& cfg);
	static void write_phase(config& cfg, game_data::PHASE phase);

	const t_string& cannot_end_turn_reason() {
		return cannot_end_turn_reason_;
	}
	bool allow_end_turn() const { return can_end_turn_; }
	void set_allow_end_turn(bool value, const t_string& reason = "") {
		can_end_turn_ = value;
		cannot_end_turn_reason_ = reason;
	}

	/** the last location where a select event fired. Used by wml menu items with needs_select=yes*/
	map_location last_selected;

	void write_snapshot(config& cfg) const;

	const std::string& next_scenario() const { return next_scenario_; }
	void set_next_scenario(const std::string& next_scenario) { next_scenario_ = next_scenario; }

	const std::string& get_id() const { return id_; }
	void set_id(const std::string& value) { id_ = value; }

	const std::string& get_theme() const { return theme_; }
	void set_theme(const std::string& value) { theme_ = value; }

	const std::vector<std::string>& get_defeat_music() const { return defeat_music_; }
	void set_defeat_music(std::vector<std::string> value) { defeat_music_ = std::move(value); }

	const std::vector<std::string>& get_victory_music() const { return victory_music_; }
	void set_victory_music(std::vector<std::string> value) { victory_music_ = std::move(value); }

	void set_end_turn_forced(bool v) { end_turn_forced_ = v; }
	bool end_turn_forced() const { return end_turn_forced_; }
private:
	void activate_scope_variable(std::string var_name) const;
	/** Used to delete variables. */
	variable_access_throw get_variable_access_throw(const std::string& varname)
	{
		activate_scope_variable(varname);
		return variable_access_throw(varname, variables_);
	}

	randomness::mt_rng rng_;
	config variables_;
	PHASE phase_;
	bool can_end_turn_;
	bool end_turn_forced_;
	t_string cannot_end_turn_reason_;
	/** the scenario coming next (for campaigns) */
	std::string next_scenario_;
	// the id of a scenario cannot change during a scenario
	std::string id_;
	std::string theme_;
	std::vector<std::string> defeat_music_;
	std::vector<std::string> victory_music_;
};
