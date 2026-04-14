#pragma once
#include "sdk/BaseDataTypes/Array.h"
#include "sdk/BaseDataTypes/HytaleString.h"
#include "sdk/BaseDataTypes/Dictionary.h"
#include "Structs/IndependentStructs.h"
#include "Structs/Enums.h"
#include "core.h"

struct SelectedHitEntity : Object {
	Vector3f* hit_location;
	Position* position;
	Direction* body_rotation;
	int network_id;

	void DBGPrint(int indent = 0) {
		std::string prefix(indent, ' ');
		Util::log("%sSelectedHitEntity:\n", prefix.c_str());
		Util::log("%s  hit_location: %s\n", prefix.c_str(), hit_location ? "(" + std::to_string(hit_location->x) + ", " + std::to_string(hit_location->y) + ", " + std::to_string(hit_location->z) + ")" : "nullptr");
		Util::log("%s  position: %s\n", prefix.c_str(), position ? "(" + std::to_string(position->x) + ", " + std::to_string(position->y) + ", " + std::to_string(position->z) + ")" : "nullptr");
		Util::log("%s  body_rotation: %s\n", prefix.c_str(), body_rotation ? "(" + std::to_string(body_rotation->pitch) + ", " + std::to_string(body_rotation->yaw) + ")" : "nullptr");
		Util::log("%s  network_id: %d\n", prefix.c_str(), network_id);
	}
};

struct ForkedChainId : Object {
	ForkedChainId* forked_id;
	int entry_index;
	int sub_index;

	void DBGPrint(int indent = 0) {
		std::string prefix(indent, ' ');
		Util::log("%sForkedChainId:\n", prefix.c_str());
		Util::log("%s  entry_index: %d\n", prefix.c_str(), entry_index);
		Util::log("%s  sub_index: %d\n", prefix.c_str(), sub_index);
		if (forked_id) {
			Util::log("%s  forked_id (nested):\n", prefix.c_str());
			forked_id->DBGPrint(indent + 4);
		} else {
			Util::log("%s  forked_id: nullptr\n", prefix.c_str());
		}
	}
};

struct InteractionChainData : Object {
	Vector3f* hit_location;
	HytaleString* hit_detail;
	BlockPosition* block_position;
	Vector3f* hit_normal;
	int entity_id;
	int target_slot;
	Guuid proxy_id;

	void DBGPrint(int indent = 0) {
		std::string prefix(indent, ' ');
		Util::log("%sInteractionChainData:\n", prefix.c_str());
		Util::log("%s  hit_location: %s\n", prefix.c_str(), hit_location ? ("(" + std::to_string(hit_location->x) + ", " + std::to_string(hit_location->y) + ", " + std::to_string(hit_location->z) + ")").c_str() : "nullptr");
		Util::log("%s  hit_detail: %s\n", prefix.c_str(), hit_detail ? hit_detail->getString().c_str() : "nullptr");
		Util::log("%s  block_position: %s\n", prefix.c_str(), block_position ? ("(" + std::to_string(block_position->x) + ", " + std::to_string(block_position->y) + ", " + std::to_string(block_position->z) + ")").c_str() : "nullptr");
		Util::log("%s  hit_normal: %s\n", prefix.c_str(), hit_normal ? ("(" + std::to_string(hit_normal->x) + ", " + std::to_string(hit_normal->y) + ", " + std::to_string(hit_normal->z) + ")").c_str() : "nullptr");
		Util::log("%s  entity_id: %d\n", prefix.c_str(), entity_id);
		Util::log("%s  target_slot: %d\n", prefix.c_str(), target_slot);
		Util::log("%s  proxy_id: %d %d %d\n", prefix.c_str(), proxy_id.a, proxy_id.b, proxy_id.c);
	}
};

struct InteractionSyncData : Object {
	BlockPosition* block_position;
	BlockRotation* block_rotation;
	Dictionary<InteractionType, int>* fork_counts;
	Array<SelectedHitEntity*>* hit_entities;
	Position* attacker_pos;
	Direction* attacker_rot;
	Position* raycast_hit;
	Vector3f* raycast_normal;
	float progress;
	int operation_counter;
	int root_interaction;
	int total_forks;
	int entity_id;
	int entered_root_interaction;
	int placed_block_id;
	float charge_value;
	int chaining_index;
	int flag_index;
	float raycast_distance;
	int next_label;
	InteractionState state;
	BlockFace block_face;
	MovementDirection movement_direction;
	ApplyForceState apply_force_state;
	Guuid generated_u_u_i_d;

	void DBGPrint(int indent = 0) {
		std::string prefix(indent, ' ');
		Util::log("%sInteractionSyncData:\n", prefix.c_str());
		Util::log("%s  block_position: %s\n", prefix.c_str(), block_position ? ("(" + std::to_string(block_position->x) + ", " + std::to_string(block_position->y) + ", " + std::to_string(block_position->z) + ")").c_str() : "nullptr");
		Util::log("%s  block_rotation: %s\n", prefix.c_str(), block_rotation ? "[BlockRotation]" : "nullptr");
		Util::log("%s  fork_counts: %s\n", prefix.c_str(), fork_counts ? ("[Dictionary Count: " + std::to_string(fork_counts->count) + "]").c_str() : "nullptr");
		Util::log("%s  hit_entities: %s\n", prefix.c_str(), hit_entities ? ("[Array Count: " + std::to_string(hit_entities->count) + "]").c_str() : "nullptr");
		if (hit_entities) {
			for (int i = 0; i < hit_entities->count; i++) {
				Util::log("%s    [%d]:\n", prefix.c_str(), i);
				hit_entities->get(i)->DBGPrint(indent + 6);
			}
		}
		Util::log("%s  attacker_pos: %s\n", prefix.c_str(), attacker_pos ? ("(" + std::to_string(attacker_pos->x) + ", " + std::to_string(attacker_pos->y) + ", " + std::to_string(attacker_pos->z) + ")").c_str() : "nullptr");
		Util::log("%s  attacker_rot: %s\n", prefix.c_str(), attacker_rot ? ("(" + std::to_string(attacker_rot->pitch) + ", " + std::to_string(attacker_rot->yaw) + ")").c_str() : "nullptr");
		Util::log("%s  raycast_hit: %s\n", prefix.c_str(), raycast_hit ? ("(" + std::to_string(raycast_hit->x) + ", " + std::to_string(raycast_hit->y) + ", " + std::to_string(raycast_hit->z) + ")").c_str() : "nullptr");
		Util::log("%s  raycast_normal: %s\n", prefix.c_str(), raycast_normal ? ("(" + std::to_string(raycast_normal->x) + ", " + std::to_string(raycast_normal->y) + ", " + std::to_string(raycast_normal->z) + ")").c_str() : "nullptr");
		Util::log("%s  progress: %f\n", prefix.c_str(), progress);
		Util::log("%s  operation_counter: %d\n", prefix.c_str(), operation_counter);
		Util::log("%s  root_interaction: %d\n", prefix.c_str(), root_interaction);
		Util::log("%s  total_forks: %d\n", prefix.c_str(), total_forks);
		Util::log("%s  entity_id: %d\n", prefix.c_str(), entity_id);
		Util::log("%s  entered_root_interaction: %d\n", prefix.c_str(), entered_root_interaction);
		Util::log("%s  placed_block_id: %d\n", prefix.c_str(), placed_block_id);
		Util::log("%s  charge_value: %f\n", prefix.c_str(), charge_value);
		Util::log("%s  chaining_index: %d\n", prefix.c_str(), chaining_index);
		Util::log("%s  flag_index: %d\n", prefix.c_str(), flag_index);
		Util::log("%s  raycast_distance: %f\n", prefix.c_str(), raycast_distance);
		Util::log("%s  next_label: %d\n", prefix.c_str(), next_label);
		Util::log("%s  state: %d\n", prefix.c_str(), state);
		Util::log("%s  block_face: %d\n", prefix.c_str(), block_face);
		Util::log("%s  movement_direction: %d\n", prefix.c_str(), movement_direction);
		Util::log("%s  apply_force_state: %d\n", prefix.c_str(), apply_force_state);
		Util::log("%s  generated_u_u_i_d: %d %d %d\n", prefix.c_str(), generated_u_u_i_d.a, generated_u_u_i_d.b, generated_u_u_i_d.c);
	}
};

struct SyncInteractionChain : Object {
	HytaleString* item_in_hand_id;
	HytaleString* utility_item_id;
	HytaleString* tools_item_id;
	ForkedChainId* forked_id;
	InteractionChainData* data;
	Array<SyncInteractionChain*>* new_forks;
	Array<InteractionSyncData*>* interaction_data;
	int active_hotbar_slot;
	int active_utility_slot;
	int active_tools_slot;
	int override_root_interaction;
	int equip_slot;
	int chain_id;
	int operation_base_index;
	bool initial;
	bool desync;
	InteractionType interaction_type;
	InteractionState state;

	void DBGPrint(int indent = 0) {
		std::string prefix(indent, ' ');
		Util::log("%sSyncInteractionChain:\n", prefix.c_str());
		Util::log("%s  interaction_type: %d\n", prefix.c_str(), interaction_type);
		Util::log("%s  state: %d\n", prefix.c_str(), state);
		Util::log("%s  item_in_hand_id: %s\n", prefix.c_str(), item_in_hand_id ? item_in_hand_id->getString().c_str() : "nullptr");
		Util::log("%s  utility_item_id: %s\n", prefix.c_str(), utility_item_id ? utility_item_id->getString().c_str() : "nullptr");
		Util::log("%s  tools_item_id: %s\n", prefix.c_str(), tools_item_id ? tools_item_id->getString().c_str() : "nullptr");
		Util::log("%s  active_hotbar_slot: %d\n", prefix.c_str(), active_hotbar_slot);
		Util::log("%s  active_utility_slot: %d\n", prefix.c_str(), active_utility_slot);
		Util::log("%s  active_tools_slot: %d\n", prefix.c_str(), active_tools_slot);
		Util::log("%s  override_root_interaction: %d\n", prefix.c_str(), override_root_interaction);
		Util::log("%s  equip_slot: %d\n", prefix.c_str(), equip_slot);
		Util::log("%s  chain_id: %d\n", prefix.c_str(), chain_id);
		Util::log("%s  operation_base_index: %d\n", prefix.c_str(), operation_base_index);
		Util::log("%s  initial: %s\n", prefix.c_str(), initial ? "true" : "false");
		Util::log("%s  desync: %s\n", prefix.c_str(), desync ? "true" : "false");

		if (forked_id) {
			Util::log("%s  forked_id:\n", prefix.c_str());
			forked_id->DBGPrint(indent + 4);
		} else {
			Util::log("%s  forked_id: nullptr\n", prefix.c_str());
		}

		if (data) {
			Util::log("%s  data:\n", prefix.c_str());
			data->DBGPrint(indent + 4);
		} else {
			Util::log("%s  data: nullptr\n", prefix.c_str());
		}

		Util::log("%s  new_forks: %s\n", prefix.c_str(), new_forks ? ("[Array Count: " + std::to_string(new_forks->count) + "]").c_str() : "nullptr");
		if (new_forks) {
			for (int i = 0; i < new_forks->count; i++) {
				Util::log("%s    [%d]:\n", prefix.c_str(), i);
				new_forks->get(i)->DBGPrint(indent + 6);
			}
		}

		Util::log("%s  interaction_data: %s\n", prefix.c_str(), interaction_data ? ("[Array Count: " + std::to_string(interaction_data->count) + "]").c_str() : "nullptr");
		if (interaction_data) {
			for (int i = 0; i < interaction_data->count; i++) {
				Util::log("%s    [%d]:\n", prefix.c_str(), i);
				interaction_data->get(i)->DBGPrint(indent + 6);
			}
		}
	}
};

struct SyncInteractionChainsPacket : Object {
	Array<SyncInteractionChain*>* updates;

	static void Send(Vector3 pos, int placedBlockId = 0, bool quickReplace = false) {
		SyncInteractionChainsPacket* packet = CreatePacket<SyncInteractionChainsPacket*>(SyncInteractionChains_BI);
		Array<SyncInteractionChain*>* updates = new Array<SyncInteractionChain*>();
		packet->updates = updates;

		Packets::SendPacketImmediate(packet);

		delete updates;
	}

	void DBGPrint() {
		Util::log("SyncInteractionChainsPacket:\n");
		Util::log("  updates: %s\n", updates ? ("[Array Count: " + std::to_string(updates->count) + "]").c_str() : "nullptr");
		if (updates) {
			for (int i = 0; i < updates->count; i++) {
				Util::log("  [%d]:\n", i);
				SyncInteractionChain* chain = updates->get(i);
				if (chain) {
					chain->DBGPrint(4);
				} else {
					Util::log("    nullptr\n");
				}
			}
		}
	}
};

