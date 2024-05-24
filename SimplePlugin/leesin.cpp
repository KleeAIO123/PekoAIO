#include "../plugin_sdk/plugin_sdk.hpp"
#include "leesin.h"
#include "utils.h"
#include "dmg_lib.h"

namespace leesin
{
	// Define the colors that will be used in on_draw()
	//
#define Q_DRAW_COLOR (MAKE_COLOR ( 62, 129, 237, 255 )) 
#define W_DRAW_COLOR (MAKE_COLOR ( 227, 203, 20, 255 ))  
#define E_DRAW_COLOR (MAKE_COLOR ( 235, 12, 223, 255 ))  
#define R_DRAW_COLOR (MAKE_COLOR ( 224, 77, 13, 255 ))   

// To declare a spell, it is necessary to create an object and registering it in load function
	script_spell* q = nullptr;
	script_spell* w = nullptr;
	script_spell* e = nullptr;
	script_spell* r = nullptr;
	script_spell* ward = nullptr;

	script_spell* flash = nullptr;
	script_spell* smite = nullptr;

	// Declaration of menu objects
	TreeTab* main_tab = nullptr;

	namespace draw_settings
	{
		TreeEntry* draw_range_q = nullptr;
		TreeEntry* draw_range_w = nullptr;
		TreeEntry* draw_range_e = nullptr;
		TreeEntry* draw_range_r = nullptr;
		TreeEntry* draw_flash_range = nullptr;
		TreeEntry* q_color = nullptr;
		TreeEntry* w_color = nullptr;
		TreeEntry* e_color = nullptr;
		TreeEntry* r_color = nullptr;
	}

	namespace combo
	{
		TreeEntry* leesin_mode = nullptr;
		TreeEntry* use_q = nullptr;
		TreeEntry* use_q_on = nullptr;
		TreeEntry* use_q2 = nullptr;
		TreeEntry* use_w = nullptr;
		TreeEntry* use_w2 = nullptr;
		TreeEntry* w_to_save_ally = nullptr;
		TreeEntry* use_w_on = nullptr;
		TreeEntry* w_HP = nullptr;
		TreeEntry* use_e_on = nullptr;
		TreeEntry* use_e = nullptr;
		TreeEntry* use_e2 = nullptr;
		TreeEntry* use_r = nullptr;
		TreeEntry* q_max_range = nullptr;
		std::map<std::uint32_t, TreeEntry*> can_use_q_on;
		TreeEntry* r_KS = nullptr;
		//TreeEntry* use_r_ks = nullptr;
	}

	namespace harass
	{
		TreeEntry* use_q = nullptr;
		TreeEntry* use_e = nullptr;
	}

	namespace laneclear
	{
		TreeEntry* use_e = nullptr;
		TreeEntry* spell_farm = nullptr;
	}
	namespace jungleclear
	{
		TreeEntry* use_q = nullptr;
		TreeEntry* use_w = nullptr;
		TreeEntry* use_e = nullptr;
	}
	namespace fleemode
	{
		TreeEntry* use_w;
	}
	namespace Insec
	{
		TreeEntry* insec_r = nullptr;
	}
	namespace misc
	{
		TreeEntry* use_flash;
		TreeEntry* wait_for_Q;
		TreeEntry* ward_jump;
		TreeEntry* ward_jump_key;

	}
	namespace hitchance
	{
		TreeEntry* q_hitchance = nullptr;
		TreeEntry* w_hitchance = nullptr;
		TreeEntry* e_hitchance = nullptr;
		TreeEntry* r_hitchance = nullptr;
	}
	namespace fleemode
	{
		TreeEntry* w_jump_on_ally_champions = nullptr;
		TreeEntry* w_jump_on_ally_minions = nullptr;
		TreeEntry* w_ward_jump = nullptr;
	}
	// Event handler functions
	void on_update();
	void on_draw();
	//void on_create_object(game_object_script sender);
	//void on_delete_object(game_object_script sender);

	// Declaring functions responsible for spell-logic
	//
	void q_logic();
	void w_logic();
	void e_logic();
	void r_logic();
	void on_process_spell_cast(game_object_script sender, spell_instance_script spell);
	float last_Q_time;
	//void ward_jump_logic();

	// Utils
	//
	bool can_use_r_on(game_object_script target);
	inline void draw_dmg_rl(game_object_script target, float damage, unsigned long color);

	void load()
	{
		// Registering a spells
		//
		q = plugin_sdk->register_spell(spellslot::q, 1200);
		q->set_skillshot(0.25f, 75.0f, 1600.0f, { collisionable_objects::minions, collisionable_objects::yasuo_wall, collisionable_objects::heroes }, skillshot_type::skillshot_line);
		w = plugin_sdk->register_spell(spellslot::w, 900);
		e = plugin_sdk->register_spell(spellslot::e, 575);
		r = plugin_sdk->register_spell(spellslot::r, 900);
		ward = plugin_sdk->register_spell(spellslot::trinket, 600);

		// Checking which slot the Summoner Flash is on 
		//
		if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
			flash = plugin_sdk->register_spell(spellslot::summoner1, 400.f);
		else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerFlash"))
			flash = plugin_sdk->register_spell(spellslot::summoner2, 400.f);

		// Checking which slot the Summoner Smite is on 
		//
		if (myhero->get_spell(spellslot::summoner1)->get_spell_data()->get_name_hash() == spell_hash("SummonerSmite"))
			flash = plugin_sdk->register_spell(spellslot::summoner1, 400.f);
		else if (myhero->get_spell(spellslot::summoner2)->get_spell_data()->get_name_hash() == spell_hash("SummonerSmite"))
			flash = plugin_sdk->register_spell(spellslot::summoner2, 400.f);

		// Create a menu according to the description in the "Menu Section"
		//
		main_tab = menu->create_tab("leesin", "Leesin");
		main_tab->set_assigned_texture(myhero->get_square_icon_portrait());
		{
			// Info
			//Urf ›]é_Ê¼Œ‘
			main_tab->add_separator(myhero->get_model() + ".aio", "Peko : " + myhero->get_model());
			auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
			{
				combo::leesin_mode = combo->add_combobox(myhero->get_model() + ".combo.h.mode", "Leesiin Mode", { {"Normal", nullptr},{"Urf", nullptr } }, 0);
				combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
				combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());


				auto q_config = combo->add_tab(myhero->get_model() + "combo.q.config", "Q Config");
				{

					{
						for (auto&& enemy : entitylist->get_enemy_heroes())
						{
							combo::can_use_q_on[enemy->get_network_id()] = q_config->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, false);
							combo::can_use_q_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
						}
						combo::q_max_range = q_config->add_slider(myhero->get_model() + "combo_q_max_range", "Max Q range", 900, 750, 1000);
					}
				}
				combo::use_q2 = combo->add_checkbox(myhero->get_model() + ".combo.q2", "Use Q2", true);
				combo::use_q2->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
				combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
				auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
				{
					combo::w_HP = w_config->add_slider(myhero->get_model() + "combo_w_hp", "W Hp", 50, 0, 100);

					combo::w_to_save_ally = w_config->add_checkbox(myhero->get_model() + ".combo.w.use_on", "Use W On", true);
					combo::w_to_save_ally->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
					auto use_w_on_tab = w_config->add_tab(myhero->get_model() + ".combo.w.use_on", "Use W On");
					{
						//for (auto&& ally : entitylist->get_ally_heroes())
						//{
						//	combo::w_use_on[ally->get_network_id()] = use_w_on_tab->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), ally->is_me() ? false : true, false);

						//	// Set texture to ally square icon
						//	//
						//	combo::w_use_on[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
						//}
					}
				}
				auto e_config = combo->add_tab(myhero->get_model() + "combo.e.config", "E Config");
				{
					combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
					combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
					auto use_e_on_tab = e_config->add_tab(myhero->get_model() + ".combo.e.use_on", "Use E On");
					{
						//for (auto&& enemy : entitylist->get_enemy_heroes())
						//{
						//	combo::e_use_on[enemy->get_network_id()] = use_e_on_tab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, false);

						//	// Set texture to enemy square icon
						//	//
						//	combo::e_use_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
						//}
					}
				}
				combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
				combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

				auto r_config = combo->add_tab(myhero->get_model() + "combo.r.config", "R Config");
				{
					combo::r_KS = r_config->add_checkbox(myhero->get_model() + "r_ks", "Use R Ks", true);
					//combo::r_use_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
					//auto use_r_on_tab = r_config->add_tab(myhero->get_model() + ".combo.r.use_on", "Use R On");
					//{
					//    for (auto&& enemy : entitylist->get_enemy_heroes())
					//    {
					//        combo::r_use_on[enemy->get_network_id()] = use_r_on_tab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, false);
					//        // Set texture to enemy square icon
					//        //
					//        combo::r_use_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
					//    }
					//}
				}
			}
			auto harass = main_tab->add_tab(myhero->get_model() + ".harass", "Harass Settings");
			{
				harass::use_q = harass->add_checkbox(myhero->get_model() + ".harass.q", "Use Q", true);
				harass::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				harass::use_e = harass->add_checkbox(myhero->get_model() + ".harass.e", "Use E", true);
				harass::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
			}
			auto Insec = main_tab->add_tab(myhero->get_model() + ".Insec", "Insec Settings");
			{
				Insec::insec_r = Insec->add_hotkey(myhero->get_model() + "insec_r", "Insec R", TreeHotkeyMode::Hold, 'T', false);

			}
			auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
			{
				laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
				laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", true);
				laneclear::use_e->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				//auto laneclear_set = laneclear->add_tab(myhero->get_model() + ".laneclear.e.config", "E Config");
				//{
				//	//laneclear::e_minimum_minions = e_config->add_slider(myhero->get_model() + ".laneclear.e.minimum_minions", "Minimum minions", 2, 0, 5);
				//}
			}
			auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
			{
				jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true);
				jungleclear::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				jungleclear::use_w = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.w", "Use W", true);
				jungleclear::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
				jungleclear::use_e = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.e", "Use E", true);
				jungleclear::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
			}
			auto fleemode = main_tab->add_tab(myhero->get_model() + ".flee", "Flee Mode");
			{
				fleemode::use_w = fleemode->add_checkbox(myhero->get_model() + ".flee.w", "Use W", true);
				fleemode::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
			}
			auto hitchance = main_tab->add_tab(myhero->get_model() + ".hitchance", "Hitchance Settings");
			{
				hitchance::q_hitchance = hitchance->add_combobox(myhero->get_model() + ".hitchance.q", "Hitchance Q", { {"Low",nullptr},{"Medium",nullptr },{"High", nullptr},{"Very High",nullptr} }, 2);
			}

			auto draw_settings = main_tab->add_tab(myhero->get_model() + ".draw", "Drawings Settings");
			{
				//float color1[] = { 0.9f, 0.5f, 0.9f, 1.0f }; //Æ«×Ï·Û
				float color2[] = { 0.5f, 0.9f, 0.9f, 1.0f }; //Æ«À¶ÂÌ
				//float color3[] = { 0.9f, 0.9f, 0.5f, 1.0f }; //Æ«»Æ
				//float color4[] = { 0.9f, 0.9f, 0.0f, 1.0f }; //»Æ
				//float color5[] = { 0.9f, 0.0f, 0.9f, 1.0f }; //×Ï
				//float color6[] = { 0.0f, 0.9f, 0.9f, 1.0f }; //À¶ÂÌ
				//float color7[] = { 0.9f, 0.5f, 0.0f, 1.0f }; //éÙ
				//float color8[] = { 0.9f, 0.0f, 0.5f, 1.0f }; //Æ«ºì·Û
				//float color9[] = { 0.0f, 0.9f, 0.5f, 1.0f }; //ÂÌ
				//float color10[] = { 0.5f, 0.9f, 0.0f, 1.0f }; //µ­ÂÌ
				//float color11[] = { 0.5f, 0.0f, 0.9f, 1.0f }; //×Ï
				//float color12[] = { 0.0f, 0.5f, 0.9f, 1.0f }; //Æ«ÉîÀ¶
				//float color13[] = { 1.0f, 0.5f, 0.0f, 1.0f }; //ºì
				//float color14[] = { 0.0f, 1.0f, 0.0f, 1.0f }; //ÂÌ
				//float color15[] = { 0.0f, 0.0f, 1.0f, 1.0f }; //À¶

				draw_settings::draw_range_q = draw_settings->add_checkbox(myhero->get_model() + ".draw.q", "Draw Q range", true);
				draw_settings::draw_range_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
				draw_settings::q_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.q.color", "Q Color", color2);

				draw_settings::draw_range_w = draw_settings->add_checkbox(myhero->get_model() + ".draw.w", "Draw W range", true);
				draw_settings::draw_range_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
				draw_settings::w_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.w.color", "W Color", color2);

				draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
				draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
				draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color2);

				draw_settings::draw_range_r = draw_settings->add_checkbox(myhero->get_model() + ".draw.r", "Draw R range", true);
				draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
				draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color2);
			}
		}

		event_handler<events::on_update>::add_callback(on_update);
		event_handler<events::on_draw>::add_callback(on_draw);
		event_handler<events::on_process_spell_cast>::add_callback(on_process_spell_cast);

		// Chat message after load
		//
		utils::on_load();
	}

	void unload()
	{
		plugin_sdk->remove_spell(q);
		plugin_sdk->remove_spell(w);
		plugin_sdk->remove_spell(e);
		plugin_sdk->remove_spell(r);

		if (flash)
			plugin_sdk->remove_spell(flash);

		if (smite)
			plugin_sdk->remove_spell(smite);

		// Remove menu tab
		//
		menu->delete_tab(main_tab);

		event_handler<events::on_update>::remove_handler(on_update);
		event_handler<events::on_draw>::remove_handler(on_draw);
		event_handler<events::on_process_spell_cast>::remove_handler(on_process_spell_cast);
	}
	void on_update()
	{
		if (myhero->is_dead())
		{
			return;
		}
		if (orbwalker->can_move(0.05f))
		{
			/*if (w->is_ready() && misc::ward_jump->get_bool() && misc::ward_jump_key->get_bool())
			{
				ward_jump_logic();
			}*/
			if (orbwalker->combo_mode())
			{
				if (q->is_ready() && combo::use_q->get_bool())
				{
					q_logic();
				}

				if (w->is_ready() && combo::use_w->get_bool())
				{
					w_logic();
				}

				if (e->is_ready() && combo::use_e->get_bool())
				{
					e_logic();
				}

				if (r->is_ready() && combo::use_r->get_bool())
				{
					r_logic();
				}
			}
			if (orbwalker->harass())
			{
				if (!myhero->is_under_enemy_turret())
				{
					if (q->is_ready() && harass::use_q->get_bool())
					{
						q_logic();
					}

					if (e->is_ready() && harass::use_e->get_bool())
					{
						e_logic();
					}
				}
			}
			if (orbwalker->flee_mode())
			{
				if (w->is_ready() && fleemode::use_w->get_bool())
				{
					std::vector<game_object_script> allies;

					if (fleemode::w_jump_on_ally_champions->get_bool())
					{
						auto champions = entitylist->get_ally_heroes();
						allies.insert(allies.end(), champions.begin(), champions.end());
					}

					if (fleemode::w_jump_on_ally_minions->get_bool())
					{
						auto minions = entitylist->get_ally_minions();
						allies.insert(allies.end(), minions.begin(), minions.end());
					}

					std::sort(allies.begin(), allies.end(), [](game_object_script a, game_object_script b)
						{
							return a->get_distance(hud->get_hud_input_logic()->get_game_cursor_position()) < b->get_distance(hud->get_hud_input_logic()->get_game_cursor_position());
						});

					allies.erase(std::remove_if(allies.begin(), allies.end(), [](game_object_script x)
						{
							return x == myhero || x->get_distance(myhero->get_position()) > q->range();
						}), allies.end());

					if (!allies.empty())
					{
						if (q->cast(allies.front()))
						{
							return;
						}
					}

					/*if (fleemode::w_ward_jump->get_bool())
					{
						ward_jump_logic();
					}*/
				}
			}
			//if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool())
			//{
			//	auto lane_minions = entitylist->get_enemy_minions();

			//	// Gets jugnle mobs from the entitylist
			//	auto monsters = entitylist->get_jugnle_mobs_minions();

			//	lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
			//		{
			//			return !x->is_valid_target(e->range());
			//		}), lane_minions.end());
			//	monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
			//		{
			//			return !x->is_valid_target(e->range());
			//		}), monsters.end());

			//	//std::sort -> sort lane minions by distance
			//	std::sort(lane_minions.begin(), lane_minions.end(), [](game_object_script a, game_object_script b)
			//		{
			//			return a->get_position().distance(myhero->get_position()) < b->get_position().distance(myhero->get_position());
			//		});

			//	//std::sort -> sort monsters by max health
			//	std::sort(monsters.begin(), monsters.end(), [](game_object_script a, game_object_script b)
			//		{
			//			return a->get_max_health() > b->get_max_health();
			//		});

			//	if (!lane_minions.empty())
			//	{
			//		if (e->is_ready() && laneclear::use_e->get_bool())
			//		{
			//			/*if (e->cast_on_best_farm_position(laneclear::e_minimum_minions->get_int()))
			//			{
			//				return;
			//			}*/
			//		}
			//	}

			//	if (!monsters.empty())
			//	{
			//		if (q->is_ready() && jungleclear::use_q->get_bool())
			//		{
			//			if (q->cast(monsters.front()))
			//			{
			//				return;
			//			}
			//		}
			//		if (w->is_ready() && jungleclear::use_w->get_bool())
			//		{
			//			if (w->cast(myhero))
			//			{
			//				return;
			//			}
			//			if (e->is_ready() && jungleclear::use_e->get_bool())
			//			{
			//				if (e->cast_on_best_farm_position(1, true))
			//				{
			//					return;
			//				}
			//			}
			//		}
			//	}
			//}
		}
	}



#pragma region q_logic
	void q_logic()
	{
		auto target = target_selector->get_target(combo::q_max_range->get_int(), damage_type::physical);
		if (target != nullptr)
		{
			bool q_active = myhero->has_buff(buff_hash("blindmonkqmanager"));
			if (combo::use_q->get_bool() && !q_active  && last_Q_time + 3.0 < gametime->get_time())
			{
				if (q->cast(target, utils::get_hitchance(hitchance::q_hitchance)))
				{


					return;

				}
			}
			if (combo::use_q2->get_bool() && q_active)
			{
				if (last_Q_time + 3.0 < gametime->get_time())
				{
					if (q->cast())
					{
						return;
					}
				}
			}
		}
	}
#pragma endregion

#pragma region w_logic
	void w_logic()
	{
		if (combo::use_w->get_bool())
		{
			if (myhero->get_health_percent() <= combo::w_HP->get_int())
			{

				for (auto&& enemy : entitylist->get_enemy_heroes())
				{
					if (enemy->is_valid() && !enemy->is_dead() && enemy->is_valid_target(400))
					{
						if (!myhero->has_buff(buff_hash("OlafFrenziedStrikes")))
						{
							if (w->cast())
							{
								return;
							}
						}

					}

				}


			}
		}
		if (combo::w_to_save_ally->get_bool())
		{
			// Get a target from a given range
			auto target = target_selector->get_target(w->range(), damage_type::magical);

			if (target != nullptr)
			{
				/*if (!utils::has_killable_buff(ally) && (!combo::w->get_bool())) && (!myhero->has_buff(buff_hash("LeeSinPassive")))
				{
					if (w->cast(myhero))
				}*/
			}
		}
	}

#pragma endregion

#pragma region ward_jump_logic
	void ward_jump_logic()
	{
		game_object_script near_ward = nullptr;

		for (auto& object : entitylist->get_other_minion_objects())
		{
			if (object->is_valid())
			{
				if (object->get_distance(hud->get_hud_input_logic()->get_game_cursor_position()) < 100)
				{
					if (myhero->is_facing(object))
					{
						if (object->get_name().compare("SightWard") == 0)
						{
							near_ward = object;
							break;
						}
					}
				}
			}
		}

		if (near_ward == nullptr)
		{
			if (ward->is_ready())
			{
				if (ward->cast(hud->get_hud_input_logic()->get_game_cursor_position()))
				{
					return;
				}
			}
		}
		else
		{
			w->cast(near_ward);
		}
	}
#pragma endregion

#pragma region e_logic
	void e_logic()
	{
		auto target = target_selector->get_target(e->range(), damage_type::magical);
		if (target != nullptr)
		{
			if (combo::use_e->get_bool())
			{
				if (e->cast(target))
				{
					if (e->is_ready())
					{
						if (e->cast())
						{
							return;
						}
					}
					else
					{
						return;
					}
				}
			}
		}

	}

#pragma endregion

#pragma region r_logic
	void r_logic()
	{
		auto target = target_selector->get_target(r->range(), damage_type::magical);
		if (target != nullptr)
		{
			if (combo::r_KS->get_bool())
			{
				if (r->get_damage(target) > target->get_health())
				{
					if (r->cast(target))
					{
						return;
					}
				}
			}
		}
	}
#pragma endregion
	void on_process_spell_cast(game_object_script sender, spell_instance_script spell)
	{
		auto spell_hash = spell->get_spell_data()->get_name_hash();
		if (sender->is_me() && spell_hash == spell_hash("BlindMonkQOne"))
		{
			//myhero->print_chat(1, "L attack");
			last_Q_time = gametime->get_time();
		}

	}
	void on_draw()
	{
		//ÀLÑu¹ ‡ú
		if (draw_settings::draw_range_q->get_bool())
			draw_manager->add_circle(myhero->get_position(), q->range(), draw_settings::q_color->get_color(), 2);

		if (draw_settings::draw_range_w->get_bool())
			draw_manager->add_circle(myhero->get_position(), w->range(), draw_settings::w_color->get_color(), 2);

		if (draw_settings::draw_range_e->get_bool())
			draw_manager->add_circle(myhero->get_position(), e->range(), draw_settings::e_color->get_color(), 2);

		if (draw_settings::draw_range_r->get_bool())
			draw_manager->add_circle(myhero->get_position(), r->range(), draw_settings::r_color->get_color(), 2);


	}

};
