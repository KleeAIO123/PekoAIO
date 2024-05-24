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

    script_spell* flash = nullptr;

    // Declaration of menu objects
    TreeTab* main_tab = nullptr;

    namespace draw_settings
    {
        TreeEntry* draw_range_q = nullptr;
        TreeEntry* draw_range_w = nullptr;
        TreeEntry* draw_range_e = nullptr;
        TreeEntry* draw_range_r = nullptr;
        TreeEntry* draw_flash_range = nullptr;
    }

    namespace combo
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_q2 = nullptr;
        TreeEntry* use_w = nullptr;
        TreeEntry* w_to_save_ally = nullptr;
        TreeEntry* use_e = nullptr;
        TreeEntry* use_e2 = nullptr;
        TreeEntry* use_r = nullptr;
        TreeEntry* use_r_ks = nullptr;
    }

    namespace harass
    {
        TreeEntry* use_q = nullptr;
        TreeEntry* use_e = nullptr;
    }

    namespace laneclear
    {
        TreeEntry* use_e = nullptr;
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
        TreeEntry* use_flash;
        TreeEntry* wait_for_Q ;
    }
    // Event handler functions
    void on_update();
    void on_draw();
    void on_create_object(game_object_script sender);
    void on_delete_object(game_object_script sender);

    // Declaring functions responsible for spell-logic
    //
    void q_logic();
    void w_logic();
    void ward_jump();
    void e_logic();
    void r_logic();

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
            //Urf mode not available
            main_tab->add_separator(myhero->get_model() + ".aio", "Peko : " + myhero->get_model());
            auto combo = main_tab->add_tab(myhero->get_model() + ".combo", "Combo Settings");
          {
            combo::use_q = combo->add_checkbox(myhero->get_model() + ".combo.q", "Use Q", true);
            combo::use_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
            combo::q_auto_on_cc = 1_config->add_checkbox(myhero->get_model() + ".combo.q.auto_on_cc", "Use Q on CC", false);
            combo::q_auto_on_cc->set_texture(myhero->get_spell(spellslot::Q)->get_icon_texture());

            auto q_config = combo->add_tab(myhero->get_model() + "combo.q.config", "Q Config");
            {
                combo::q_force_use_selected_targe = q_config->add_checkbox(myhero->get_model() + ".combo.q.force_use_selected_targe", "Force use Q on selected target", true);
                auto use_q_on_tab = q_config->add_tab(myhero->get_model() + ".combo.q.use_on", "Use Q On");
                {
                    for (auto&& enemy : entitylist->get_enemy_heroes())
                    {
                        combo::q_use_on[enemy->get_network_id()] = use_q_on_tab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, false);

                        // Set texture to enemy square icon
                        //
                        combo::q_use_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
                    }
                }
            }
            combo::use_q2 = combo->add_checkbox(myhero->get_model() + ".combo.q2", "Use Q2", true);
            combo::use_q2->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
            combo::use_w = combo->add_checkbox(myhero->get_model() + ".combo.w", "Use W", true);
            combo::use_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
            auto w_config = combo->add_tab(myhero->get_model() + ".combo.w.config", "W Config");
            {
                combo::w_to_save_ally = w_config->add_checkbox(myhero->get_model() + ".combo.w.use_on", "Use W On", true);
                combo:w_to_save_ally->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
                auto use_w_on_tab = w_config->add_tab(myhero->get_model() + ".combo.w.use_on", "Use W On");
                {
                    for (auto&& ally : entitylist->get_ally_heroes())
                    {
                        combo::w_use_on[ally->get_network_id()] = use_w_on_tab->add_checkbox(std::to_string(ally->get_network_id()), ally->get_model(), ally->is_me() ? false : true, false);

                        // Set texture to ally square icon
                        //
                        combo::w_use_on[ally->get_network_id()]->set_texture(ally->get_square_icon_portrait());
                    }
                }
            }
            auto e_config = combo->add_tab(myhero->get_model() + "combo.e.config", "E Config");
            {
                combo::use_e = combo->add_checkbox(myhero->get_model() + ".combo.e", "Use E", true);
                combo::use_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
                auto use_e_on_tab = e_config->add_tab(myhero->get_model() + ".combo.e.use_on", "Use E On");
                {
                    for (auto&& enemy : entitylist->get_enemy_heroes())
                    {
                        combo::e_use_on[enemy->get_network_id()] = use_e_on_tab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, false);

                        // Set texture to enemy square icon
                        //
                        combo::e_use_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
                    }
                }
            }
            combo::use_r = combo->add_checkbox(myhero->get_model() + ".combo.r", "Use R", true);
            combo::use_r->set_texture(myhero->get_spell(spellslot::r)->get_icon_texture());

            auto r_config = combo->add_tab(myhero->get_model() + "combo.r.config", "R Config");
            {
                combo::r_KS = use_r_on_tab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, false);
                combo::r_use_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
                auto use_r_on_tab = r_config->add_tab(myhero->get_model() + ".combo.r.use_on", "Use R On");
                {
                    for (auto&& enemy : entitylist->get_enemy_heroes())
                    {
                        combo::r_use_on[enemy->get_network_id()] = use_r_on_tab->add_checkbox(std::to_string(enemy->get_network_id()), enemy->get_model(), true, false);
                        // Set texture to enemy square icon
                        //
                        combo::r_use_on[enemy->get_network_id()]->set_texture(enemy->get_square_icon_portrait());
                    }
                }
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
            { hotkey = element->add_hotkey("Insec Mode", "Enable mode", TreeHotkeyMode::Toggle, 0x54 /*T key*/, false);

            }
            auto laneclear = main_tab->add_tab(myhero->get_model() + ".laneclear", "Lane Clear Settings");
            {
            laneclear::spell_farm = laneclear->add_hotkey(myhero->get_model() + ".laneclear.enabled", "Toggle Spell Farm", TreeHotkeyMode::Toggle, 0x04, true);
            laneclear::use_e = laneclear->add_checkbox(myhero->get_model() + ".laneclear.e", "Use E", true);
            laneclear::use_e->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
            auto e_config = laneclear->add_tab(myhero->get_model() + ".laneclear.e.config", "E Config");
                {
                laneclear::e_minimum_minions = e_config->add_slider(myhero->get_model() + ".laneclear.e.minimum_minions", "Minimum minions", 2, 0, 5);
                }
            }
            auto jungleclear = main_tab->add_tab(myhero->get_model() + ".jungleclear", "Jungle Clear Settings");
            {
            jungleclear::use_q = jungleclear->add_checkbox(myhero->get_model() + ".jungleclear.q", "Use Q", true;
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
            draw_settings::draw_range_q = draw_settings->add_checkbox(myhero->get_model() + ".draw.q", "Draw Q range", true);
            draw_settings::draw_range_q->set_texture(myhero->get_spell(spellslot::q)->get_icon_texture());
            draw_settings::q_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.q.color", "Q Color", color);

            draw_settings::draw_range_w = draw_settings->add_checkbox(myhero->get_model() + ".draw.w", "Draw W range", true);
            draw_settings::draw_range_w->set_texture(myhero->get_spell(spellslot::w)->get_icon_texture());
            draw_settings::w_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.w.color", "W Color", color);

            draw_settings::draw_range_e = draw_settings->add_checkbox(myhero->get_model() + ".draw.e", "Draw E range", true);
            draw_settings::draw_range_e->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            draw_settings::e_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.e.color", "E Color", color);

            draw_settings::draw_range_r = draw_settings->add_checkbox(myhero->get_model() + ".draw.r", "Draw R range", true);
            draw_settings::draw_range_r->set_texture(myhero->get_spell(spellslot::e)->get_icon_texture());
            draw_settings::r_color = draw_settings->add_colorpick(myhero->get_model() + ".draw.r.color", "R Color", color);
            }
        }
    
        event_handler<events::on_update>::add_callback(on_update);
        event_handler<events::on_draw>::add_callback(on_draw);

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
    }
    void on_update()
    {
        if (myhero->is_dead())
        {
            return;
        }
        if (orbwalker->can_move(0.05f))
        {
            if (w->is_ready() && misc::ward_jump->get_bool() && misc::ward_jump_key->get_bool())
            {
                ward_jump_logic();
            }
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

                if (fleemode::w_ward_jump->get_bool())
                {
                    ward_jump_logic();
                }
            }
        }
        if (orbwalker->lane_clear_mode() && laneclear::spell_farm->get_bool())
        {
          auto lane_minions = entitylist->get_enemy_minions();

          // Gets jugnle mobs from the entitylist
          auto monsters = entitylist->get_jugnle_mobs_minions();

          lane_minions.erase(std::remove_if(lane_minions.begin(), lane_minions.end(), [](game_object_script x)
          {
              return !x->is_valid_target(e->range());
          }), lane_minions.end());
              monsters.erase(std::remove_if(monsters.begin(), monsters.end(), [](game_object_script x)
              {
                  return !x->is_valid_target(e->range());
              }), monsters.end());

              //std::sort -> sort lane minions by distance
              std::sort(lane_minions.begin(), lane_minions.end(), [](game_object_script a, game_object_script b)
              {
                  return a->get_position().distance(myhero->get_position()) < b->get_position().distance(myhero->get_position());
              });

              //std::sort -> sort monsters by max health
              std::sort(monsters.begin(), monsters.end(), [](game_object_script a, game_object_script b)
              {
                  return a->get_max_health() > b->get_max_health();
              });

              if (!lane_minions.empty())
              {
                 if (e->is_ready() && laneclear::use_e->get_bool())
                 {
                   if (e->cast_on_best_farm_position(laneclear::e_minimum_minions->get_int()))
                   {
                      return;
                   }
                 }
              }

              if (!monsters.empty())
              {
                  if (q->is_ready() && jungleclear::use_q->get_bool())
                  {
                      if (q->cast(monsters.front(), get_hitchance(hitchance::q_hitchance)))
                      {
                          return;
                      }
                  }
                    if (w->is_ready() && jungleclear::use_w->get_bool())
                    {
                        if (w->cast(myhero))
                        {
                          return;
                        }
                         if (e->is_ready() && jungleclear::use_e->get_bool())
                         {
                            if (e->cast_on_best_farm_position(1, true))
                            {
                              return;
                            }
                         }
                    }
              }
            
        

#pragma region q_logic
    void q_logic()
    {
        if (combo::use_q2->get_bool())
        {
            for (auto& enemy : entitylist->get_enemy_heroes())
            {
                if (enemy->is_valid() && !enemy->is_dead() && can_use_q_on(enemy) && (!enemy->is_under_ally_turret()&&(!myhero->has_buff(buff_hash("LeeSinPassive"))) || scheduler->delay_action(2.9f, [] 
                    {if (q->cast())
                     {
                        return;
                    }
                } 
            }
        }   
            if (combo::q_auto_on_cc->get_bool())
            {
            // Get a target from a given range
            auto target = target_selector->get_target(combo::q_max_range->get_int(), damage_type::physical);

            // Always check an object is not a nullptr!
            if (target != nullptr && can_use_q_on(target) && myhero->get_distance(target))
                {
                return false;
                }
            // Get a target from a given range
            auto selected_target = target_selector->get_selected_target();
            auto target = selected_target != nullptr && selected_target->is_valid() && selected_target->is_valid_target(combo::q_max_range->get_int())
                ? selected_target : target_selector->get_target(combo::q_max_range->get_int(), damage_type::additional);

                if (target != nullptr && can_use_q_on(target) && myhero->get_distance(target) > min_distance && (!myhero->has_buff(buff_hash("LeeSinPassive")))
                {
            q->cast(target, get_hitchance(hitchance::q_hitchance));
                }
            }

#pragma endregion

#pragma region w_logic
    void w_logic()
    {
        if (combo::use_w2->get_bool())
        {
            for (auto& ally: entitylist->get_ally_heroes())
            {
                if (!utils::has_unkillable_buff(ally) && (!combo::w->get_bool()))
                {
                    if ((ally->get_health_percent() < (ally->is_me() ? combo::w_myhero_hp_under->get_int() : combo::w_ally_hp_under->get_int())))
                    {
                        if (w->cast(ally))
                        {
                            return;
                        }
                    }
                }
            }
        }
    }

    if (combo::w_use_while_chasing->get_bool())
    {
        // Get a target from a given range
        auto target = target_selector->get_target(w->range(), damage_type::magical);

       if (target != nullptr)
       {
          if (!utils::has_killable_buff(ally) && (!combo::w->get_bool())) && (!myhero->has_buff(buff_hash("LeeSinPassive")))
          {
              if(w->cast(myhero))
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
    if (combo::use_e2->get_bool())
    {
        for (auto& enemy : entitylist->get_enemy_heroes())
        {
            if (enemy->is_valid() && !enemy->is_dead() && can_use_e_on(enemy) && (!enemy->is_under_ally_turret() && (!myhero->has_buff(buff_hash("LeeSinPassive")))
            {if (e->cast())
                {
                return;
                }
            }
        }
    }
#pragma endregion

#pragma region r_logic
    void r_logic()
    if (combo::use_r_ks->get_bool())
    {
        for (auto& enemy : enemies)
        {
            if (r->get_damage(enemy) > enemy->get_real_health())
             { if (r->cast();
              {
              return;
              }
#pragma endregion



#pragma region get_hitchance
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
#pragma endregion
