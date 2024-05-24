#include "../plugin_sdk/plugin_sdk.hpp"

namespace utils
{

	// AIO utilities
	//
	void on_load();

	// Buff checks
	//
	bool has_unkillable_buff(game_object_script target);
	bool has_untargetable_buff(game_object_script target);
	bool has_crowd_control_buff(game_object_script target);

	// Spell casting
	//
	bool fast_cast(script_spell* spell, bool delay = true);
	bool cast(spellslot slot, bool is_charged_spell);
	bool cast(spellslot slot, game_object_script unit, bool is_charged_spell);
	bool fast_cast(script_spell* spell, vector position, bool delay = true);
	bool cast(spellslot slot, vector position, bool is_charged_spell);
	bool fast_cast(script_spell* spell, game_object_script unit, hit_chance minimum, bool aoe = false, int min_targets = 0, bool delay = true);
	bool fast_cast(script_spell* spell, int minMinions, bool is_jugnle_mobs = false, bool delay = true);

	// Spell utilities
	//
	bool is_ready(spellslot slot);

	// Champion utilities
	//
	float get_current_attackspeed(float base_as, float as_ratio);

	// Vector utilities
	//
	vector to_2d(vector vec);
	vector to_3d(vector vec);
	vector to_3d2(vector vec);
	vector add(vector source, float add);

	// Drawing utilities
	//
	void draw_dmg_rl(game_object_script target, float damage, unsigned long color);

	// Damage utilities
	//
	float get_damage(game_object_script target, std::vector<script_spell*> spells, int include_aa);

	// Minion utilities
	//
	int32_t count_enemies_in_range(vector vec, float range, bool include_bounding_radius = false);
	int32_t count_minions_in_range(float range);
	int32_t count_minions_in_range(game_object_script target, float range);
	int32_t count_minions_in_range(vector vec, float range, bool include_bounding_radius = false);
	int32_t count_monsters_in_range(float range);
	int32_t count_monsters_in_range(game_object_script target, float range);
	int32_t count_monsters_in_range(vector vec, float range, bool include_bounding_radius = false);

	// Target utilities
	//
	game_object_script get_closest_target(float range);
	game_object_script get_closest_target(float range, vector from);

	// Other
	//
	bool enabled_in_map(std::map<std::uint32_t, TreeEntry*>& map, game_object_script target);
	hit_chance get_hitchance(TreeEntry* entry);
};

