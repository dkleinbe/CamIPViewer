/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <system_settings.h>
#include <privacy_privilege_manager.h>
#include <Elementary.h>
#include <efl_extension.h>
#include <media_content.h>
#include <player.h>
#include <app.h>
#include <media_streamer.h>
#include "utils.h"
#include "settings.h"

#include <view_audio.h>



#define MEDIA_DIRECTORY "/opt/usr/media"
#define IMG_PATH_NO_ALBUM "images/music_no_album_art.png"

static struct main_info {
	Ecore_Timer *timer_progressbar;
	player_h player;
} s_info = {
	.timer_progressbar = NULL,
	.player = NULL,
};

static void _btn_down_cb(void *user_data, Evas *e, Evas_Object *obj, void *event_info);
static void _btn_up_cb(void *user_data, Evas *e, Evas_Object *obj, void *event_info);
static Eina_Bool _progressbar_timer_cb(void *user_data);
static void _start_progressbar(void *user_data);
static void _stop_progressbar(void);
static bool _play_current_music(const char *file_path, void *user_data);
static void _change_play_button_image(void *user_data);
static void _play_btn_clicked_cb(void *user_data, Evas_Object *obj, void *event_info);
static void _player_completed_cb(void *user_data);
static void _content_back_cb(void *user_data, Evas_Object *obj, void *event_info);

static int _progress = 0;

/*
 * @brief: Get path of resource
 * @param[file_in]: File name
 * @param[file_path_out]: The point to which save full path of the resource
 * @param[file_path_max]: Size of file name include path
 */
void data_get_resource_path(const char *file_in, char *file_path_out, int file_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(file_path_out, file_path_max, "%s%s", res_path, file_in);
		free(res_path);
	}
}

/*
 * @brief: Get a image path
 * @param[part_name]: Part name to which you want to set
 */
char *data_get_image(const char *part_name)
{
	/*
	 * You can use this function to retrieve data.
	 */
	char *file_path = NULL;
	char full_path[PATH_MAX] = { 0, };

	if (!strcmp("sw.icon.vol", part_name)) {
		file_path = "images/Controller/music_top_button_icon_volume.png";
	} else if (!strcmp("sw.progressbar.bg", part_name)) {
		file_path = "images/Controller/b_music_progress_bg.png";
	} else if (!strcmp("sw.icon.play", part_name)) {
		file_path = "images/Controller/music_controller_btn_play.png";
	} else if (!strcmp("sw.icon.prev", part_name)) {
		file_path = "images/Controller/music_controller_btn_prev.png";
	} else if (!strcmp("sw.icon.next", part_name)) {
		file_path = "images/Controller/music_controller_btn_next.png";
	} else {
		EINA_LOG_ERR("failed to get image.");
		return NULL;
	}

	data_get_resource_path(file_path, full_path, (int)PATH_MAX);

	return strdup(full_path);
}

/*
 * @brief: Hook to take necessary actions before main event loop starts
 * Initialize UI resources and application's data
 * If this function returns true, the main loop of application starts
 * If this function returns false, the application is terminated
 */
bool
create_audio_view(void *user_data)
{
	Evas_Object *conform = NULL;
	Evas_Object *content = NULL;
	char *icon_path = NULL;
	char full_path[PATH_MAX] = { 0, };
	char default_img_path[PATH_MAX] = { 0, };

	//_check_storage_permission();
	/*
	 * Connects to the media content service
	 */
	if (media_content_connect() != MEDIA_CONTENT_ERROR_NONE) {
		EINA_LOG_ERR("Can not connect media content.");
		return false;
	}

	/*
	 * Creates a player handle for playing multimedia content if not already created
	 */
	if ((s_info.player == NULL) && (player_create(&s_info.player) != PLAYER_ERROR_NONE))
	{
		EINA_LOG_ERR("Can not create player handler.");
		media_content_disconnect();
		return false;
	}

	/*
	 * Create basic GUI
	 */
	if (view_get_window() != NULL)
	{
		view_raise_window();
		return true;
	}

	view_create();

	/*
	 * Get EDJ file path will be used for GUI
	 */
	data_get_resource_path("edje/audioview.edj", full_path, sizeof(full_path));

	/*
	 * Create GUI for music player
	 */
	conform = view_get_conformant();
	content = view_create_layout_for_conformant(conform, full_path, GRP_MAIN, _content_back_cb, NULL);
	if (content == NULL) {
		EINA_LOG_ERR("failed to create a content.");
		return NULL;
	}

	/*
	 * Set default album art
	 */
	data_get_resource_path(IMG_PATH_NO_ALBUM, default_img_path, sizeof(default_img_path));
	view_music_set_album_art(content, "sw.bg", "NULL", default_img_path);

	/*
	 * Set progressbar background image and color
	 */
	icon_path = data_get_image("sw.progressbar.bg");
	view_set_image(content, "sw.progressbar.bg", icon_path);
	view_set_color(content, "sw.progressbar.bg", 0, 87, 107, 255);
	free(icon_path);

	/*
	 * Set Play button and color
	 */
	icon_path = data_get_image("sw.icon.play");
	view_set_button(content, "sw.icon.play", "focus", icon_path, NULL, _btn_down_cb, _btn_up_cb, _play_btn_clicked_cb, content);
	view_set_color(content, "sw.icon.play", 250, 250, 250, 255);
	free(icon_path);

	/*
	 * Set progressbar that is used to display play time
	 */
	view_set_progressbar(content, "sw.progressbar", 53, 5);
	view_set_color_of_circle_object(content, "sw.progressbar", 0, 192, 235, 255 * 0.5);
	view_set_progressbar_val(content, "sw.progressbar", 0);


	/*
	 * Registers a callback function to be invoked when the playback is finished
	 */
	if (player_set_completed_cb(s_info.player, _player_completed_cb, content) != PLAYER_ERROR_NONE) {
		EINA_LOG_ERR("failed to register player state changed cb");
	}

	return true;
}


/*
 * @brief: The function operated on button down
 * @param[user_data]: Data to pass to function when it is called
 * @param[e]: An opaque handle to an Evas canvas
 * @param[obj]: Object that event is triggered
 * @param[event_info]: Informations regarding with event
 */
static void _btn_down_cb(void *user_data, Evas *e, Evas_Object *obj, void *event_info)
{
	evas_object_color_set(obj, 250, 250, 250, 102);
}

/*
 * @brief: The function operated on button up
 * @param[user_data]: Data to pass to function when it is called
 * @param[e]: An opaque handle to an Evas canvas
 * @param[obj]: Object that event is triggered
 * @param[event_info]: Informations regarding with event
 */
static void _btn_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	evas_object_color_set(obj, 250, 250, 250, 255);

}

/*
 * @brief: Function to update progressbar value
 * @param[user_data]: Data to pass to function when it is called
 */


static Eina_Bool _progressbar_timer_cb(void *user_data)
{
	Evas_Object *content = user_data;
	double ratio = 0;
	int dur_msec = 0, pos_msec = 0;

	// FIXME: remove this useless code
	player_get_duration(s_info.player, &dur_msec);
	player_get_play_position(s_info.player, &pos_msec);
	ratio = ((double) pos_msec / dur_msec) * 100;
	if (ratio == 100) {
		s_info.timer_progressbar = NULL;
		return ECORE_CALLBACK_CANCEL;
	}
	// END FIX

	_progress = _progress % 100;
	view_set_progressbar_val(content, "sw.progressbar", _progress);

	return ECORE_CALLBACK_RENEW;
}

/*
 * @brief: Create a timer to update progressbar
 * @param[user_data]: Data to pass to function when it is called
 */
static void _start_progressbar(void *user_data)
{
	Evas_Object *content = user_data;

	_progress = 0;

	if (s_info.timer_progressbar) {
		ecore_timer_del(s_info.timer_progressbar);
		s_info.timer_progressbar = NULL;
	}
	s_info.timer_progressbar = ecore_timer_add(1.0f, _progressbar_timer_cb, content);
}

/*
 * @brief: Remove a timer
 */
static void _stop_progressbar(void)
{
	_progress = 0;
	if (s_info.timer_progressbar) {
		ecore_timer_del(s_info.timer_progressbar);
		s_info.timer_progressbar = NULL;
	}
}

/*
 * @brief: Play a music that is displayed at screen
 * @param[file_path]: File path of music
 * @param[user_data]: Data to pass to function when it is called
 */
static bool
_play_current_music(const char *file_path, void *user_data)
{
	Evas_Object *content = user_data;
	player_state_e player_state;
	char url[2048];
	int ret = 0;

	if (player_get_state(s_info.player, &player_state) != PLAYER_ERROR_NONE)
	{
		EINA_LOG_ERR("failed to get player state");
		return false;
	}

	if (player_state != PLAYER_STATE_IDLE)
	{
		if (player_unprepare(s_info.player) != PLAYER_ERROR_NONE)
		{
			EINA_LOG_ERR("unprepare error");
			return false;
		}
	}
	// http://radio666.net:8000
	// http://100radio-albi.ice.infomaniak.ch/100radio-albi-128.mp3
	// http://192.168.1.22:8080/audio.wav
	// set URL to fetch
	snprintf(url, 1023, "http://%s:%s@%s:%s/%s", get_setting(CAM_USER),
			get_setting(CAM_PASSWORD), get_setting(CAM_IP),
			get_setting(CAM_PORT), get_setting(CAM_AUDIO));

	EINA_LOG_DBG("Audio url: %s", url);
	// "http://ipcam:ipc642lccost@192.168.1.22:8080/audio.wav"
	if (player_set_uri(s_info.player, url) != PLAYER_ERROR_NONE)
	{
		EINA_LOG_ERR("set url error");
		return false;
	}

	player_set_streaming_user_agent(s_info.player,
			"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36",
			strlen(
					"Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/74.0.3729.169 Safari/537.36"));
	if ((ret = player_prepare(s_info.player)) != PLAYER_ERROR_NONE)
	{
		EINA_LOG_ERR("prepare error");
		return false;
	}

	if (player_start(s_info.player) != PLAYER_ERROR_NONE)
	{
		EINA_LOG_ERR("start error");
		return false;
	}

	_start_progressbar(content);

	return true;
}

/*
 * @brief: Change a play button icon by player state
 * @param[user_data]: Data to pass to function when it is called
 */
static void _change_play_button_image(void *user_data)
{
	Evas_Object *content = user_data;
	Evas_Object *btn = NULL;
	char *icon_path = NULL;
	char full_path[PATH_MAX] = { 0, };
	player_state_e player_state;

	if (player_get_state(s_info.player, &player_state) != PLAYER_ERROR_NONE) {
		EINA_LOG_ERR("failed to get player state");
		return;
	}

	switch (player_state) {
	case PLAYER_STATE_NONE:
	case PLAYER_STATE_IDLE:
	case PLAYER_STATE_READY:
	case PLAYER_STATE_PAUSED:
		icon_path = "images/Controller/music_controller_btn_play.png";
		break;
	case PLAYER_STATE_PLAYING:
		icon_path = "images/Controller/music_controller_btn_pause.png";
		break;
	default:
		break;
	}

	data_get_resource_path(icon_path, full_path, sizeof(full_path));

	btn = elm_object_part_content_get(content, "sw.icon.play");
	if (btn) {
		view_set_image(btn, "icon", full_path);
	}
}

/*
 * @brief: The function operated on play button click
 * @param[user_data]: Data to pass to function when it is called
 * @param[obj]: Object that event is triggered
 * @param[event_info]: Informations regarding with event
 */
static void _play_btn_clicked_cb(void *user_data, Evas_Object *obj,
		void *event_info)
{
	Evas_Object *content = user_data;

	char *file_path = NULL;
	player_state_e player_state;

	if (player_get_state(s_info.player, &player_state) != PLAYER_ERROR_NONE)
	{
		EINA_LOG_ERR("failed to get player state");
		return;
	}

	switch (player_state)
	{

		case PLAYER_STATE_IDLE:

			_start_progressbar(content);
			if (! _play_current_music(file_path, content))
			{
				//popup_text_1button(ad, "Can\'t get image :("));
				_stop_progressbar();

				_change_play_button_image((void *)PLAYER_STATE_NONE);

				return;
			}
			break;

		case PLAYER_STATE_PLAYING:

			if (player_stop(s_info.player) != PLAYER_ERROR_NONE)
			{
				return;
			}
			_stop_progressbar();
			break;

		case PLAYER_STATE_PAUSED:
		case PLAYER_STATE_READY:

			if (player_start(s_info.player) != PLAYER_ERROR_NONE)
			{
				return;
			}
			_start_progressbar(content);
			break;
		case PLAYER_STATE_NONE:
		default:
			break;
	}

	_change_play_button_image(content);
}



/*
 * @brief: Called when the media player is completed
 * @param[user_data]: Data passed from the callback registration function
 */
static void _player_completed_cb(void *user_data)
{
	//Evas_Object *content = user_data;

	//_next_btn_clicked_cb(content, NULL, NULL);

	return;
}

/*
 * @brief: The function operated on back key event
 * @param[user_data]: Data to pass to function when it is called
 * @param[obj]: Object that event is triggered
 * @param[event_info]: Informations regarding with event
 */
static void _content_back_cb(void *user_data, Evas_Object *obj,
		void *event_info)
{
	player_state_e player_state;

	if (player_get_state(s_info.player, &player_state) != PLAYER_ERROR_NONE)
	{
		EINA_LOG_ERR("failed to get player state");
		return;
	}


	EINA_LOG_DBG("lowers the window object");
	view_lower_window();

}


