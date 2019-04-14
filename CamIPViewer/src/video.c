/*
 * video.c
 *
 *  Created on: Apr 13, 2019
 *      Author: denis
 */



#include <net_connection.h>
#include <system_info.h>
#include <player.h>

#include "utils.h"
#include "settings.h"
#include "video.h"

static Evas_Object *
_video_rect_add(Evas_Object *parent)
{
	Evas *evas = evas_object_evas_get(parent);
	Evas_Object *image = evas_object_image_filled_add(evas);

	evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(image, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(image);

	return image;
}

static void
_prepare_cb(void *data)
{
	dlog_print(DLOG_INFO, LOG_TAG, "Media prepared!");
}
/*
 * @brief Function to create gui object
 * @param[in] ad The data structure to manage gui object
 */
void
create_video_view(appdata_s *ad)
{
	player_h player = NULL;
	Evas_Object *video_rect = NULL;
	Elm_Object_Item *nf_video = NULL;
	media_format_h mjpeg_fmt = NULL;
	int ret = 0;
	const char *res_path = get_resource_path("videos/video_360x240_1mb.mp4");

	// Create video rect
	video_rect = _video_rect_add(ad->naviframe);

	// Create video format
	media_format_create(&mjpeg_fmt);
	if (media_format_set_video_mime(mjpeg_fmt, MEDIA_FORMAT_MJPEG) != MEDIA_FORMAT_ERROR_NONE)
	{
		dlog_print(DLOG_ERROR, LOG_TAG, "media_format_set_video_mime failed!");
	}
	ret = media_format_set_video_height(mjpeg_fmt, 480);
	ret = media_format_set_video_width(mjpeg_fmt, 640);

	// Create player

	ret = player_create(&player);

	ret = player_set_media_stream_info(player, PLAYER_STREAM_TYPE_VIDEO, mjpeg_fmt);

	ret = player_set_streaming_user_agent(player, "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/73.0.3683.103 Safari/537.36", 116);


#ifdef MY_TEST
	ret = player_set_uri(player, res_path);
#else
	ret = player_set_uri(player, "http://192.168.1.22:8080/video");
#endif

	ret = player_set_display(player, PLAYER_DISPLAY_TYPE_EVAS, GET_DISPLAY(video_rect));
	switch (ret)
	{
		case PLAYER_ERROR_INVALID_PARAMETER:
			break;

		case PLAYER_ERROR_INVALID_OPERATION:
			break;

		case PLAYER_ERROR_INVALID_STATE:
			break;

		default:
			break;
	}
	ret = player_prepare(player);
	//ret = player_prepare_async(player, _prepare_cb, NULL);
	switch (ret)
	{
		case PLAYER_ERROR_INVALID_PARAMETER:
			break;
		case PLAYER_ERROR_INVALID_URI:
			break;
		case PLAYER_ERROR_NO_SUCH_FILE:
			break;
		case PLAYER_ERROR_NOT_SUPPORTED_FILE:
			break;
		case PLAYER_ERROR_INVALID_OPERATION:
			break;
		case PLAYER_ERROR_PERMISSION_DENIED:
			break;
		case PLAYER_ERROR_NOT_SUPPORTED_AUDIO_CODEC:
			break;
		case PLAYER_ERROR_NOT_SUPPORTED_VIDEO_CODEC:
			break;
		default:
			break;

	}
	ret = player_start(player);

	nf_video = elm_naviframe_item_push(ad->naviframe, NULL, NULL, NULL, video_rect, "empty");
	//elm_naviframe_item_pop_cb_set(nf_image, _image_pop_cb, NULL);
}
