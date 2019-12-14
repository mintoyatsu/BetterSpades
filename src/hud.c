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

#include "common.h"
#include "http.h"

struct hud hud_mapload;

struct hud* hud_active;
struct window_instance* hud_window;

static int is_inside_centered(double mx, double my, int x, int y, int w, int h) {
	return mx>=x-w/2 && mx<x+w/2 && my>=y-h/2 && my<y+h/2;
}

static int is_inside(double mx, double my, int x, int y, int w, int h) {
	return mx>=x && mx<x+w && my>=y && my<y+h;
}

void hud_init() {
	hud_change(&hud_serverlist);
}

void hud_change(struct hud* new) {
	config_key_reset_togglestates();
	hud_active = new;
	if(hud_active->init)
		hud_active->init();
}

/* HUD_INGAME START */

int screen_current = SCREEN_NONE;
int show_exit = 0;
static void hud_ingame_init() {
	window_textinput(0);
	chat_input_mode = CHAT_NO_INPUT;
	show_exit = 0;
	window_mousemode(WINDOW_CURSOR_DISABLED);
}

struct player_table {
	unsigned char id;
	unsigned int score;
};

static int playertable_sort(const void* a, const void* b) {
	struct player_table* aa = (struct player_table*)a;
	struct player_table* bb = (struct player_table*)b;
	return bb->score-aa->score;
}

static void hud_ingame_render3D() {
	glDepthRange(0.0F,0.05F);

	float lambient[4] = {0.5F,0.5F,0.5F,1.0F};
	float ldiffuse[4] = {0.5F,0.5F,0.5F,1.0F};
	glLightfv(GL_LIGHT0,GL_AMBIENT,lambient);
	glLightfv(GL_LIGHT0,GL_DIFFUSE,ldiffuse);

	matrix_select(matrix_projection);
	matrix_identity();
	matrix_perspective(camera_fov,((float)settings.window_width)/((float)settings.window_height),0.1F,128.0F);
	matrix_select(matrix_view);
	matrix_identity();
	matrix_upload_p();
	matrix_select(matrix_model);

	if(!network_map_transfer) {
		if(camera_mode==CAMERAMODE_FPS && players[local_player_id].items_show)
			players[local_player_id].input.buttons.rmb = 0;

		if(window_key_down(WINDOW_KEY_HIDEHUD))
			return;

		if(camera_mode==CAMERAMODE_FPS && players[local_player_id].items_show) {
			matrix_identity();
			matrix_translate(-0.5F,3.5F-(players[local_player_id].held_item==TOOL_SPADE)*0.5F,-7.0F);
			matrix_rotate(window_time()*57.4F,0.0F,1.0F,0.0F);
			matrix_translate((model_spade.xpiv-model_spade.xsiz/2)*0.05F,(model_spade.zpiv-model_spade.zsiz/2)*0.05F,(model_spade.ypiv-model_spade.ysiz/2)*0.05F);
			if(players[local_player_id].held_item==TOOL_SPADE) {
				matrix_scale(1.5F,1.5F,1.5F);
			}
			matrix_upload();
			glViewport(-settings.window_width*0.4F,0.0F,settings.window_width,settings.window_height);
			kv6_render(&model_spade,players[local_player_id].team);
			glViewport(0.0F,0.0F,settings.window_width,settings.window_height);

			if(local_player_blocks>0) {
				matrix_identity();
				matrix_translate(-0.5F,3.5F-(players[local_player_id].held_item==TOOL_BLOCK)*0.5F,-7.0F);
				matrix_translate(1.0F,0.0F,0.0F); // offset from the previous weapon
				matrix_rotate(window_time()*57.4F,0.0F,1.0F,0.0F);
				matrix_translate((model_block.xpiv-model_block.xsiz/2)*0.05F,(model_block.zpiv-model_block.zsiz/2)*0.05F,(model_block.ypiv-model_block.ysiz/2)*0.05F);
				if(players[local_player_id].held_item==TOOL_BLOCK) {
					matrix_scale(1.5F,1.5F,1.5F);
				}
				model_block.red = players[local_player_id].block.red/255.0F;
				model_block.green = players[local_player_id].block.green/255.0F;
				model_block.blue = players[local_player_id].block.blue/255.0F;
				matrix_upload();
				glViewport(-settings.window_width*0.4F,0.0F,settings.window_width,settings.window_height);
				kv6_render(&model_block,players[local_player_id].team);
				glViewport(0.0F,0.0F,settings.window_width,settings.window_height);
			}

			if(local_player_ammo+local_player_ammo_reserved>0) {
				struct kv6_t* gun;
				switch(players[local_player_id].weapon) {
					default:
					case WEAPON_RIFLE:
						gun = &model_semi;
						break;
					case WEAPON_SMG:
						gun = &model_semi;
						break;
					case WEAPON_SHOTGUN:
						gun = &model_semi;
						break;
				}
				matrix_identity();
				matrix_translate(-0.5F,3.5F-(players[local_player_id].held_item==TOOL_GUN)*0.5F,-7.0F);
				matrix_translate(2.5F,0.0F,0.0F);
				matrix_rotate(window_time()*57.4F,0.0F,1.0F,0.0F);
				matrix_translate((gun->xpiv-gun->xsiz/2)*0.05F,(gun->zpiv-gun->zsiz/2)*0.05F,(gun->ypiv-gun->ysiz/2)*0.05F);
				if(players[local_player_id].held_item==TOOL_GUN) {
					matrix_scale(1.5F,1.5F,1.5F);
				}
				matrix_upload();
				glViewport(-settings.window_width*0.4F,0.0F,settings.window_width,settings.window_height);
				kv6_render(gun,players[local_player_id].team);
				glViewport(0.0F,0.0F,settings.window_width,settings.window_height);
			}

			if(local_player_grenades>0) {
				matrix_identity();
				matrix_translate(-0.5F,3.5F-(players[local_player_id].held_item==TOOL_GRENADE)*0.5F,-7.0F);
				matrix_translate(4.0F,0.0F,0.0F);
				matrix_rotate(window_time()*57.4F,0.0F,1.0F,0.0F);
				matrix_translate((model_grenade.xpiv-model_grenade.xsiz/2)*0.05F,(model_grenade.zpiv-model_grenade.zsiz/2)*0.05F,(model_grenade.ypiv-model_grenade.ysiz/2)*0.05F);
				if(players[local_player_id].held_item==TOOL_GRENADE) {
					matrix_scale(1.5F,1.5F,1.5F);
				}
				matrix_upload();
				glViewport(-settings.window_width*0.4F,0.0F,settings.window_width,settings.window_height);
				kv6_render(&model_grenade,players[local_player_id].team);
				glViewport(0.0F,0.0F,settings.window_width,settings.window_height);
			}
		}

		if(screen_current==SCREEN_TEAM_SELECT) {
			matrix_identity();
			matrix_translate(-1.5F,-1.25F,-3.25F);
			matrix_rotate(window_time()*90.0F,0.0F,1.0F,0.0F);
			matrix_translate((model_team.xpiv-model_team.xsiz/2.0F)*model_team.scale,
							(model_team.zpiv-model_team.zsiz/2.0F)*model_team.scale,
							(model_team.ypiv-model_team.ysiz/2.0F)*model_team.scale);
			matrix_scale(8.0F,8.0F,8.0F);
			matrix_upload();
			kv6_render(&model_team,TEAM_1);

			matrix_identity();
			matrix_translate(1.5F,-1.25F,-3.25F);
			matrix_rotate(window_time()*90.0F,0.0F,1.0F,0.0F);
			matrix_translate((model_team.xpiv-model_team.xsiz/2.0F)*model_team.scale,
							(model_team.zpiv-model_team.zsiz/2.0F)*model_team.scale,
							(model_team.ypiv-model_team.ysiz/2.0F)*model_team.scale);
			matrix_scale(8.0F,8.0F,8.0F);
			matrix_upload();
			kv6_render(&model_team,TEAM_2);
		}

		struct kv6_t* rotating_model = NULL;
		int rotating_model_team = TEAM_SPECTATOR;
		if(gamestate.gamemode_type==GAMEMODE_CTF) {
			switch(players[local_player_id].team) {
				case TEAM_1:
					if(gamestate.gamemode.ctf.team_2_intel && gamestate.gamemode.ctf.team_2_intel_location.held.player_id==local_player_id) {
						rotating_model = &model_intel;
						rotating_model_team = TEAM_2;
					}
					break;
				case TEAM_2:
					if(gamestate.gamemode.ctf.team_1_intel && gamestate.gamemode.ctf.team_1_intel_location.held.player_id==local_player_id) {
						rotating_model = &model_intel;
						rotating_model_team = TEAM_1;
					}
					break;
			}
		}
		if(gamestate.gamemode_type==GAMEMODE_TC) {
			for(int k=0;k<gamestate.gamemode.tc.territory_count;k++) {
				float l = pow(gamestate.gamemode.tc.territory[k].x-players[local_player_id].pos.x,2.0F)
						 +pow((63.0F-gamestate.gamemode.tc.territory[k].z)-players[local_player_id].pos.y,2.0F)
						 +pow(gamestate.gamemode.tc.territory[k].y-players[local_player_id].pos.z,2.0F);
				if(l<=20.0F*20.0F) {
					rotating_model = &model_tent;
					rotating_model_team = gamestate.gamemode.tc.territory[k].team;
					break;
				}
			}
		}
		if(rotating_model) {
			matrix_identity();
			matrix_translate(0.0F,-(rotating_model->zsiz*0.5F+rotating_model->zpiv)*rotating_model->scale,-10.0F);
			matrix_rotate(window_time()*90.0F,0.0F,1.0F,0.0F);
			matrix_upload();
			glViewport(-settings.window_width*0.4F,settings.window_height*0.1F,settings.window_width,settings.window_height);
			kv6_render(rotating_model,rotating_model_team);
			glViewport(0.0F,0.0F,settings.window_width,settings.window_height);
		}
	}
}

static void hud_ingame_render(float scalex, float scalef) {
	//window_mousemode(camera_mode==CAMERAMODE_SELECTION?WINDOW_CURSOR_ENABLED:WINDOW_CURSOR_DISABLED);
	hud_active->render_localplayer = players[local_player_id].team!=TEAM_SPECTATOR && (screen_current==SCREEN_NONE || camera_mode!=CAMERAMODE_FPS);

	if(window_key_down(WINDOW_KEY_NETWORKSTATS)) {
		if(network_map_transfer)
			glColor3f(1.0F,1.0F,1.0F);
		else
			glColor3f(0.0F,0.0F,0.0F);
		glEnable(GL_DEPTH_TEST);
		glColorMask(0,0,0,0);
		texture_draw_empty(11.0F*scalex,380.0F*scalef,160.0F*scalef,160.0F*scalef);
		glColorMask(1,1,1,1);
		glDepthFunc(GL_NOTEQUAL);
		texture_draw_empty(10.0F*scalex,381.0F*scalef,162.0F*scalef,162.0F*scalef);
		glDepthFunc(GL_LEQUAL);
		glDisable(GL_DEPTH_TEST);
		char dbg_str[32];

		int max = 0;
		for(int k=0;k<40;k++) {
			max = max(max,network_stats[k].ingoing+network_stats[k].outgoing);
		}
		for(int k=0;k<40;k++) {
			float in_h = (float)(network_stats[39-k].ingoing)/max*160.0F;
			float out_h = (float)(network_stats[39-k].ingoing+network_stats[39-k].outgoing)/max*160.0F;
			float ping_h = min(network_stats[39-k].avg_ping/25.0F,160.0F);

			glColor3f(0.0F,0.0F,1.0F);
			texture_draw_empty(11.0F*scalex+4*k*scalef,(220.0F+out_h)*scalef,4.0F*scalef,out_h*scalef);
			if(!k) {
				sprintf(dbg_str,"Out: %i b/s",network_stats[1].outgoing);
				font_render(11.0F*scalex+80*scalef,212.0F*scalef,8.0F*scalef,dbg_str);
			}

			glColor3f(0.0F,1.0F,0.0F);
			texture_draw_empty(11.0F*scalex+4*k*scalef,(220.0F+in_h)*scalef,4.0F*scalef,in_h*scalef);
			if(!k) {
				sprintf(dbg_str,"In: %i b/s",network_stats[1].ingoing);
				font_render(11.0F*scalex,212.0F*scalef,8.0F*scalef,dbg_str);
			}

			glColor3f(1.0F,0.0F,0.0F);
			texture_draw_empty(11.0F*scalex+4*k*scalef,(220.0F+ping_h)*scalef,4.0F*scalef,ping_h*scalef);
			if(!k) {
				sprintf(dbg_str,"Ping: %i",network_stats[1].avg_ping);
				font_render(11.0F*scalex,202.0F*scalef,8.0F*scalef,dbg_str);
			}
		}
		glColor3f(1.0F,1.0F,1.0F);
	}

	if(network_map_transfer) {
		float p = (compressed_chunk_data_estimate>0)?((float)compressed_chunk_data_offset/(float)compressed_chunk_data_estimate):0.0F;;
		glColor3ub(64,64,64);
		texture_draw_empty((settings.window_width-440.0F*scalef)/2.0F+440.0F*scalef*p,settings.window_height*0.5F,440.0F*scalef*(1.0F-p),2.0F*scalef);
		glColor3ub(0,255,255);
		texture_draw_empty((settings.window_width-440.0F*scalef)/2.0F,settings.window_height*0.5F,440.0F*scalef*p,2.0F*scalef);

		glColor3ub(255,255,255);
		char str[128];
		sprintf(str,"Downloading Map (%iKB/%iKB)",compressed_chunk_data_offset/1024,compressed_chunk_data_estimate/1024);
		font_centered(settings.window_width/2.0F,settings.window_height/2.0F+15*scalef,8*scalef,str);
	} else {
		if(window_key_down(WINDOW_KEY_HIDEHUD))
			return;

		if(screen_current==SCREEN_TEAM_SELECT) {
			glColor3f(1.0F,0.0F,0.0F);
			char join_str[48];
			sprintf(join_str,"Press 1 to join %s",gamestate.team_1.name);
			font_centered(settings.window_width/4.0F,61*scalef,8.0F*scalef,join_str);
			sprintf(join_str,"Press 2 to join %s",gamestate.team_2.name);
			font_centered(settings.window_width/4.0F*3.0F,61*scalef,8.0F*scalef,join_str);
			font_centered(settings.window_width/2.0F,61*scalef,8.0F*scalef,"Press 3 to spectate");
			glColor3f(1.0F,1.0F,1.0F);
		}

		if(window_key_down(WINDOW_KEY_TAB) || camera_mode==CAMERAMODE_SELECTION) {
			font_select(FONT_KNUMB);
			char score_str[8];
			glColor3ub(gamestate.team_1.red,gamestate.team_1.green,gamestate.team_1.blue);
			switch(gamestate.gamemode_type) {
				case GAMEMODE_CTF:
					sprintf(score_str,"%i-%i",gamestate.gamemode.ctf.team_1_score,gamestate.gamemode.ctf.capture_limit);
					break;
				case GAMEMODE_TC:
				{
					int t = 0;
					for(int k=0;k<gamestate.gamemode.tc.territory_count;k++)
						if(gamestate.gamemode.tc.territory[k].team==TEAM_1)
							t++;
					sprintf(score_str,"%i-%i",t,gamestate.gamemode.tc.territory_count);
					break;
				}
			}
			font_centered(settings.window_width/4.0F,519*scalef,32.0F*scalef,score_str);
			glColor3ub(gamestate.team_2.red,gamestate.team_2.green,gamestate.team_2.blue);
			switch(gamestate.gamemode_type) {
				case GAMEMODE_CTF:
					sprintf(score_str,"%i-%i",gamestate.gamemode.ctf.team_2_score,gamestate.gamemode.ctf.capture_limit);
					break;
				case GAMEMODE_TC:
				{
					int t = 0;
					for(int k=0;k<gamestate.gamemode.tc.territory_count;k++)
						if(gamestate.gamemode.tc.territory[k].team==TEAM_2)
							t++;
					sprintf(score_str,"%i-%i",t,gamestate.gamemode.tc.territory_count);
					break;
				}
			}
			font_centered(settings.window_width/4.0F*3.0F,519*scalef,32.0F*scalef,score_str);
			font_select(FONT_SMALLFNT);

			struct player_table pt[PLAYERS_MAX];
			int connected = 0;
			for(int k=0;k<PLAYERS_MAX;k++) {
				if(players[k].connected) {
					pt[connected].id = k;
					pt[connected++].score = players[k].score;
				}
			}
			qsort(pt,connected,sizeof(struct player_table),playertable_sort);

			int cntt[3] = {0};
			for(int k=0;k<connected;k++) {
				int mul = 0;
				switch(players[pt[k].id].team) {
					case TEAM_1:
						mul = 1;
						glColor3ub(gamestate.team_1.red,gamestate.team_1.green,gamestate.team_1.blue);
						break;
					case TEAM_2:
						mul = 3;
						glColor3ub(gamestate.team_2.red,gamestate.team_2.green,gamestate.team_2.blue);
						break;
					default:
					case TEAM_SPECTATOR:
						mul = 2;
						glColor3f(1.0F,1.0F,1.0F);
						break;
				}
				char id_str[16];
				sprintf(id_str,"#%i",pt[k].id);
				font_render(settings.window_width/4.0F*mul-font_length(8.0F*scalef,players[pt[k].id].name),(459-14*cntt[mul-1])*scalef,8.0F*scalef,players[pt[k].id].name);
				font_render(settings.window_width/4.0F*mul+8.82F*scalef,(459-14*cntt[mul-1])*scalef,8.0F*scalef,id_str);
				if(mul!=2) {
					sprintf(id_str,"%i",pt[k].score);
					font_render(settings.window_width/4.0F*mul+44.1F*scalef,(459-14*cntt[mul-1])*scalef,8.0F*scalef,id_str);
				}
				cntt[mul-1]++;
			}
		}

		int is_local = (camera_mode==CAMERAMODE_FPS) || (cameracontroller_bodyview_player==local_player_id);
		int local_id = (camera_mode==CAMERAMODE_FPS)?local_player_id:cameracontroller_bodyview_player;

		if(camera_mode==CAMERAMODE_FPS || ((camera_mode==CAMERAMODE_BODYVIEW || camera_mode==CAMERAMODE_SPECTATOR) && cameracontroller_bodyview_mode)) {
			glColor3f(1.0F,1.0F,1.0F);
			if(players[local_id].held_item==TOOL_GUN && players[local_id].input.buttons.rmb && players[local_id].alive) {
				struct texture* zoom;
				switch(players[local_id].weapon) {
					case WEAPON_RIFLE:
						zoom = &texture_zoom_semi;
						break;
					case WEAPON_SMG:
						zoom = &texture_zoom_semi;
						break;
					case WEAPON_SHOTGUN:
						zoom = &texture_zoom_semi;
						break;
				}
				float last_shot = is_local?weapon_last_shot:players[local_id].gun_shoot_timer;
				float zoom_factor = max(0.25F*(1.0F-((window_time()-last_shot)/weapon_delay(players[local_id].weapon)))+1.0F,1.0F);
				texture_draw(zoom,(settings.window_width-settings.window_height*4.0F/3.0F*zoom_factor)/2.0F,settings.window_height*(zoom_factor*0.5F+0.5F),settings.window_height*4.0F/3.0F*zoom_factor,settings.window_height*zoom_factor);
			} else {
				texture_draw(&texture_target,(settings.window_width-16)/2.0F,(settings.window_height+16)/2.0F,16,16);
			}

			font_select(FONT_KNUMB);
			int health = is_local?(players[local_id].alive?local_player_health:0):(players[local_id].alive?100:0);
			if(window_time()-local_player_last_damage_timer<=0.5F && is_local) // TODO: in .54 number gradually turns red depending on health
				glColor3f(1,0,0);
			else
				glColor3f(1,1,1);
			char hp[4];
			sprintf(hp,"%i",health);
			font_centered(settings.window_width/2.0F,44.0F*scalef,32.0F*scalef,hp);

			char item_mini_str[32];
			int off = 0;
			glColor3f(1.0F,1.0F,1.0F);
			switch(players[local_id].held_item) {
				default:
				case TOOL_BLOCK:
					off = 64*scalef;
					// fallthrough
				case TOOL_SPADE:
					sprintf(item_mini_str,"%i",is_local?local_player_blocks:50);
					break;
				case TOOL_GRENADE:
					sprintf(item_mini_str,"%i",is_local?local_player_grenades:3);
					break;
				case TOOL_GUN:
				{
					int ammo = is_local?local_player_ammo:players[local_id].ammo;
					int ammo_reserve = is_local?local_player_ammo_reserved:players[local_id].ammo_reserved;
					sprintf(item_mini_str,"%i-%i",ammo,ammo_reserve);
					if(ammo==0)
						glColor3f(1.0F,0.0F,0.0F);
					break;
				}
			}
			font_render(settings.window_width-font_length(32.0F*scalef,item_mini_str)-11.0F*scalef-off,44.0F*scalef,32.0F*scalef,item_mini_str);
			font_select(FONT_SMALLFNT);
			glColor3f(1.0F,1.0F,1.0F);

			if(players[local_id].held_item==TOOL_BLOCK) {
				for(int y=0;y<8;y++) {
					for(int x=0;x<8;x++) {
						if(texture_block_color(x,y)==players[local_id].block.packed) {
							unsigned char g = (((int)(window_time()*4))&1)*0xFF;
							glColor3ub(g,g,g);
							texture_draw_empty(settings.window_width+(x*8-65)*scalef,(65-y*8)*scalef,8*scalef,8*scalef);
							y = 10; //to break outer loop too
							break;
						}
					}
				}
				glColor3f(1.0F,1.0F,1.0F);
				texture_draw(&texture_color_selection,settings.window_width-64*scalef,64*scalef,64*scalef,64*scalef);
			}
		}

		if(camera_mode!=CAMERAMODE_SELECTION) {
			glColor3f(1.0F,1.0F,1.0F);
			if(chat_input_mode!=CHAT_NO_INPUT) {
				switch(chat_input_mode) {
					case CHAT_ALL_INPUT:
						font_render(11.0F*scalef,settings.window_height*0.20F+20.0F*scalef,8.0F*scalef,"Global:");
						break;
					case CHAT_TEAM_INPUT:
						font_render(11.0F*scalef,settings.window_height*0.20F+20.0F*scalef,8.0F*scalef,"Team:");
						break;
				}
				int l = strlen(chat[0][0]);
				chat[0][0][l] = '_';
				chat[0][0][l+1] = 0;
				font_render(11.0F*scalef,settings.window_height*0.20F+10.0F*scalef,8.0F*scalef,chat[0][0]);
				chat[0][0][l] = 0;
			}
			for(int k=0;k<6;k++) {
				if(window_time()-chat_timer[0][k+1]<10.0F || chat_input_mode!=CHAT_NO_INPUT) { // chat
					glColor3ub(red(chat_color[0][k+1]),green(chat_color[0][k+1]),blue(chat_color[0][k+1]));
					font_render(11.0F*scalef,settings.window_height*0.20F-10.0F*scalef*k,8.0F*scalef,chat[0][k+1]);
				}
				if(window_time()-chat_timer[1][k+1]<10.0F) { // killfeed
					glColor3ub(red(chat_color[1][k+1]),green(chat_color[1][k+1]),blue(chat_color[1][k+1]));
					font_render(settings.window_width-font_length(8.0F*scalef,chat[1][k+1])-11.0F*scalef,settings.window_height-15.0F*scalef-10.0F*scalef*k,8.0F*scalef,chat[1][k+1]);
				}
			}
			glColor3f(1.0F,1.0F,1.0F);
		}

		if(gamestate.gamemode_type==GAMEMODE_TC
			&& gamestate.progressbar.tent<gamestate.gamemode.tc.territory_count
			&& gamestate.gamemode.tc.territory[gamestate.progressbar.tent].team!=gamestate.progressbar.team_capturing) {
			float p = max(min(gamestate.progressbar.progress+0.05F*gamestate.progressbar.rate*(window_time()-gamestate.progressbar.update),1.0F),0.0F);
			float l = pow(gamestate.gamemode.tc.territory[gamestate.progressbar.tent].x-players[local_player_id].pos.x,2.0F)
					 +pow((63.0F-gamestate.gamemode.tc.territory[gamestate.progressbar.tent].z)-players[local_player_id].pos.y,2.0F)
					 +pow(gamestate.gamemode.tc.territory[gamestate.progressbar.tent].y-players[local_player_id].pos.z,2.0F);
			if(p<1.0F && l<20.0F*20.0F) {
				switch(gamestate.gamemode.tc.territory[gamestate.progressbar.tent].team) {
					case TEAM_1:
						glColor3ub(gamestate.team_1.red,gamestate.team_1.green,gamestate.team_1.blue);
						break;
					case TEAM_2:
						glColor3ub(gamestate.team_2.red,gamestate.team_2.green,gamestate.team_2.blue);
						break;
					default:
						glColor3f(1.0F,1.0F,1.0F);
				}
				texture_draw(&texture_white,(settings.window_width-440.0F*scalef)/2.0F+440.0F*scalef*p,settings.window_height*0.25F,440.0F*scalef*(1.0F-p),20.0F*scalef);
				switch(gamestate.progressbar.team_capturing) {
					case TEAM_1:
						glColor3ub(gamestate.team_1.red,gamestate.team_1.green,gamestate.team_1.blue);
						break;
					case TEAM_2:
						glColor3ub(gamestate.team_2.red,gamestate.team_2.green,gamestate.team_2.blue);
						break;
					default:
						glColor3f(1.0F,1.0F,1.0F);
				}
				texture_draw(&texture_white,(settings.window_width-440.0F*scalef)/2.0F,settings.window_height*0.25F,440.0F*scalef*p,20.0F*scalef);
			}
		}

		if(camera_mode!=CAMERAMODE_SELECTION) {
			glColor3f(1.0F,1.0F,1.0F);
			if(window_key_down(WINDOW_KEY_MAP)) {
				float minimap_x = (settings.window_width-(map_size_x+1)*scalef)/2.0F;
				float minimap_y = ((600-map_size_z-1)/2.0F+map_size_z+1)*scalef;
				texture_draw(&texture_minimap,minimap_x,minimap_y,512*scalef,512*scalef);

				char c[2] = {0};
				for(int k=0;k<8;k++) {
					c[0] = 'A'+k;
					font_centered(minimap_x+(64*k+32)*scalef,minimap_y+8.0F*scalef,8.0F*scalef,c);
					c[0] = '1'+k;
					font_centered(minimap_x-8*scalef,minimap_y-(64*k+32-4)*scalef,8.0F*scalef,c);
				}

				if(gamestate.gamemode_type==GAMEMODE_CTF) {
					if(!gamestate.gamemode.ctf.team_1_intel) {
						glColor3ub(gamestate.team_1.red,gamestate.team_1.green,gamestate.team_1.blue);
						texture_draw_empty_rotated(minimap_x+(gamestate.gamemode.ctf.team_1_intel_location.dropped.x-3)*scalef,minimap_y-gamestate.gamemode.ctf.team_1_intel_location.dropped.y*scalef,1*scalef,7*scalef,0.0F);
						texture_draw_empty_rotated(minimap_x+(gamestate.gamemode.ctf.team_1_intel_location.dropped.x+3)*scalef,minimap_y-gamestate.gamemode.ctf.team_1_intel_location.dropped.y*scalef,1*scalef,7*scalef,0.0F);
						texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_1_intel_location.dropped.y+3)*scalef,7*scalef,1*scalef,0.0F);
						texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_1_intel_location.dropped.y-3)*scalef,7*scalef,1*scalef,0.0F);
						glColor3f(1.0F,1.0F,1.0F);
						if (63.0F-gamestate.gamemode.ctf.team_1_intel_location.dropped.z+1.0F > players[local_player_id].pos.y-1.0F) {
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_1_intel_location.dropped.y-4)*scalef,7*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_1_intel_location.dropped.y-5)*scalef,5*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_1_intel_location.dropped.y-6)*scalef,3*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_1_intel_location.dropped.y-7)*scalef,1*scalef,1*scalef,0.0F);
						} else if (63.0F-gamestate.gamemode.ctf.team_1_intel_location.dropped.z+1.0F < players[local_player_id].pos.y-2.0F) {
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_1_intel_location.dropped.y+4)*scalef,7*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_1_intel_location.dropped.y+5)*scalef,5*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_1_intel_location.dropped.y+6)*scalef,3*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_1_intel_location.dropped.y+7)*scalef,1*scalef,1*scalef,0.0F);
						}
					}
					if(map_object_visible(gamestate.gamemode.ctf.team_1_base.x,0.0F,gamestate.gamemode.ctf.team_1_base.y)) {
						glColor3ub(gamestate.team_1.red,gamestate.team_1.green,gamestate.team_1.blue);
						texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_base.x*scalef,minimap_y-gamestate.gamemode.ctf.team_1_base.y*scalef,7*scalef,1*scalef,0.0F);
						texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_1_base.x*scalef,minimap_y-gamestate.gamemode.ctf.team_1_base.y*scalef,1*scalef,7*scalef,0.0F);
					}

					if(!gamestate.gamemode.ctf.team_2_intel) {
						glColor3ub(gamestate.team_2.red,gamestate.team_2.green,gamestate.team_2.blue);
						texture_draw_empty_rotated(minimap_x+(gamestate.gamemode.ctf.team_2_intel_location.dropped.x-3)*scalef,minimap_y-gamestate.gamemode.ctf.team_2_intel_location.dropped.y*scalef,1*scalef,7*scalef,0.0F);
						texture_draw_empty_rotated(minimap_x+(gamestate.gamemode.ctf.team_2_intel_location.dropped.x+3)*scalef,minimap_y-gamestate.gamemode.ctf.team_2_intel_location.dropped.y*scalef,1*scalef,7*scalef,0.0F);
						texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_2_intel_location.dropped.y+3)*scalef,7*scalef,1*scalef,0.0F);
						texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_2_intel_location.dropped.y-3)*scalef,7*scalef,1*scalef,0.0F);
						glColor3f(1.0F,1.0F,1.0F);
						if (63.0F-gamestate.gamemode.ctf.team_2_intel_location.dropped.z+1.0F > players[local_player_id].pos.y) {
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_2_intel_location.dropped.y-4)*scalef,7*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_2_intel_location.dropped.y-5)*scalef,5*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_2_intel_location.dropped.y-6)*scalef,3*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_2_intel_location.dropped.y-7)*scalef,1*scalef,1*scalef,0.0F);
						} else if (63.0F-gamestate.gamemode.ctf.team_2_intel_location.dropped.z+1.0F < players[local_player_id].pos.y) {
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_2_intel_location.dropped.y+4)*scalef,7*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_2_intel_location.dropped.y+5)*scalef,5*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_2_intel_location.dropped.y+6)*scalef,3*scalef,1*scalef,0.0F);
							texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_intel_location.dropped.x*scalef,minimap_y-(gamestate.gamemode.ctf.team_2_intel_location.dropped.y+7)*scalef,1*scalef,1*scalef,0.0F);
						}
					}
					if(map_object_visible(gamestate.gamemode.ctf.team_2_base.x,0.0F,gamestate.gamemode.ctf.team_2_base.y)) {
						glColor3ub(gamestate.team_2.red,gamestate.team_2.green,gamestate.team_2.blue);
						texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_base.x*scalef,minimap_y-gamestate.gamemode.ctf.team_2_base.y*scalef,7*scalef,1*scalef,0.0F);
						texture_draw_empty_rotated(minimap_x+gamestate.gamemode.ctf.team_2_base.x*scalef,minimap_y-gamestate.gamemode.ctf.team_2_base.y*scalef,1*scalef,7*scalef,0.0F);
					}
				}
				if(gamestate.gamemode_type==GAMEMODE_TC) {
					for(int k=0;k<gamestate.gamemode.tc.territory_count;k++) {
						switch(gamestate.gamemode.tc.territory[k].team) {
							case TEAM_1:
								glColor3ub(gamestate.team_1.red,gamestate.team_1.green,gamestate.team_1.blue);
								break;
							case TEAM_2:
								glColor3ub(gamestate.team_2.red,gamestate.team_2.green,gamestate.team_2.blue);
								break;
							default:
							case TEAM_SPECTATOR:
								glColor3f(0.0F,0.0F,0.0F);
						}
						texture_draw_empty_rotated(minimap_x+gamestate.gamemode.tc.territory[k].x*scalef,minimap_y-gamestate.gamemode.tc.territory[k].y*scalef,7*scalef,1*scalef,0.0F);
						texture_draw_empty_rotated(minimap_x+gamestate.gamemode.tc.territory[k].x*scalef,minimap_y-gamestate.gamemode.tc.territory[k].y*scalef,1*scalef,7*scalef,0.0F);
					}
				}

				for(int k=0;k<PLAYERS_MAX;k++) {
					if(players[k].connected && players[k].alive && k!=local_player_id && players[k].team!=TEAM_SPECTATOR && (players[k].team==players[local_player_id].team || camera_mode==CAMERAMODE_SPECTATOR)) {
						if (camera_mode==CAMERAMODE_SPECTATOR) {
							switch(players[k].team) {
								case TEAM_1:
									glColor3ub(gamestate.team_1.red,gamestate.team_1.green,gamestate.team_1.blue);
									break;
								case TEAM_2:
									glColor3ub(gamestate.team_2.red,gamestate.team_2.green,gamestate.team_2.blue);
									break;
							}
						} else {
							if (players[k].team==players[local_player_id].team) {
								glColor3f(1.0F,1.0F,1.0F);
							} else {
								glColor3f(1.0F,0.0F,0.0F);
							}
						}
						texture_draw_rotated(&texture_icon,minimap_x+players[k].pos.x*scalef,minimap_y-players[k].pos.z*scalef,7*scalef,7*scalef,0.0F);
					}
				}

				if (camera_mode==CAMERAMODE_FPS) {
					glColor3f(0.0F,1.0F,1.0F);
					texture_draw_rotated(&texture_icon,minimap_x+camera_x*scalef,minimap_y-camera_z*scalef,7*scalef,7*scalef,0.0F);
				}
				glColor3f(1.0F,1.0F,1.0F);
			}
		}

		if(player_intersection_type>=0 && (players[local_player_id].team==TEAM_SPECTATOR || players[player_intersection_player].team==players[local_player_id].team)) {
			char str[32];
			switch(players[player_intersection_player].team) {
				case TEAM_1:
					glColor3ub(gamestate.team_1.red,gamestate.team_1.green,gamestate.team_1.blue);
					break;
				case TEAM_2:
					glColor3ub(gamestate.team_2.red,gamestate.team_2.green,gamestate.team_2.blue);
					break;
				default:
					glColor3f(1.0F,1.0F,1.0F);
			}
			sprintf(str,"%s",players[player_intersection_player].name);
			font_centered(settings.window_width/2.0F,settings.window_height*0.2F,8.0F*scalef,str);
		}

		if(show_exit) {
			glColor3f(1.0F,0.0F,0.0F);
			font_render((settings.window_width-font_length(8.0F*scalef,"EXIT GAME? Y/N"))/2.0F,settings.window_height/2.0F+44.0F*scalef,8.0F*scalef,"EXIT GAME? Y/N");
		}
		if(window_time()-chat_popup_timer<chat_popup_duration) {
			glColor3ub(red(chat_popup_color),green(chat_popup_color),blue(chat_popup_color));
			font_render((settings.window_width-font_length(8.0F*scalef,chat_popup))/2.0F,settings.window_height/2.0F+32.0F*scalef,8.0F*scalef,chat_popup);
		}
		glColor3f(1.0F,1.0F,1.0F);
	}

	if(settings.show_fps) {
		char debug_str[16];
		int debug_ping = network_ping();
		int debug_fps = (int)fps;
		if (debug_ping >= 250) {
			glColor3f(1.0F,0.0F,0.0F);
		}
		else if (debug_ping >= 150) {
			glColor3f(1.0F,1.0F,0.0F);
		}
		else {
			glColor3f(0.0F,1.0F,0.0F);
		}
		sprintf(debug_str,"Ping: %ims",debug_ping);
		font_render(11.0F*scalef,settings.window_height-15.0F,8.0F*scalef,debug_str);
		if (debug_fps >= 50) {
			glColor3f(0.0F,1.0F,0.0F);
		}
		else if (debug_fps >= 25) {
			glColor3f(1.0F,1.0F,0.0F);
		}
		else {
			glColor3f(1.0F,0.0F,0.0F);
		}
		sprintf(debug_str,"FPS: %i",debug_fps);
		font_render(11.0F*scalef,settings.window_height-25.0F*scalef,8.0F*scalef,debug_str);
		glColor3f(1.0F,1.0F,1.0F);
	}
}

static void hud_ingame_scroll(double yoffset) {
	if(camera_mode==CAMERAMODE_FPS && yoffset!=0.0F) {
		int h = players[local_player_id].held_item;
		if(!players[local_player_id].items_show)
			local_player_lasttool = h;
		h += (yoffset<0)?1:-1;
		if(h<0)
			h = 3;
		if(h==TOOL_BLOCK && local_player_blocks==0)
			h += (yoffset<0)?1:-1;
		if(h==TOOL_GUN && local_player_ammo+local_player_ammo_reserved==0)
			h += (yoffset<0)?1:-1;
		if(h==TOOL_GRENADE && local_player_grenades==0)
			h += (yoffset<0)?1:-1;
		if(h>3)
			h = 0;
		players[local_player_id].held_item = h;
		sound_create(NULL,SOUND_LOCAL,&sound_switch,0.0F,0.0F,0.0F)->stick_to_player = local_player_id;
		players[local_player_id].item_disabled = window_time();
		players[local_player_id].items_show_start = window_time();
		players[local_player_id].items_show = 1;
	}
}

static double last_x, last_y;
static void hud_ingame_mouselocation(double x, double y) {
	if(show_exit) {
		last_x = x;
		last_y = y;
		return;
	}
	float dx = x-last_x;
	float dy = y-last_y;
	last_x = x;
	last_y = y;

	float s = 1.0F;
	if(camera_mode==CAMERAMODE_FPS && players[local_player_id].held_item==TOOL_GUN && players[local_player_id].input.buttons.rmb) {
		s = 0.5F;
	}

	if(settings.invert_y)
		dy *= -1.0F;

	camera_rot_x -= dx*settings.mouse_sensitivity/5.0F*(float)MOUSE_SENSITIVITY*s;
	camera_rot_y += dy*settings.mouse_sensitivity/5.0F*(float)MOUSE_SENSITIVITY*s;

	camera_overflow_adjust();
}

static void hud_ingame_mouseclick(double x, double y, int button, int action, int mods) {
	if(button==WINDOW_MOUSE_LMB) {
		button_map[0] = (action==WINDOW_PRESS);
	}
	if(button==WINDOW_MOUSE_RMB) {
		if(action==WINDOW_PRESS && players[local_player_id].held_item==TOOL_GUN) {
			players[local_player_id].input.buttons.rmb ^= 1;
			if(players[local_player_id].items_show) {
				players[local_player_id].input.buttons.rmb = 0;
			}
		}
		if(local_player_drag_active && action==WINDOW_RELEASE && players[local_player_id].held_item==TOOL_BLOCK) {
			int* pos = camera_terrain_pick(0);
			if(pos!=NULL && pos[1]>1 && (pow(pos[0]-camera_x,2)+pow(pos[1]-camera_y,2)+pow(pos[2]-camera_z,2))<5*5) {
				int amount = map_cube_line(local_player_drag_x,local_player_drag_z,63-local_player_drag_y,pos[0],pos[2],63-pos[1],NULL);
				if(amount<=local_player_blocks) {
					struct PacketBlockLine line;
					line.player_id = local_player_id;
					line.sx = local_player_drag_x;
					line.sy = local_player_drag_z;
					line.sz = 63-local_player_drag_y;
					line.ex = pos[0];
					line.ey = pos[2];
					line.ez = 63-pos[1];
					network_send(PACKET_BLOCKLINE_ID,&line,sizeof(line));
					local_player_blocks -= amount;
				}
				players[local_player_id].item_showup = window_time();
			}
		}
		local_player_drag_active = 0;
		if(action==WINDOW_PRESS && players[local_player_id].held_item==TOOL_BLOCK && window_time()-players[local_player_id].item_showup>=0.5F) {
			int* pos = camera_terrain_pick(0);
			if(pos!=NULL && pos[1]>1 && distance3D(camera_x,camera_y,camera_z,pos[0],pos[1],pos[2])<5.0F*5.0F) {
				local_player_drag_active = 1;
				local_player_drag_x = pos[0];
				local_player_drag_y = pos[1];
				local_player_drag_z = pos[2];
			}
		}
		button_map[1] = (action==WINDOW_PRESS);
	}
	if(button==WINDOW_MOUSE_MMB) {
		button_map[2] = (action==WINDOW_PRESS);
	}
	if(camera_mode==CAMERAMODE_BODYVIEW && button==WINDOW_MOUSE_MMB && action==WINDOW_PRESS) {
		float nearest_dist = FLT_MAX;
		int nearest_player = -1;
		for(int k=0;k<PLAYERS_MAX;k++)
			if(player_can_spectate(&players[k]) && players[k].alive && k!=cameracontroller_bodyview_player
			&& distance3D(camera_x,camera_y,camera_z,players[k].pos.x,players[k].pos.y,players[k].pos.z)<nearest_dist) {
				nearest_dist = distance3D(camera_x,camera_y,camera_z,players[k].pos.x,players[k].pos.y,players[k].pos.z);
				nearest_player = k;
			}
		if(nearest_player>=0)
			cameracontroller_bodyview_player = nearest_player;
	}
	if(button==WINDOW_MOUSE_RMB && action==WINDOW_PRESS) {
		players[local_player_id].input.buttons.rmb_start = window_time();
		if(camera_mode==CAMERAMODE_BODYVIEW || camera_mode==CAMERAMODE_SPECTATOR) {
			if(camera_mode==CAMERAMODE_SPECTATOR)
				cameracontroller_bodyview_mode = 1;
			for(int k=0;k<PLAYERS_MAX*2;k++) {
				cameracontroller_bodyview_player = (cameracontroller_bodyview_player+1)%PLAYERS_MAX;
				if(player_can_spectate(&players[cameracontroller_bodyview_player]))
					break;
			}
			cameracontroller_bodyview_zoom = 0.0F;
		}
	}
	if(button==WINDOW_MOUSE_LMB) {
		if(camera_mode==CAMERAMODE_FPS && window_time()-players[local_player_id].item_showup>=0.5F) {
			if(players[local_player_id].held_item==TOOL_GRENADE && local_player_grenades>0) {
				if(action==WINDOW_RELEASE) {
					local_player_grenades = max(local_player_grenades-1,0);
					struct PacketGrenade g;
					g.player_id = local_player_id;
					g.fuse_length = max(3.0F-(window_time()-players[local_player_id].input.buttons.lmb_start),0.0F);
					g.x = players[local_player_id].pos.x;
					g.y = players[local_player_id].pos.z;
					g.z = 63.0F-players[local_player_id].pos.y;
					g.vx = (g.fuse_length==0.0F)?0.0F:(players[local_player_id].orientation.x+players[local_player_id].physics.velocity.x);
					g.vy = (g.fuse_length==0.0F)?0.0F:(players[local_player_id].orientation.z+players[local_player_id].physics.velocity.z);
					g.vz = (g.fuse_length==0.0F)?0.0F:(-players[local_player_id].orientation.y-players[local_player_id].physics.velocity.y);
					network_send(PACKET_GRENADE_ID,&g,sizeof(g));
					read_PacketGrenade(&g,sizeof(g)); //server won't loop packet back
					players[local_player_id].item_showup = window_time();
				}
				if(action==WINDOW_PRESS) {
					sound_create(NULL,SOUND_LOCAL,&sound_grenade_pin,0.0F,0.0F,0.0F)->stick_to_player = local_player_id;
				}
			}
		}
	}
	if(button==WINDOW_MOUSE_LMB && action==WINDOW_PRESS) {
		players[local_player_id].input.buttons.lmb_start = window_time();

		if(camera_mode==CAMERAMODE_FPS) {
			if(players[local_player_id].held_item==TOOL_GUN) {
				if(weapon_reloading()) {
					weapon_reload_abort();
				}
				if(local_player_ammo==0 && window_time()-players[local_player_id].item_showup>=0.5F) {
					sound_create(NULL,SOUND_LOCAL,&sound_empty,0.0F,0.0F,0.0F);
				}
			}
		}

		if(camera_mode==CAMERAMODE_BODYVIEW || camera_mode==CAMERAMODE_SPECTATOR) {
			if(camera_mode==CAMERAMODE_SPECTATOR)
				cameracontroller_bodyview_mode = 1;
			for(int k=0;k<PLAYERS_MAX*2;k++) {
				cameracontroller_bodyview_player = (cameracontroller_bodyview_player-1)%PLAYERS_MAX;
				if(cameracontroller_bodyview_player<0)
					cameracontroller_bodyview_player = PLAYERS_MAX-1;
				if(player_can_spectate(&players[cameracontroller_bodyview_player]))
					break;
			}
			cameracontroller_bodyview_zoom = 0.0F;
		}
	}
}

struct autocomplete_type {
	const char* str;
	int acceptance;
};

static int autocomplete_type_cmp(const void* a, const void* b) {
	struct autocomplete_type* aa = (struct autocomplete_type*)a;
	struct autocomplete_type* bb = (struct autocomplete_type*)b;
	return bb->acceptance-aa->acceptance;
}

static const char* hud_ingame_completeword(const char* s) {
	//find most likely player name or command

	struct autocomplete_type candidates[PLAYERS_MAX*2+64] = {0};
	int candidates_cnt = 0;

	for(int k=0;k<PLAYERS_MAX;k++) {
		if(players[k].connected) {
			candidates[candidates_cnt++] = (struct autocomplete_type) {players[k].name,0};
			char nmbr[8];
			sprintf(nmbr,"#%i",k);
			candidates[candidates_cnt++] = (struct autocomplete_type) {nmbr,0};
		}
	}

	candidates[candidates_cnt++] = (struct autocomplete_type) {gamestate.team_1.name,0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {gamestate.team_2.name,0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/help",0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/medkit",0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/squad",0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/votekick",0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/login",0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/airstrike",0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/streak",0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/ratio",0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/intel",0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/time",0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/admin",0};
	candidates[candidates_cnt++] = (struct autocomplete_type) {"/ping",0};

	//valuate all strings
	for(int k=0;k<candidates_cnt;k++) {
		for(int i=0;i<strlen(candidates[k].str) && i<strlen(s);i++) {
			if(candidates[k].str[i]==s[i])
				candidates[k].acceptance += 2;
			else
				if(tolower(candidates[k].str[i])==tolower(s[i]) || s[i]=='*') {
					candidates[k].acceptance++;
				} else {
					candidates[k].acceptance = 0;
					break;
				}
		}
	}

	qsort(candidates,candidates_cnt,sizeof(struct autocomplete_type),autocomplete_type_cmp);
	return (strlen(candidates[0].str)>0 && candidates[0].acceptance>0)?candidates[0].str:NULL;
}

static void hud_ingame_keyboard(int key, int action, int mods, int internal) {
	if(chat_input_mode!=CHAT_NO_INPUT && action==WINDOW_PRESS && key==WINDOW_KEY_TAB && strlen(chat[0][0])>0) {
		//autocomplete word
		char* incomplete = strrchr(chat[0][0],' ')+1;
		if(incomplete==(char*)1)
			incomplete = chat[0][0];
		const char* match = hud_ingame_completeword(incomplete);
		if(match && strlen(match)+strlen(chat[0][0])<128)
			strcpy(incomplete,match);
	}

	if(chat_input_mode==CHAT_NO_INPUT) {
		if(action==WINDOW_PRESS) {
			if(!network_connected) {
				if(key==WINDOW_KEY_F1) {
					camera_mode = CAMERAMODE_SELECTION;
				}
				if(key==WINDOW_KEY_F2) {
					camera_mode = CAMERAMODE_FPS;
				}
				if(key==WINDOW_KEY_F3) {
					camera_mode = CAMERAMODE_SPECTATOR;
				}
				if(key==WINDOW_KEY_F4) {
					camera_mode = CAMERAMODE_BODYVIEW;
				}
				if(key==WINDOW_KEY_SNEAK) {
					log_debug("%f,%f,%f,%f,%f",camera_x,camera_y,camera_z,camera_rot_x,camera_rot_y);
					players[local_player_id].pos.x = 256.0F;
					players[local_player_id].pos.y = 63.0F;
					players[local_player_id].pos.z = 256.0F;
				}
				if(key==WINDOW_KEY_CROUCH) {
					//particle_create(0xFF00FF,players[local_player_id].pos.x,players[local_player_id].pos.y,players[local_player_id].pos.z,2.0F,1.0F,1024,0.1F,0.25F);
					players[local_player_id].alive = !players[local_player_id].alive;
				}
			}

			if(key==WINDOW_KEY_LASTTOOL) {
				int tmp = players[local_player_id].held_item;
				players[local_player_id].held_item = local_player_lasttool;
				local_player_lasttool = tmp;
				players[local_player_id].item_disabled = window_time();
				players[local_player_id].items_show_start = window_time();
				players[local_player_id].items_show = 1;
			}

			if(key==WINDOW_KEY_VOLUME_UP) {
				settings.volume = min(settings.volume+1,10);
			}
			if(key==WINDOW_KEY_VOLUME_DOWN) {
				settings.volume = max(settings.volume-1,0);
			}
			if(key==WINDOW_KEY_VOLUME_UP || key==WINDOW_KEY_VOLUME_DOWN) {
				sound_volume(settings.volume/10.0F);
				char volstr[64];
				sprintf(volstr,"Volume: %i",settings.volume);
				chat_add(0,0x0000FF,volstr);
			}

			if(key==WINDOW_KEY_COMMAND) {
				window_textinput(1);
				chat_input_mode = CHAT_ALL_INPUT;
				strcpy(chat[0][0],"/");
			}

			if(key==WINDOW_KEY_CHAT) {
				window_textinput(1);
				chat_input_mode = CHAT_ALL_INPUT;
				chat[0][0][0] = 0;
			}

			if(show_exit && key==WINDOW_KEY_NO) {
				show_exit = 0;
				window_mousemode(WINDOW_CURSOR_DISABLED);
			}

			if(key==WINDOW_KEY_YES) {
				if(show_exit) {
					hud_change(&hud_serverlist);
				} else {
					window_textinput(1);
					chat_input_mode = CHAT_TEAM_INPUT;
					chat[0][0][0] = 0;
				}
			}

			if((key==WINDOW_KEY_CURSOR_UP || key==WINDOW_KEY_CURSOR_DOWN || key==WINDOW_KEY_CURSOR_LEFT || key==WINDOW_KEY_CURSOR_RIGHT) && camera_mode==CAMERAMODE_FPS && players[local_player_id].held_item==TOOL_BLOCK) {
				int y;
				for(y=0;y<8;y++) {
					for(int x=0;x<8;x++) {
						if(texture_block_color(x,y)==players[local_player_id].block.packed) {
							switch(key) {
								case WINDOW_KEY_CURSOR_LEFT:
									x--;
									if(x<0)
										x = 7;
									break;
								case WINDOW_KEY_CURSOR_RIGHT:
									x++;
									if(x>7)
										x = 0;
									break;
								case WINDOW_KEY_CURSOR_UP:
									y--;
									if(y<0)
										y = 7;
									break;
								case WINDOW_KEY_CURSOR_DOWN:
									y++;
									if(y>7)
										y = 0;
									break;
							}
							players[local_player_id].block.packed = texture_block_color(x,y);
							network_updateColor();
							y = 10;
							break;
						}
					}
				}
				if(y<10) {
					players[local_player_id].block.packed = texture_block_color(3,0);
					network_updateColor();
				}
			}

			if(key==WINDOW_KEY_RELOAD && camera_mode==CAMERAMODE_FPS && players[local_player_id].held_item==TOOL_GUN) {
				weapon_reload();
			}

			if(key==WINDOW_KEY_SNEAK && (camera_mode==CAMERAMODE_BODYVIEW || camera_mode==CAMERAMODE_SPECTATOR)) {
				cameracontroller_bodyview_mode = !cameracontroller_bodyview_mode;
			}

			if(screen_current==SCREEN_NONE && camera_mode==CAMERAMODE_FPS) {
				unsigned char tool_switch = 0;
				switch(key) {
					case WINDOW_KEY_TOOL1:
						if(players[local_player_id].held_item!=TOOL_SPADE) {
							local_player_lasttool = players[local_player_id].held_item;
							players[local_player_id].held_item = TOOL_SPADE;
							tool_switch = 1;
						}
						break;
					case WINDOW_KEY_TOOL2:
						if(players[local_player_id].held_item!=TOOL_BLOCK) {
							local_player_lasttool = players[local_player_id].held_item;
							players[local_player_id].held_item = TOOL_BLOCK;
							tool_switch = 1;
						}
						break;
					case WINDOW_KEY_TOOL3:
						if(players[local_player_id].held_item!=TOOL_GUN) {
							local_player_lasttool = players[local_player_id].held_item;
							players[local_player_id].held_item = TOOL_GUN;
							tool_switch = 1;
						}
						break;
					case WINDOW_KEY_TOOL4:
						if(players[local_player_id].held_item!=TOOL_GRENADE) {
							local_player_lasttool = players[local_player_id].held_item;
							players[local_player_id].held_item = TOOL_GRENADE;
							tool_switch = 1;
						}
						break;
				}

				if(tool_switch) {
					sound_create(NULL,SOUND_LOCAL,&sound_switch,0.0F,0.0F,0.0F)->stick_to_player = local_player_id;
					players[local_player_id].item_disabled = window_time();
					players[local_player_id].items_show_start = window_time();
					players[local_player_id].items_show = 1;
				}
			}

			if(screen_current==SCREEN_NONE) {
				if(key==WINDOW_KEY_CHANGETEAM) {
					screen_current = SCREEN_TEAM_SELECT;
					return;
				}
			}

			if(screen_current==SCREEN_TEAM_SELECT) {
				int new_team = 256;
				switch(key) {
					case WINDOW_KEY_TOOL1:
						new_team = TEAM_1;
						break;
					case WINDOW_KEY_TOOL2:
						new_team = TEAM_2;
						break;
					case WINDOW_KEY_TOOL3:
						new_team = TEAM_SPECTATOR;
						break;
				}
				if(new_team<=255) {
					if(network_logged_in) {
						struct PacketChangeTeam p;
						p.player_id = local_player_id;
						p.team = new_team;
						network_send(PACKET_CHANGETEAM_ID,&p,sizeof(p));
						screen_current = SCREEN_NONE;
						return;
					} else {
						struct PacketExistingPlayer login;
						login.player_id = local_player_id;
						login.team = local_player_newteam;
						login.weapon = WEAPON_RIFLE;
						login.held_item = TOOL_GUN;
						login.kills = 0;
						login.blue = players[local_player_id].block.blue;
						login.green = players[local_player_id].block.green;
						login.red = players[local_player_id].block.red;
						strcpy(login.name,settings.name);
						network_send(PACKET_EXISTINGPLAYER_ID,&login,sizeof(login)-sizeof(login.name)+strlen(settings.name)+1);
						screen_current = SCREEN_NONE;
						return;
					}
				}
				if((key==WINDOW_KEY_CHANGETEAM || key==WINDOW_KEY_ESCAPE) && (!network_connected || (network_connected && network_logged_in))) {
					screen_current = SCREEN_NONE;
					return;
				}
			}

			if(key==WINDOW_KEY_ESCAPE) {
				show_exit ^= 1;
				window_mousemode(show_exit?WINDOW_CURSOR_ENABLED:WINDOW_CURSOR_DISABLED);
				return;
			}

			if(key==WINDOW_KEY_PICKCOLOR && players[local_player_id].held_item==TOOL_BLOCK) {
				players[local_player_id].item_disabled = window_time();
				players[local_player_id].items_show_start = window_time();
				players[local_player_id].items_show = 1;

				struct Camera_HitType hit;
				camera_hit_fromplayer(&hit,local_player_id,128.0F);

				switch(hit.type) {
					case CAMERA_HITTYPE_BLOCK:
						players[local_player_id].block.packed = map_get(hit.x,hit.y,hit.z);
						float dmg = (100.0F-map_damage_get(hit.x,hit.y,hit.z))/100.0F*0.75F+0.25F;
						players[local_player_id].block.red *= dmg;
						players[local_player_id].block.green *= dmg;
						players[local_player_id].block.blue *= dmg;
						break;
					case CAMERA_HITTYPE_PLAYER:
						players[local_player_id].block.packed = players[hit.player_id].block.packed;
						break;
					case CAMERA_HITTYPE_NONE:
					default:
						players[local_player_id].block.red = fog_color[0]*255;
						players[local_player_id].block.green = fog_color[1]*255;
						players[local_player_id].block.blue = fog_color[2]*255;
						break;
				}
				network_updateColor();
			}
		}
	} else {
		if(action!=WINDOW_RELEASE) {
			if(key==WINDOW_KEY_V && mods) {
				const char* clipboard = window_clipboard();
				if(clipboard)
					strcpy(chat[0][0]+strlen(chat[0][0]),clipboard);
			}
			if(key==WINDOW_KEY_ESCAPE || key==WINDOW_KEY_ENTER) {
				if(key==WINDOW_KEY_ENTER && strlen(chat[0][0])>0) {
					struct PacketChatMessage msg;
					msg.player_id = local_player_id;
					msg.chat_type = (chat_input_mode==CHAT_ALL_INPUT)?CHAT_ALL:CHAT_TEAM;
					strcpy(msg.message,chat[0][0]);
					network_send(PACKET_CHATMESSAGE_ID,&msg,sizeof(msg)-sizeof(msg.message)+strlen(chat[0][0])+1);
				}
				window_textinput(0);
				chat_input_mode = CHAT_NO_INPUT;
			}
			if(key==WINDOW_KEY_BACKSPACE) {
				size_t text_len = strlen(chat[0][0]);
				if (text_len > 0){
					chat[0][0][text_len-1] = 0;
				}
			}
		}
	}
}

struct hud hud_ingame = {
	hud_ingame_init,
	hud_ingame_render3D,
	hud_ingame_render,
	hud_ingame_keyboard,
	hud_ingame_mouselocation,
	hud_ingame_mouseclick,
	hud_ingame_scroll,
	1,
	0
};

/* HUD_SERVERLIST START */

static http_t* request_version = NULL;
static int serverlist_is_outdated;

static void hud_serverlist_init() {
	ping_stop();
	network_disconnect();
	window_title(NULL);

	window_mousemode(WINDOW_CURSOR_ENABLED);

	request_version = http_get("http://aos.party/bs/version/",NULL);

	chat_input_mode = CHAT_ALL_INPUT;
	chat[0][0][0] = 0;
	serverlist_is_outdated = 0;

	window_textinput(1);
}

static int render_tooltip(char* str, float x, float y, float scaley) {
	if(*str) {
		glColor3f(1.0F,1.0F,1.0F);
		texture_draw_empty(x+5*scaley,settings.window_height-y+5*scaley,font_length(16*scaley,str)+10*scaley,26*scaley);
		glColor3f(0.0F,0.0F,0.0F);
		font_render(x+10*scaley,settings.window_height-y,16*scaley,str);
	}
}

static void hud_serverlist_render(float scalex, float scaley) {
	glColor3f(1.0F,1.0F,1.0F);
	texture_draw_rotated(&texture_ui_bg,settings.window_width/2.0F,settings.window_height/2.0F,settings.window_width,settings.window_height,0.0F);

	glColor4f(0.0F,0.0F,0.0F,0.66F);
	glEnable(GL_BLEND);
	texture_draw_empty((settings.window_width-640*scaley)/2.0F,320*scaley,120*scaley,240*scaley);
	texture_draw_empty((settings.window_width-640*scaley)/2.0F+131*scaley,320*scaley,509*scaley,240*scaley);
	glDisable(GL_BLEND);

	glColor3f(1.0F,1.0F,0.0F);
	font_render((settings.window_width-629*scaley)/2.0F+0*scaley,309*scaley,8*scaley,"Connect");
	glColor3f(0.5F,0.5F,0.5F);
	font_render((settings.window_width-629*scaley)/2.0F+0*scaley,309*scaley-22*scaley,8*scaley,"Settings");
	font_render((settings.window_width-629*scaley)/2.0F+0*scaley,309*scaley-44*scaley,8*scaley,"Controls");

	glColor3f(1.0F,1.0F,1.0F);
	font_render((settings.window_width-629*scaley)/2.0F+131*scaley,309*scaley,8*scaley,"Server address:");
	glColor3f(0.5F,0.5F,0.5F);
	font_render((settings.window_width-629*scaley)/2.0F+131*scaley,309*scaley-11*scaley,8*scaley,"Type an AOS identifier (with port number, if specified) and press ENTER.");
	font_render((settings.window_width-629*scaley)/2.0F+131*scaley,309*scaley-22*scaley,8*scaley,"Example: aos://16777343:32887");

	glColor3f(1.0F,1.0F,1.0F);
	texture_draw_sector(&texture_ui_input,settings.window_width/2.0F-300*scaley,485*scaley,8*scaley,32*scaley,0.0F,0.0F,0.25F,1.0F);
	texture_draw_sector(&texture_ui_input,settings.window_width/2.0F-300*scaley+8*scaley,485*scaley,(400-32)*scaley,32*scaley,0.25F,0.0F,0.5F,1.0F);
	texture_draw_sector(&texture_ui_input,settings.window_width/2.0F-300*scaley+(400-32+8)*scaley,485*scaley,8*scaley,32*scaley,0.75F,0.0F,0.25F,1.0F);

	texture_draw(&texture_ui_join,settings.window_width/2.0F+90*scaley,485*scaley,32*scaley,32*scaley);

	int a = strlen(chat[0][0]);
	chat[0][0][a] = '_';
	chat[0][0][a+1] = 0;
	font_render((settings.window_width-600*scaley)/2.0F+font_length(8*scaley," "),481*scaley,8*scaley,chat[0][0]);
	chat[0][0][a] = 0;

	serverlist_is_outdated = 0;
	if(serverlist_is_outdated) {
		glColor4f(0.0F,0.0F,0.0F,0.9F);
		glEnable(GL_BLEND);
		texture_draw_empty((settings.window_width-350*scaley)/2.0F,(settings.window_height-200*scaley)/2.0F+200*scaley,350*scaley,200*scaley);
		glDisable(GL_BLEND);
		glColor3f(1.0F,1.0F,0.0F);
		font_centered(settings.window_width/2.0F,(settings.window_height-200*scaley)/2.0F+150*scaley,8*scaley,"NEW CLIENT VERSION AVAILABLE!");
		glColor3f(1.0F,0.0F,0.0F);
		font_centered(settings.window_width/2.0F,(settings.window_height-200*scaley)/2.0F+(100-8)*scaley,8*scaley,"Your game client is outdated and");
		font_centered(settings.window_width/2.0F,(settings.window_height-200*scaley)/2.0F+(100-8-8)*scaley,8*scaley,"should be updated immediately.");
		glColor3f(1.0F,1.0F,1.0F);
		font_centered(settings.window_width/2.0F,(settings.window_height-200*scaley)/2.0F+(100-8-24)*scaley,8*scaley,"Head over to https://github.com/");
		font_centered(settings.window_width/2.0F,(settings.window_height-200*scaley)/2.0F+(100-8-32)*scaley,8*scaley,"or click to open a webpage.");
	}

	if(window_time()-chat_popup_timer<chat_popup_duration) {
		float fade = 1.0F-(window_time()-chat_popup_timer)/chat_popup_duration;
		glColor4f(1.0F,0.0F,0.0F,(fade>0.25F)?0.9F:fade/0.25F*0.9F);
		glEnable(GL_BLEND);
		texture_draw_empty((settings.window_width-350*scaley)/2.0F,(settings.window_height-100*scaley)/2.0F+100*scaley,350*scaley,100*scaley);
		glDisable(GL_BLEND);
		glColor4f(1.0F,1.0F,1.0F,(fade>0.25F)?1.0F:fade/0.25F);
		char reason_str[32];
		sprintf(reason_str,"Reason: %s.",chat_popup);
		font_centered(settings.window_width/2.0F,(settings.window_height-100*scaley)/2.0F+80*scaley,8*scaley,"Disconnected from server");
		font_centered(settings.window_width/2.0F,(settings.window_height-100*scaley)/2.0F+(80-40)*scaley,8*scaley,reason_str);
	}

	if(request_version) {
		switch(http_process(request_version)) {
			case HTTP_STATUS_COMPLETED:
				serverlist_is_outdated = 1;
				log_info("Newest game version: %s",request_version->response_data);
				log_info("Current game version: %s",FOURSPADES_VERSION);
				serverlist_is_outdated = strcmp(request_version->response_data,FOURSPADES_VERSION)!=0;
				http_release(request_version);
				request_version = NULL;
				break;
			case HTTP_STATUS_FAILED:
				http_release(request_version);
				request_version = NULL;
				break;
		}
	}

	double x,y;
	window_mouseloc(&x,&y);
	if(is_inside(x,settings.window_height-y,settings.window_width/2.0F+90*scaley,(485-32)*scaley,32*scaley,32*scaley))
		render_tooltip("Join address",x,y,scaley);
}

static void server_c(char* address, char* name) {
	if(file_exists(address)) {
		map_vxl_load(file_load(address),map_colors);
		chunk_rebuild_all();
		camera_mode = CAMERAMODE_FPS;
		players[local_player_id].pos.x = map_size_x/2.0F;
		players[local_player_id].pos.y = map_size_y-1.0F;
		players[local_player_id].pos.z = map_size_z/2.0F;
		window_title(address);
		hud_change(&hud_ingame);
	} else {
		window_title(name);
		hud_change(network_connect_string(address)?&hud_ingame:&hud_serverlist);
	}
}

static void hud_serverlist_mouseclick(double x, double y, int button, int action, int mods) {
	float scaley = settings.window_height/600.0F;

	if(action==WINDOW_PRESS) {
		if(serverlist_is_outdated) {
			serverlist_is_outdated = 0;
			file_url("https://aos.party/bs");
			return;
		}

		if(x>=(settings.window_width-600*scaley)/2.0F+250*scaley-font_length(20*scaley,"Settings")/2
		&& x<(settings.window_width-600*scaley)/2.0F+250*scaley+font_length(20*scaley,"Settings")/2
		&& y>=77*scaley && y<97*scaley) {
			hud_change(&hud_settings);
		}

		if(x>=(settings.window_width-600*scaley)/2.0F+340*scaley-font_length(20*scaley,"Controls")/2
		&& x<(settings.window_width-600*scaley)/2.0F+340*scaley+font_length(20*scaley,"Controls")/2
		&& y>=77*scaley && y<97*scaley) {
			hud_change(&hud_controls);
		}

		//texture_draw(&texture_ui_join,settings.window_width/2.0F+90*scaley,485*scaley,32*scaley,32*scaley);
		if(is_inside(x,settings.window_height-y,settings.window_width/2.0F+90*scaley,(485-32)*scaley,32*scaley,32*scaley)
		&& strlen(chat[0][0])>0)
			server_c(chat[0][0],NULL);
			
			
		if(is_inside(x,settings.window_height-y,settings.window_width/2.0F+122*scaley,(485-32)*scaley,32*scaley,32*scaley))
			server_c("aos://16777343",NULL);

		if(is_inside(x,settings.window_height-y,settings.window_width/2.0F+130*scaley,(485-32)*scaley,32*scaley,32*scaley))
			hud_change(&hud_serverlist);
	}
}

static void hud_serverlist_keyboard(int key, int action, int mods, int internal) {
	if(action!=WINDOW_RELEASE) {
		if(key==WINDOW_KEY_BACKSPACE) {
			size_t text_len = strlen(chat[0][0]);
			if(text_len>0)
				chat[0][0][text_len-1] = 0;
		}
		if(key==WINDOW_KEY_ENTER && strlen(chat[0][0])>0)
			server_c(chat[0][0],NULL);
	}
}

struct hud hud_serverlist = {
	hud_serverlist_init,
	(void*)NULL,
	hud_serverlist_render,
	hud_serverlist_keyboard,
	(void*)NULL,
	hud_serverlist_mouseclick,
	(void*)NULL,
	0,
	0
};

/* HUD_SETTINGS START */

static struct config_setting* hud_settings_edit = NULL;

static void hud_settings_init() {
	memcpy(&settings_tmp,&settings,sizeof(struct RENDER_OPTIONS));
	chat_input_mode = CHAT_ALL_INPUT;
	chat[0][0][0] = 0;
}

static void hud_settings_render(float scalex, float scaley) {
	glColor3f(1.0F,1.0F,1.0F);
	texture_draw_rotated(&texture_ui_bg,settings.window_width/2.0F,settings.window_height/2.0F,settings.window_width,settings.window_height,0.0F);

	glColor4f(0.0F,0.0F,0.0F,0.66F);
	glEnable(GL_BLEND);
	texture_draw_empty((settings.window_width-640*scaley)/2.0F,550*scaley,640*scaley,600*scaley);
	glDisable(GL_BLEND);

	glColor3f(1.0F,1.0F,0.0F);
	font_render((settings.window_width-600*scaley)/2.0F+0*scaley,535*scaley,36*scaley,"Settings");
	font_centered((settings.window_width-600*scaley)/2.0F+174*scaley,70*scaley,30*scaley,"Apply");
	glColor3f(0.5F,0.5F,0.5F);
	font_centered((settings.window_width-600*scaley)/2.0F+210*scaley,535*scaley-12*scaley,20*scaley,"Controls");
	font_centered((settings.window_width-600*scaley)/2.0F+320*scaley,535*scaley-12*scaley,20*scaley,"Server list");

	for(int k=0;k<list_size(&config_settings);k++) {
		struct config_setting* a = list_get(&config_settings,k);

		float row_offset = (k+1)/2;
		float col_offset = 0.0F;
		if(k>0)
			col_offset = ((k+1)&1)?314*scaley:0;

		glColor3f(1.0F,1.0F,1.0F);
		font_render((settings.window_width-600*scaley)/2.0F+col_offset,(460-40*row_offset)*scaley,16*scaley,a->name);

		char tmp[32];
		switch(a->type) {
			case CONFIG_TYPE_STRING:
				glColor3f(1.0F,1.0F,1.0F);
				texture_draw_sector(&texture_ui_input,(settings.window_width-600*scaley)/2.0F+154*scaley+col_offset,(466-40*row_offset)*scaley,8*scaley,32*scaley,0.0F,0.0F,0.25F,1.0F);
				texture_draw_sector(&texture_ui_input,(settings.window_width-600*scaley)/2.0F+162*scaley+col_offset,(466-40*row_offset)*scaley,200*scaley,32*scaley,0.25F,0.0F,0.5F,1.0F);
				texture_draw_sector(&texture_ui_input,settings.window_width/2.0F-300*scaley+362*scaley+col_offset,(466-40*row_offset)*scaley,8*scaley,32*scaley,0.75F,0.0F,0.25F,1.0F);
				if(hud_settings_edit==a)
					glColor3f(1.0F,0.0F,0.0F);
				font_render((settings.window_width-600*scaley)/2.0F+162*scaley+col_offset,(460-40*row_offset)*scaley,20*scaley,(hud_settings_edit==a)?chat[0][0]:a->value);
				break;
			case CONFIG_TYPE_INT:
				glColor3f(1.0F,1.0F,1.0F);
				if(hud_settings_edit==a)
					strcpy(tmp,chat[0][0]);
				else
					sprintf(tmp,"%i",*(int*)a->value);
				if(a->max==1) {
					texture_draw_rotated(*(int*)a->value?&texture_ui_box_check:&texture_ui_box_empty,
						(settings.window_width-600*scaley)/2.0F+220*scaley+col_offset,(450-40*row_offset)*scaley,32*scaley,32*scaley,0.0F);
				} else {
					texture_draw_rotated(&texture_ui_arrow,(settings.window_width-600*scaley)/2.0F+270*scaley+col_offset,(450-40*row_offset)*scaley,32*scaley,32*scaley,0.0F);
					texture_draw_rotated(&texture_ui_arrow,(settings.window_width-600*scaley)/2.0F+170*scaley+col_offset,(450-40*row_offset)*scaley,32*scaley,32*scaley,PI);
					if(hud_settings_edit==a)
						glColor3f(1.0F,0.0F,0.0F);
					font_centered((settings.window_width-600*scaley)/2.0F+220*scaley+col_offset,(460-40*row_offset)*scaley,20*scaley,tmp);
				}
				break;
			case CONFIG_TYPE_FLOAT:
				glColor3f(1.0F,1.0F,1.0F);
				texture_draw_rotated(&texture_ui_arrow,(settings.window_width-600*scaley)/2.0F+270*scaley+col_offset,(450-40*row_offset)*scaley,32*scaley,32*scaley,0.0F);
				texture_draw_rotated(&texture_ui_arrow,(settings.window_width-600*scaley)/2.0F+170*scaley+col_offset,(450-40*row_offset)*scaley,32*scaley,32*scaley,PI);
				if(hud_settings_edit==a) {
					glColor3f(1.0F,0.0F,0.0F);
					strcpy(tmp,chat[0][0]);
				} else {
					sprintf(tmp,"%0.2f",*(float*)a->value);
				}
				font_centered((settings.window_width-600*scaley)/2.0F+220*scaley+col_offset,(460-40*row_offset)*scaley,20*scaley,tmp);
				break;
		}
	}

	//now find the correct help str and draw it above everything else
	double x,y;
	window_mouseloc(&x,&y);
	for(int k=0;k<list_size(&config_settings);k++) {
		struct config_setting* a = list_get(&config_settings,k);

		float row_offset = (k+1)/2;
		float col_offset = 0.0F;
		if(k>0)
			col_offset = ((k+1)&1)?314*scaley:0;

		if(is_inside(x,settings.window_height-y,(settings.window_width-600*scaley)/2.0F+col_offset,(435-40*row_offset)*scaley,300*scaley,32*scaley)) {
			render_tooltip(a->help,x,y,scaley);
			break;
		}
	}
}

static int is_int(const char* x) {
	while(*x) {
		if(!(*x>='0' && *x<='9'))
			return 0;
		x++;
	}
	return 1;
}

static int is_float(char* x) {
	char* decimalpoint = strchr(x,'.');
	if(!decimalpoint) {
		return is_int(x);
	} else {
		*decimalpoint = 0;
		if(is_int(x)) {
			if(is_int(decimalpoint+1)) {
				*decimalpoint = '.';
				return 1;
			} else {
				return 0;
			}
		} else {
			return 0;
		}
	}
}

static void hud_settings_keyboard(int key, int action, int mods, int internal) {
	if(hud_settings_edit && action!=WINDOW_RELEASE && key==WINDOW_KEY_BACKSPACE) {
		size_t text_len = strlen(chat[0][0]);
		if(text_len>0) {
			chat[0][0][text_len-1] = 0;
		}
	}
}

static void hud_settings_mouseclick(double x, double y, int button, int action, int mods) {
	if(action==WINDOW_PRESS) {
		if(hud_settings_edit) {
			if(strlen(chat[0][0])) {
				switch(hud_settings_edit->type) {
					case CONFIG_TYPE_INT:
						if(is_int(chat[0][0]))
							*(int*)hud_settings_edit->value = max(min(strtol(chat[0][0],NULL,10),hud_settings_edit->max),0);
						break;
					case CONFIG_TYPE_FLOAT:
						if(is_float(chat[0][0]))
							*(float*)hud_settings_edit->value = max(min(strtod(chat[0][0],NULL),hud_settings_edit->max),0);
						break;
					case CONFIG_TYPE_STRING:
						strncpy(hud_settings_edit->value,chat[0][0],hud_settings_edit->max);
						break;
				}
			}
			window_textinput(0);
			hud_settings_edit = NULL;
		}

		float scaley = settings.window_height/600.0F;

		if(x>=(settings.window_width-600*scaley)/2.0F+320*scaley-font_length(20*scaley,"Server list")/2
		&& x<(settings.window_width-600*scaley)/2.0F+320*scaley+font_length(20*scaley,"Server list")/2
		&& y>=77*scaley && y<97*scaley) {
			hud_change(&hud_serverlist);
		}

		if(x>=(settings.window_width-600*scaley)/2.0F+210*scaley-font_length(20*scaley,"Controls")/2
		&& x<(settings.window_width-600*scaley)/2.0F+210*scaley+font_length(20*scaley,"Controls")/2
		&& y>=77*scaley && y<97*scaley) {
			hud_change(&hud_controls);
		}

		y = settings.window_height-y;

		if(x>=(settings.window_width-600*scaley)/2.0F+174*scaley-font_length(30*scaley,"Apply")/2
		&& x<(settings.window_width-600*scaley)/2.0F+174*scaley+font_length(30*scaley,"Apply")/2
		&& y>=40*scaley && y<70*scaley) {
			memcpy(&settings,&settings_tmp,sizeof(struct RENDER_OPTIONS));
			window_fromsettings();
			sound_volume(settings.volume/10.0F);
			config_save();
		}

		for(int k=0;k<list_size(&config_settings);k++) {
			struct config_setting* a = list_get(&config_settings,k);

			float row_offset = (k+1)/2;
			float col_offset = 0.0F;
			if(k>0)
				col_offset = ((k+1)&1)?314*scaley:0;

			if(is_inside_centered(x,y,
				(settings.window_width-600*scaley)/2.0F+220*scaley+col_offset,
				(450-40*row_offset)*scaley,68*scaley,32*scaley)
			&& !(a->type==CONFIG_TYPE_INT && a->max==1)) {
					hud_settings_edit = a;
					switch(a->type) {
						case CONFIG_TYPE_INT:
							sprintf(chat[0][0],"%i",*(int*)a->value);
							break;
						case CONFIG_TYPE_FLOAT:
							sprintf(chat[0][0],"%0.2f",*(float*)a->value);
							break;
						case CONFIG_TYPE_STRING:
							strcpy(chat[0][0],a->value);
							break;
					}
					window_textinput(1);
			}
			switch(a->type) {
				case CONFIG_TYPE_INT:
					if(a->max==1) {
						if(is_inside_centered(x,y,
							(settings.window_width-600*scaley)/2.0F+220*scaley+col_offset,
							(450-40*row_offset)*scaley,32*scaley,32*scaley)) { //checkbox pressed
							*(int*)a->value = !*(int*)a->value;
						}
					} else {
						if(is_inside_centered(x,y,
							(settings.window_width-600*scaley)/2.0F+270*scaley+col_offset,
							(450-40*row_offset)*scaley,32*scaley,32*scaley)) { //right arrow pressed
							int next = a->defaults_length?(*(int*)a->value):((*(int*)a->value)+1);
							int diff = INT_MAX;
							for(int l=0;l<a->defaults_length;l++) {
								if(a->defaults[l]>*(int*)a->value && a->defaults[l]-*(int*)a->value<diff) {
									diff = a->defaults[l]-*(int*)a->value;
									next = a->defaults[l];
								}
							}
							*(int*)a->value = min(next,a->max);
						}
						if(is_inside_centered(x,y,
							(settings.window_width-600*scaley)/2.0F+170*scaley+col_offset,
							(450-40*row_offset)*scaley,32*scaley,32*scaley)) { //left arrow pressed
							int next = a->defaults_length?(*(int*)a->value):((*(int*)a->value)-1);
							int diff = INT_MAX;
							for(int l=0;l<a->defaults_length;l++) {
								if(a->defaults[l]<*(int*)a->value && *(int*)a->value-a->defaults[l]<diff) {
									diff = *(int*)a->value-a->defaults[l];
									next = a->defaults[l];
								}
							}
							*(int*)a->value = max(next,0);
						}
					}
					break;
				case CONFIG_TYPE_FLOAT:
					if(is_inside_centered(x,y,
						(settings.window_width-600*scaley)/2.0F+270*scaley+col_offset,
						(450-40*row_offset)*scaley,32*scaley,32*scaley)) { //right arrow pressed
						*(float*)a->value = min((*(float*)a->value)+0.1F,a->max);
					}
					if(is_inside_centered(x,y,
						(settings.window_width-600*scaley)/2.0F+170*scaley+col_offset,
						(450-40*row_offset)*scaley,32*scaley,32*scaley)) { //left arrow pressed
						*(float*)a->value = max((*(float*)a->value)-0.1F,0);
					}
					break;
			}
		}
	}
}

struct hud hud_settings = {
	hud_settings_init,
	(void*)NULL,
	hud_settings_render,
	hud_settings_keyboard,
	(void*)NULL,
	hud_settings_mouseclick,
	(void*)NULL,
	0,
	0
};


/* HUD_CONTROLS START */

static struct config_key_pair* hud_controls_edit = NULL;

static void hud_controls_render(float scalex, float scaley) {
	glColor3f(1.0F,1.0F,1.0F);
	texture_draw_rotated(&texture_ui_bg,settings.window_width/2.0F,settings.window_height/2.0F,settings.window_width,settings.window_height,0.0F);

	glColor4f(0.0F,0.0F,0.0F,0.66F);
	glEnable(GL_BLEND);
	texture_draw_empty((settings.window_width-640*scaley)/2.0F,550*scaley,640*scaley,600*scaley);
	glDisable(GL_BLEND);

	glColor3f(1.0F,1.0F,0.0F);
	font_render((settings.window_width-600*scaley)/2.0F+0*scaley,535*scaley,36*scaley,"Controls");
	glColor3f(0.5F,0.5F,0.5F);
	font_centered((settings.window_width-600*scaley)/2.0F+225*scaley,535*scaley-12*scaley,20*scaley,"Server list");
	font_centered((settings.window_width-600*scaley)/2.0F+340*scaley,535*scaley-12*scaley,20*scaley,"Settings");

	int x_off = 0;
	int y_off = 0;
	for(int k=0;k<list_size(&config_keys);k++) {
		struct config_key_pair* a = list_get(&config_keys,k);
		if(*a->display) {
			glColor3f(1.0F,1.0F,1.0F);
			font_render((settings.window_width-600*scaley)/2.0F+200*x_off*scaley,(460-40*y_off)*scaley,16*scaley,a->display);

			texture_draw_sector(&texture_ui_input,(settings.window_width-600*scaley)/2.0F+(200*x_off+110)*scaley,(466-40*y_off)*scaley,8*scaley,32*scaley,0.0F,0.0F,0.25F,1.0F);
			texture_draw_sector(&texture_ui_input,(settings.window_width-600*scaley)/2.0F+(200*x_off+118)*scaley,(466-40*y_off)*scaley,64*scaley,32*scaley,0.25F,0.0F,0.5F,1.0F);
			texture_draw_sector(&texture_ui_input,(settings.window_width-600*scaley)/2.0F+(200*x_off+182)*scaley,(466-40*y_off)*scaley,8*scaley,32*scaley,0.75F,0.0F,0.25F,1.0F);

			if(hud_controls_edit==a)
				glColor3f(1.0F,0.0F,0.0F);
			font_centered((settings.window_width-600*scaley)/2.0F+(200*x_off+150)*scaley,(460-40*y_off)*scaley,20*scaley,window_keyname(a->def));

			y_off++;
			if(y_off>10) {
				y_off = 0;
				x_off++;
			}
		}
	}
}

static void hud_controls_mouseclick(double x, double y, int button, int action, int mods) {
	if(action==WINDOW_PRESS) {
		if(hud_controls_edit)
			hud_controls_edit = NULL;

		float scaley = settings.window_height/600.0F;

		if(x>=(settings.window_width-600*scaley)/2.0F+225*scaley-font_length(20*scaley,"Server list")/2
		&& x<(settings.window_width-600*scaley)/2.0F+225*scaley+font_length(20*scaley,"Server list")/2
		&& y>=77*scaley && y<97*scaley) {
			hud_change(&hud_serverlist);
		}

		if(x>=(settings.window_width-600*scaley)/2.0F+340*scaley-font_length(20*scaley,"Settings")/2
		&& x<(settings.window_width-600*scaley)/2.0F+340*scaley+font_length(20*scaley,"Settings")/2
		&& y>=77*scaley && y<97*scaley) {
			hud_change(&hud_settings);
		}

		y = settings.window_height-y;

		int x_off = 0;
		int y_off = 0;
		for(int k=0;k<list_size(&config_keys);k++) {
			struct config_key_pair* a = list_get(&config_keys,k);
			if(*a->display) {

				if(is_inside(x,y,
					(settings.window_width-600*scaley)/2.0F+(200*x_off+110)*scaley,
					(434-40*y_off)*scaley,80*scaley,32*scaley)) {
					hud_controls_edit = a;
					break;
				}

				y_off++;
				if(y_off>10) {
					y_off = 0;
					x_off++;
				}
			}
		}
	}
}

static void hud_controls_keyboard(int key, int action, int mods, int internal) {
	if(hud_controls_edit) {
		hud_controls_edit->def = internal;
		hud_controls_edit = NULL;
		config_save();
	}
}

struct hud hud_controls = {
	(void*)NULL,
	(void*)NULL,
	hud_controls_render,
	hud_controls_keyboard,
	(void*)NULL,
	hud_controls_mouseclick,
	(void*)NULL,
	0,
	0
};
