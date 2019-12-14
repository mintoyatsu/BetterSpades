/*
	Copyright (c) 2017-2018 ByteBit

	This file is part of BetterSpades.

    BetterSpades is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    BetterSpades is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with BetterSpades.  If not, see <http://www.gnu.org/licenses/>.
*/

const char* network_reason_disconnect(int code);

unsigned int network_ping(void);
void network_send(int id, void* data, int len);
void network_updateColor(void);
void network_disconnect(void);
int network_identifier_split(char* addr, char* ip_out, int* port_out);
int network_connect(char* ip, int port);
int network_connect_string(char* addr);
int network_update(void);
int network_status(void);
void network_init(void);

void read_PacketPositionData(void* data, int len);
void read_PacketOrientationData(void* data, int len);
void read_PacketMovementData(void* data, int len);
void read_PacketAnimationData(void* data, int len);
void read_PacketHit(void* data, int len);
void read_PacketGrenade(void* data, int len);
void read_PacketSetTool(void* data, int len);
void read_PacketSetColor(void* data, int len);
void read_PacketExistingPlayer(void* data, int len);
void read_PacketIntelAction(void* data, int len);
void read_PacketCreatePlayer(void* data, int len);
void read_PacketBlockAction(void* data, int len);
void read_PacketStateData(void* data, int len);
void read_PacketKillAction(void* data, int len);
void read_PacketChatMessage(void* data, int len);

extern void (*packets[256]) (void* data, int len);
extern int network_connected;
extern int network_logged_in;
extern int network_map_transfer;
extern int network_received_packets;

extern float network_pos_update;
extern float network_orient_update;
extern unsigned char network_keys_last;
extern unsigned char network_buttons_last;
extern unsigned char network_tool_last;

#define VERSION_075		3
#define VERSION_076		4

extern struct network_stat {
	int outgoing;
	int ingoing;
	int avg_ping;
} network_stats[40];

extern float network_stats_last;

#pragma pack(push,1)

#define PACKET_POSITIONDATA_ID 0
struct PacketPositionData {
	unsigned char player_id;
	float x,y,z;
};

#define PACKET_ORIENTATIONDATA_ID 1
struct PacketOrientationData {
	unsigned char player_id;
	float x,y,z;
};

#define PACKET_MOVEMENTDATA_ID 2
struct PacketMovementData {
	unsigned char player_id;
	unsigned char keys;
};

#define PACKET_ANIMATIONDATA_ID 3
struct PacketAnimationData {
	unsigned char player_id;
	unsigned char fire, jump, crouch, aim;
};

#define PACKET_HIT_ID 4
struct PacketHit {
	unsigned char value;
	unsigned char player_id;
};
#define HITTYPE_TORSO	0
#define HITTYPE_HEAD	1
#define HITTYPE_ARMS	2
#define HITTYPE_LEGS	3
#define HITTYPE_SPADE	4

#define PACKET_GRENADE_ID 5
struct PacketGrenade {
	unsigned char player_id;
	float fuse_length;
};

#define PACKET_SETTOOL_ID 6
struct PacketSetTool {
	unsigned char player_id;
	unsigned char tool;
};
#define TOOL_SPADE		0
#define TOOL_GRENADE	1
#define TOOL_BLOCK		2
#define TOOL_GUN		3

#define PACKET_SETCOLOR_ID 7
struct PacketSetColor {
	unsigned char player_id;
	unsigned char blue,green,red;
};

#define PACKET_EXISTINGPLAYER_ID 8
struct PacketExistingPlayer {
	unsigned char player_id;
	unsigned char team;
	unsigned char weapon;
	unsigned char held_item;
	unsigned int kills;
	unsigned char blue,green,red;
	char name[17];
};
#define WEAPON_RIFLE	0
#define WEAPON_SMG		1
#define WEAPON_SHOTGUN	2

#define PACKET_INTELACTION_ID 9
struct PacketIntelAction {
	unsigned char player_id;
	unsigned char action_type;
	int x,y,z;
	unsigned char object_id;
	int team_1_flag_x, team_1_flag_y;
	int team_2_flag_x, team_2_flag_y;
	int team_1_base_x, team_1_base_y;
	int team_2_base_x, team_2_base_y;
	unsigned char winning;
};
#define MOVE			0
#define INTELPICKUP		1
#define INTELDROP		2
#define INTELCAPTURE	3
#define REFILL			4

#define TEAM_1_FLAG	0
#define TEAM_2_FLAG	1
#define TEAM_1_BASE	2
#define TEAM_2_BASE	3

#define PACKET_CREATEPLAYER_ID 10
struct PacketCreatePlayer {
	unsigned char player_id;
	int x,y;
	unsigned char weapon;
	char name[17];
};

#define PACKET_BLOCKACTION_ID 11
struct PacketBlockAction {
	unsigned char player_id;
	unsigned char action_type;
	int x,y,z;
};
#define ACTION_BUILD	0
#define ACTION_DESTROY	1
#define ACTION_SPADE	2
#define ACTION_GRENADE	3

#define PACKET_STATEDATA_ID 12
struct PacketStateData {
	unsigned char player_left;
	unsigned char player_id;

	union Gamemodes {
		struct GM_CTF {
			unsigned char team_1_score;
			unsigned char team_2_score;
			unsigned char capture_limit;
			unsigned char team_1_intel : 1;
			unsigned char team_2_intel : 1;
			union intel_location {
				struct {
					unsigned char player_id;
					unsigned char padding[11];
				} held;
				struct {
					int x,y,z;
				} dropped;
			} team_1_intel_location;
			union intel_location team_2_intel_location;
			struct {
				int x,y,z;
			} team_1_base;
			struct {
				int x,y,z;
			} team_2_base;
		} ctf;

		struct GM_TC {
			unsigned char territory_count;
			struct {
				float x,y,z;
				unsigned char team;
			} territory[16];
		} tc;
	} gamemode_data;
};

#define PACKET_KILLACTION_ID 13
struct PacketKillAction {
	unsigned char not_fall;
	unsigned char player_id;
	unsigned char killer_id;
};

#define PACKET_CHATMESSAGE_ID 14
struct PacketChatMessage {
	unsigned char chat_type;
	unsigned char player_id;
	char message[255];
};
#define CHAT_TEAM	0
#define CHAT_ALL	1

#pragma pack(pop)
