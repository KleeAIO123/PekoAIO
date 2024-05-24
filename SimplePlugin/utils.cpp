#include "../plugin_sdk/plugin_sdk.hpp"
#include "utils.h"

namespace utils
{

	// Declaration of menu objects
	TreeTab* main_tab = nullptr;

	// Delays map
	std::map<spellslot, float> delays;

	namespace developer
	{
		TreeEntry* debug_mode = nullptr;
	}

	void on_load()
	{
		std::string data = myhero->get_model();
		std::transform(data.begin(), data.end(), data.begin(),
			[](unsigned char c) { return std::tolower(c); });

		main_tab = menu->get_tab(data);
		if (main_tab != nullptr)
		{
			auto developer = main_tab->add_tab(myhero->get_model() + ".aio.settings", "AIO Settings");
			{
				developer->add_separator(myhero->get_model() + ".aio.version", "ShadowAIO : 10/21/2022");
				developer::debug_mode = developer->add_checkbox(myhero->get_model() + ".developer.debug_mode", "Debug Mode", false);
				developer::debug_mode->set_texture(myhero->get_passive_icon_texture());
				developer::debug_mode->is_hidden() = myhero->get_champion() != champion_id::Rengar;

				developer->add_separator(myhero->get_model() + ".aio.separator1", "");
				developer->add_separator(myhero->get_model() + ".aio.separator2", "Created by: kuezese");
				developer->add_separator(myhero->get_model() + ".aio.separator3", "Discord: racism gaming#0375");

				if (myhero->get_champion() == champion_id::Yasuo)
				{
					developer->add_separator(myhero->get_model() + ".aio.separator4", "");
					developer->add_separator(myhero->get_model() + ".aio.separator5", "Helped with Yasuo:");
					developer->add_separator(myhero->get_model() + ".aio.separator6", "Klee");
					developer->add_separator(myhero->get_model() + ".aio.separator7", "div");
					developer->add_separator(myhero->get_model() + ".aio.separator8", "KissMyMp5");
					developer->add_separator(myhero->get_model() + ".aio.separator9", "");
				}
			}
		}

		std::string msg = "<b><font color=\"#7289da\">[ShadowAIO]</font></b><font color=\"#FFFFFF\">: Loaded champion</font> <b><font color=\"#7289da\">" + myhero->get_model() + "</font></b>";
		myhero->print_chat(1, msg.c_str());
		msg = "<b><font color=\"#7289da\">[ShadowAIO]</font></b><font color=\"#FFFFFF\">: Bugs and suggestions please send on Discord -</font> <b><font color=\"#7289da\">racism gaming#0375</font></b>";
		myhero->print_chat(1, msg.c_str());
	}

	bool has_unkillable_buff(game_object_script target)
	{
		return target->is_zombie() || target->has_buff({
			buff_hash("UndyingRage"),
			buff_hash("ChronoShift"),
			buff_hash("KayleR"),
			buff_hash("KindredRNoDeathBuff"),
			buff_hash("VladimirSanguinePool")
			});
	}

	bool has_untargetable_buff(game_object_script target)
	{
		return target->has_buff({ buff_hash("JaxCounterStrike"), buff_hash("ShenW") });
	}

	bool has_crowd_control_buff(game_object_script target)
	{
		return target->has_buff_type({
			buff_type::Stun,
			buff_type::Knockup,
			buff_type::Asleep,
			buff_type::Berserk,
			buff_type::Charm,
			buff_type::Flee,
			buff_type::Fear,
			buff_type::Snare,
			buff_type::Suppression,
			buff_type::Polymorph,
			buff_type::Taunt
			});
	}

	bool is_empowered()
	{
		return myhero->get_mana() >= 4.0f;
	}

	bool fast_cast(script_spell* spell, bool delay)
	{
		if (delay && gametime->get_time() < delays[spell->slot] + sciprt_spell_wait)
			return false;

		if (main_tab != nullptr && developer::debug_mode->get_bool())
		{
			myhero->print_chat(1, "[Debug] Fast Cast (spell) %s ready: %s empowered: %s", spell->name().c_str(), spell->is_ready() ? "Yes" : "No", is_empowered() ? "Yes" : "No");
		}

		myhero->cast_spell(spell->slot, true, spell->is_charged_spell);
		delays[spell->slot] = gametime->get_time();
		return true;
	}

	bool cast(spellslot slot, bool is_charged_spell)
	{
		if (gametime->get_time() < delays[slot] + sciprt_spell_wait)
			return false;

		myhero->cast_spell(slot, true, is_charged_spell);
		delays[slot] = gametime->get_time();
		return true;
	}

	bool cast(spellslot slot, game_object_script unit, bool is_charged_spell)
	{
		if (gametime->get_time() < delays[slot] + sciprt_spell_wait)
			return false;

		myhero->cast_spell(slot, unit, true, is_charged_spell);
		delays[slot] = gametime->get_time();
		return true;
	}

	bool fast_cast(script_spell* spell, vector position, bool delay)
	{
		if (delay && gametime->get_time() < delays[spell->slot] + sciprt_spell_wait)
			return false;

		if (main_tab != nullptr && developer::debug_mode->get_bool())
		{
			myhero->print_chat(1, "[Debug] Fast Cast (spell, position) %s ready: %s empowered: %s", spell->name().c_str(), spell->is_ready() ? "Yes" : "No", is_empowered() ? "Yes" : "No");
		}

		if (!spell->is_charged_spell)
		{
			myhero->cast_spell(spell->slot, position);
			delays[spell->slot] = gametime->get_time();
			return true;
		}
		if (spell->is_charging() && gametime->get_time() - spell->charging_started_time > 0.f)
		{
			myhero->update_charged_spell(spell->slot, position, true);
			delays[spell->slot] = gametime->get_time();
			return true;
		}
		return spell->start_charging();
	}

	bool cast(spellslot slot, vector position, bool is_charged_spell)
	{
		if (gametime->get_time() < delays[slot] + sciprt_spell_wait)
			return false;

		myhero->cast_spell(slot, position, true, is_charged_spell);

		delays[slot] = gametime->get_time();
		return true;
	}

	bool fast_cast(script_spell* spell, game_object_script unit, hit_chance minimum, bool aoe, int min_targets, bool delay)
	{
		if (delay && gametime->get_time() < delays[spell->slot] + sciprt_spell_wait)
			return false;

		if (main_tab != nullptr && developer::debug_mode->get_bool())
		{
			myhero->print_chat(1, "[Debug] Fast Cast (spell, unit) %s unit: %s ready: %s empowered: %s", spell->name().c_str(), unit->get_name_cstr(), spell->is_ready() ? "Yes" : "No", is_empowered() ? "Yes" : "No");
		}

		vector cast_position;

		prediction_input x;

		if (!spell->from.is_valid())
			x._from = myhero->get_position();
		else
			x._from = spell->from;

		x.unit = unit;
		x.delay = spell->delay;
		x.radius = spell->radius;
		x.speed = spell->speed;
		x.collision_objects = spell->collision_flags;
		x.range = spell->range();
		x.type = spell->type;
		x.aoe = aoe;
		x.spell_slot = spell->slot;
		x.use_bounding_radius = spell->type != skillshot_type::skillshot_circle;

		auto output = prediction->get_prediction(&x);

		if (output.hitchance >= minimum && output.aoe_targets_hit_count() >= min_targets)
		{
			cast_position = output.get_cast_position();

			if (!spell->is_charged_spell)
			{
				myhero->cast_spell(spell->slot, cast_position);
				delays[spell->slot] = gametime->get_time();
				return true;
			}

			if (spell->is_charging() && gametime->get_time() - spell->charging_started_time > 0.f)
			{
				myhero->update_charged_spell(spell->slot, cast_position, true);
				delays[spell->slot] = gametime->get_time();
				return true;
			}
		}
		return false;
	}

	bool fast_cast(script_spell* spell, int minMinions, bool is_jugnle_mobs, bool delay)
	{
		if (delay && gametime->get_time() < delays[spell->slot] + sciprt_spell_wait)
			return false;

		auto best_pos = spell->get_cast_on_best_farm_position(minMinions, is_jugnle_mobs);

		if (best_pos.is_valid())
		{
			if (!spell->is_charged_spell)
			{
				if (developer::debug_mode->get_bool())
				{
					myhero->print_chat(1, "[Debug] Fast Cast (spell, farm) %s ready: %s", spell->name().c_str(), spell->is_ready() ? "Yes" : "No");
				}
				myhero->cast_spell(spell->slot, best_pos);
				delays[spell->slot] = gametime->get_time();
				return true;
			}

			if (spell->is_charging() && gametime->get_time() - spell->charging_started_time > 0.f)
			{
				myhero->update_charged_spell(spell->slot, best_pos, true);
				delays[spell->slot] = gametime->get_time();
				return true;
			}
		}

		return false;
	}

	bool is_ready(spellslot slot)
	{
		auto spellInfo = myhero->get_spell(slot);

		if (spellInfo && spellInfo->is_learned())
		{
			auto spell_state = myhero->get_spell_state(slot);

			if (spell_state == 2)
			{
				return true;
			}
		}

		return false;
	}

	float get_current_attackspeed(float base_as, float as_ratio)
	{
		return 1.0f / (base_as * (1.f + myhero->mPercentAttackSpeedMod() * (as_ratio / base_as)));
	}

	vector to_2d(vector vec)
	{
		return vector(vec.x, vec.y);
	}

	vector to_3d(vector vec)
	{
		return vector(vec.x, vec.y, myhero->get_position().z);
	}

	vector to_3d2(vector vec)
	{
		return vector(vec.x, vec.y, navmesh->get_height_for_position(vec.x, vec.y));
	}

	vector add(vector source, float add)
	{
		return vector(source.x + add, source.y + add);
	}

	void draw_dmg_rl(game_object_script target, float damage, unsigned long color)
	{
		if (target != nullptr && target->is_valid() && target->is_visible_on_screen() && target->is_hpbar_recently_rendered())
		{
			auto bar_pos = target->get_hpbar_pos();

			if (bar_pos.is_valid() && !target->is_dead() && target->is_visible())
			{
				const auto health = target->get_health();

				bar_pos = vector(bar_pos.x + (105 * (health / target->get_max_health())), bar_pos.y -= 10);

				auto damage_size = (105 * (damage / target->get_max_health()));

				if (damage >= health)
				{
					damage_size = (105 * (health / target->get_max_health()));
				}

				if (damage_size > 105)
				{
					damage_size = 105;
				}

				const auto size = vector(bar_pos.x + (damage_size * -1), bar_pos.y + 11);

				draw_manager->add_filled_rect(bar_pos, size, color);
			}
		}
	}

	float get_damage(game_object_script target, std::vector<script_spell*> spells, int include_aa)
	{
		float damage = 0.0f;

		for (auto&& spell : spells)
		{
			if (spell->is_ready())
			{
				damage += spell->get_damage(target);
			}
		}

		if (include_aa != 0)
		{
			damage += myhero->get_auto_attack_damage(target);
		}

		return damage;
	}


	int32_t count_enemies_in_range(vector vec, float range, bool include_bounding_radius)
	{
		auto count = 0;

		for (auto&& enemy : entitylist->get_enemy_heroes())
		{
			if (!enemy->is_dead() && enemy->is_visible() && enemy->is_targetable() && myhero->get_distance(enemy) < range + (include_bounding_radius ? enemy->get_bounding_radius() : 0))
				count++;
		}

		return count;
	}

	int32_t count_minions_in_range(float range)
	{
		auto count = 0;

		for (auto&& minion : entitylist->get_enemy_minions())
		{
			if (!minion->is_dead() && minion->is_visible() && minion->is_targetable() && myhero->get_distance(minion) < range)
				count++;
		}

		return count;
	}

	int32_t count_minions_in_range(game_object_script target, float range)
	{
		auto count = 0;

		for (auto&& minion : (target->is_ally() ? entitylist->get_ally_minions() : entitylist->get_enemy_minions()))
		{
			if (target->get_handle() != minion->get_handle() && !minion->is_dead() && minion->is_visible() && minion->is_targetable() && target->get_distance(minion) < range)
				count++;
		}

		return count;
	}

	int32_t count_minions_in_range(vector vec, float range, bool include_bounding_radius)
	{
		auto count = 0;

		for (auto&& minion : entitylist->get_enemy_minions())
		{
			if (!minion->is_dead() && minion->is_visible() && minion->is_targetable() && vec.distance(minion) < range + (include_bounding_radius ? minion->get_bounding_radius() : 0))
				count++;
		}

		return count;
	}

	int32_t count_monsters_in_range(float range)
	{
		auto count = 0;

		for (auto&& minion : entitylist->get_jugnle_mobs_minions())
		{
			if (!minion->is_dead() && minion->is_visible() && minion->is_targetable() && myhero->get_distance(minion) < range)
				count++;
		}

		return count;
	}

	int32_t count_monsters_in_range(game_object_script target, float range)
	{
		auto count = 0;

		for (auto&& minion : entitylist->get_jugnle_mobs_minions())
		{
			if (target->get_handle() != minion->get_handle() && !minion->is_dead() && minion->is_visible() && minion->is_targetable() && target->get_distance(minion) < range)
				count++;
		}

		return count;
	}

	int32_t count_monsters_in_range(vector vec, float range, bool include_bounding_radius)
	{
		auto count = 0;

		for (auto&& minion : entitylist->get_jugnle_mobs_minions())
		{
			if (!minion->is_dead() && minion->is_visible() && minion->is_targetable() && vec.distance(minion) < range + (include_bounding_radius ? minion->get_bounding_radius() : 0))
				count++;
		}

		return count;
	}

	game_object_script get_closest_target(float range)
	{
		auto targets = entitylist->get_enemy_heroes();

		targets.erase(std::remove_if(targets.begin(), targets.end(), [range](game_object_script x)
			{
				return !x->is_valid_target(range) || x->is_dead();
			}), targets.end());

		std::sort(targets.begin(), targets.end(), [](game_object_script a, game_object_script b)
			{
				return a->get_distance(myhero) < b->get_distance(myhero);
			});

		if (!targets.empty())
			return targets.front();

		return nullptr;
	}

	game_object_script get_closest_target(float range, vector from)
	{
		auto targets = entitylist->get_enemy_heroes();

		targets.erase(std::remove_if(targets.begin(), targets.end(), [range, from](game_object_script x)
			{
				return !x->is_valid_target(range, from) || x->is_dead();
			}), targets.end());

		std::sort(targets.begin(), targets.end(), [from](game_object_script a, game_object_script b)
			{
				return a->get_distance(from) < b->get_distance(from);
			});

		if (!targets.empty())
			return targets.front();

		return nullptr;
	}

	bool enabled_in_map(std::map<std::uint32_t, TreeEntry*>& map, game_object_script target)
	{
		auto it = map.find(target->get_network_id());
		if (it == map.end())
			return false;

		return it->second->get_bool();
	}

	hit_chance get_hitchance(TreeEntry* entry)
	{
		switch (entry->get_int())
		{
		case 0:
			return hit_chance::low;
		case 1:
			return hit_chance::medium;
		case 2:
			return hit_chance::high;
		case 3:
			return hit_chance::very_high;
		}
		return hit_chance::medium;
	}
};

