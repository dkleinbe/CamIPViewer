/*
 * video.c
 *
 *  Created on: Apr 13, 2019
 *      Author: denis
 */


#include <system_info.h>
#include <player.h>
#include <efl_util.h>

#include "utils.h"
#include "settings.h"
#include "video.h"

#define STATE_FRAME 0
#define STATE_MAIN_HEADER 30
#define STATE_FRAME_HEADER 36

#define JPEG_BUFFER_SIZE 5*1024*1024



static Evas_Object *_app_naviframe;
/* FIXME: move to a structure */
static appdata_s *_appdata;
static Evas_Object *scroller;
static Evas_Object *layout;
static int screen_size_w = 0;
static int screen_size_h = 0;

static image_data_s downloaded_image;

static int g_state = STATE_MAIN_HEADER;
static int jpeg_frame_size = 0;
static int jpeg_frame_index = 0;

static int jpeg_every = 1;

static int jpeg_frame_position = 0;
static int jpeg_num_headers = 0;
static int block_counter = 0;
static unsigned char *jpeg_buffer = NULL;
static char headerline[1000] = { 0, };


static long total_bytes = 0;

/**
 * @brief get http header
 * @param buf
 */
static void _on_headerline(char *buf)
{
	// printf("HEADER: %s\n", buf);
	if (strncmp(buf, "Content-Length:", 15) == 0)
	{
		jpeg_frame_size = atoi(buf + 16);
		// printf("HEADER: Got content length = %d bytes\n", jpeg_frame_size);
	}
	if (strncmp(buf, "Content-Type:", 13) == 0)
	{
		// printf("HEADER: Got content type = %s\n", buf + 14);
	}
}
static void
_on_connection_error(appdata_s *ad)
{
	feedback_msg_s *fm = malloc(sizeof(feedback_msg_s));

	fm->ad = ad;
	fm->frame = NULL;
	fm->command = FEEDBACK_CMD_CONNECTION_ERROR;

	EINA_LOG_INFO("FEEDBACK_CMD_CONNECTION_ERROR");

	ecore_thread_feedback(ad->thread, fm);
}

/**
 * @brief Create jpeg buffer from data
 * @param[in] ptr pointer to image data
 * @param[in] len nb bytes of image
 * @param[in] ad application data
 */
static void
_on_frame(unsigned char *ptr, int len, appdata_s *ad)
{

	if (jpeg_frame_index % jpeg_every == 0)
	{
		//decode_into_current(ptr, len);
		//current_callback(bmp);
	}
	memcpy(downloaded_image.image_file_buf, ptr, len);
	downloaded_image.image_size = len;

	jpeg_frame_index++;

	feedback_msg_s *fm = malloc(sizeof(feedback_msg_s));

	fm->ad = ad;
	fm->frame = &downloaded_image;
	fm->command = FEEDBACK_CMD_NEW_FRAME;

	EINA_LOG_INFO("NEW FRAME %d", jpeg_frame_index);

	ecore_thread_feedback(ad->thread, fm);
}

/**
 * This function is called in the main thread whenever ecore_thread_feedback()
 * is called in the download thread.
 * @param data application data
 * @param thread
 * @param msg_data feedback_msg_s
 */
static void
_download_feedback_cb(void *data, Ecore_Thread *thread, void *msg_data)
{
    if (msg_data == NULL)
    {
    	EINA_LOG_ERR("msg_data is NULL");
        return;
    }

    feedback_msg_s *fm = (feedback_msg_s *)msg_data;
    appdata_s *ad = fm->ad;

    switch (fm->command)
    {
		case FEEDBACK_CMD_NONE:

			EINA_LOG_ERR("No command set");
			break;

		case FEEDBACK_CMD_NEW_FRAME:

			elm_image_memfile_set(
					ad->image_jpg,
					&fm->frame->image_file_buf,
					fm->frame->image_size,
					"jpg", NULL);
			break;

		case FEEDBACK_CMD_CONNECTION_ERROR:

			popup_text_1button(ad, "Can\'t get video :(");
			elm_naviframe_item_pop(ad->naviframe);
			break;

		default:
			break;
    }

    free(msg_data);
}
static int
_progress_callback_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
    if (!clientp)
    {
    	EINA_LOG_ERR("data is NULL");
        return 0;
    }

    EINA_LOG_DBG("Progress : dltotal: %lld dlnow: %lld ultotal: %lld, ulnow: %lld", dltotal, dlnow, ultotal, ulnow);

    appdata_s *ad = (appdata_s *)clientp;
    if (ad->cancel_requested)
    {
    	EINA_LOG_DBG("in progress_cb(): cancelling");
    	ad->cancel_requested = false;

    	return 1;

    }
    return 0;
}
/**
 * @brief Curl callback for writing data
 * @param ptr delivered data
 * @param size 1
 * @param nmemb size of that data
 * @param userdata
 * @return number of bytes consumed
 */
static size_t
_write_callback_cb(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	appdata_s *ad = userdata;

	//dlog_print(DLOG_INFO, LOG_TAG, "data read size: %d", nmemb);

    unsigned char *bptr = (unsigned char *)ptr;

    size_t nbytes = size * nmemb;

    //dlog_print(DLOG_INFO, LOG_TAG, "got %d bytes (%d, %d) (total %d)...\n", nbytes, size, nmemb, total_bytes);

    total_bytes += nbytes;

    for(int i=0; i<nbytes; i++)
    {
        unsigned char b = bptr[i];
        // printf("byte #%d.%d %02x (%c)\n", block_counter, i, b, (b > 32 && b < 128) ? b : '?');
        // printf("%c", (b > 32 && b < 128) ? b : '?');
        if (g_state == STATE_MAIN_HEADER || g_state == STATE_FRAME_HEADER)
        {
            int p = strlen(headerline);
            if (p < 1000)
                headerline[p] = b;
            if (b == '\n') {
                if (headerline[p-1] == '\r')
                {
                    headerline[p] = 0;
                    headerline[p-1] = 0;
                    // printf("got header newline after \"%s\".\n", headerline);
                    _on_headerline(headerline);
                    if (strncmp(headerline, "--", 2) == 0)
                    {
                        // printf("got boundary.\n");
                        g_state = STATE_FRAME_HEADER;
                    }
                    if (strlen(headerline) == 0 && jpeg_num_headers > 0)
                    {
                        // did we get an empty line, switch state.
                        // printf("empty header, switch state.\n");
                        if (g_state == STATE_FRAME_HEADER)
                        {
                            // printf("starting new jpeg frame.\n");
                            g_state = STATE_FRAME;
                            jpeg_frame_position = 0;
                        }
                    }
                    memset(&headerline, 0, 1000);
                    jpeg_num_headers ++;
                }
            }
        }
        else if (g_state == STATE_FRAME)
        {
            if (jpeg_frame_position < JPEG_BUFFER_SIZE)
            {
                jpeg_buffer[jpeg_frame_position] = b;
            }
            jpeg_frame_position ++;
            if (jpeg_frame_position >= jpeg_frame_size)
            {
                //printf("position %d / %d\n", jpeg_frame_position, jpeg_frame_size);
                //printf("end of frame.\n");
                _on_frame(jpeg_buffer, jpeg_frame_size, ad);
                g_state = STATE_FRAME_HEADER;
                memset(headerline, 0, 1000);
                jpeg_frame_position = 0;
                jpeg_num_headers = 0;
            }
        }
    }

    // written = fwrite(ptr, size, nmemb, stream);

    block_counter ++;
    return nbytes;
}

static void
_cleanup(appdata_s *ad)
{
    if (ad == NULL)
    {
    	EINA_LOG_ERR("cleanup(): ad is NULL");
        return;
    }

    if (ad->cleanup_done)
        return;

    if (ad->curl != NULL)
    {
        curl_easy_cleanup(ad->curl);

        if (jpeg_buffer)
        	free(jpeg_buffer);
    	jpeg_buffer = NULL;

    	EINA_LOG_DBG("curl_easy_cleanup(ad->curl)");
    } else
    	EINA_LOG_DBG("cleanup(): ad->curl is NULL");

//    if (ad->stream) {
//        // Close the local file
//        dlog_print(DLOG_DEBUG, LOG_TAG, "closing the local file");
//        fclose(ad->stream);
//    }

	connection_unset_proxy_address_changed_cb(ad->connection);
	connection_destroy(ad->connection);

    ad->cleanup_done = true;
    ad->downloading = false;
    downloaded_image.image_size = 0;


}

static
void _thread_end_cleanup(appdata_s *ad)
{
	EINA_LOG_DBG("thread_end_cleanup(): taking lock");
    eina_lock_take(&ad->mutex);
    EINA_LOG_DBG("thread_end_cleanup(): lock taken");

    _cleanup(ad);

    ad->thread_running = false;

    EINA_LOG_DBG("thread_end_cleanup(): freeing lock");
    eina_lock_release(&ad->mutex);
}


static void
_download_thread_cancel_cb(void *data, Ecore_Thread *thread)
{
	EINA_LOG_DBG("in download_thread_cancel_cb()");

    if (data == NULL) {
    	EINA_LOG_ERR("data is NULL");
        return;
    }

    appdata_s *ad = (appdata_s *)data;

    _thread_end_cleanup(ad);
}
/**
 *
 * @param data
 * @param thread
 */
static void
_download_thread_end_cb(void *data, Ecore_Thread *thread)
{
	EINA_LOG_DBG("in download_thread_end_cb()");

    if (data == NULL) {
    	EINA_LOG_ERR("data is NULL");
        return;
    }

    appdata_s *ad = (appdata_s *)data;

    _thread_end_cleanup(ad);
}

/**
 * Run curl in the downloading thread
 * @param data
 * @param thread
 */
static void
_download_thread_cb(void *data, Ecore_Thread *thread)
{
	CURL *curl;
	CURLcode curl_err;
	connection_h connection;
	char url[1024];

	appdata_s *ad = data;

	if (eina_log_main_thread_check())
		EINA_LOG_DBG("DOWNLOAD THREAD STARTED MAIN");
	else
		EINA_LOG_DBG("DOWNLOAD THREAD STARTED");
	//
	// Test net connection
	//
	if (init_net_connection(&connection) != true)
	{
		EINA_LOG_ERR("CONNECTION ERROR");
		return;
	}
	ad->connection = connection;
	downloaded_image.image_size = 0;

	// set URL to fetch
	snprintf(url, 1023, "http://%s:%s/%s", get_setting(CAM_IP), get_setting(CAM_PORT), get_setting(CAM__VIDEO));
	EINA_LOG_DBG("Video url: %s", url);
	//
	// Init curl connection
	//
	curl = init_curl_connection(connection, url, _write_callback_cb, ad, _progress_callback_cb, ad);
	ad->curl = curl;
	if (curl == NULL)
	{
		EINA_LOG_ERR("CURL INIT ERROR");
		_on_connection_error(ad);

		_cleanup(ad);
		return;
	}
	//
	// Perform blocking http streaming
	//
	ad->downloading = true;
	jpeg_buffer = (unsigned char *)malloc(JPEG_BUFFER_SIZE);

	EINA_LOG_DBG("PERFORM CURL REQUEST");
	//
	// perform curl request
	//
	curl_err = curl_easy_perform(curl);
	switch (curl_err)
	{
		case CURLE_OK:

			EINA_LOG_DBG("END CURL REQUEST");

			_cleanup(ad);

			break;

		case CURLE_ABORTED_BY_CALLBACK:

			EINA_LOG_ERR("CURLE_ABORTED_BY_CALLBACK");

			_cleanup(ad);

			break;

		case CURLE_OPERATION_TIMEDOUT:

			EINA_LOG_ERR("CURLE_OPERATION_TIMEDOUT");

			_cleanup(ad);

			break;

		default:

			EINA_LOG_ERR("CURLE_OPERATION ERROR");

			EINA_LOG_ERR("curl error: %s (%d)\n",
							curl_easy_strerror(curl_err),
							curl_err);

			_cleanup(ad);

			break;
	}

}
/**
 * Start streaming thread
 * @param data application data
 * @return
 */
static bool
_start_video_streaming(void *data)
{

	appdata_s *ad = data;
	// Start download

	if (ad->downloading)
	{
		// Cancel current download
		ad->cancel_requested = true;
		return false;
	}

    /*
     * Start a thread which will communicate with the main thread
     * by calling ecore_thread_feedback(), which will cause download_feedback_cb()
     * to be called in the main thread.
     *
     * This is necessary because the download thread needs to change the UI
     * as the download progresses, and that should be done only in the main
     * thread.
     */
    ad->thread = ecore_thread_feedback_run(
    		_download_thread_cb,
    		_download_feedback_cb,
			_download_thread_end_cb,
			_download_thread_cancel_cb,
			ad,
			EINA_FALSE);

    if (ad->thread == NULL)
    {
    	EINA_LOG_ERR("Could not create thread");
        return false;
    }
    else
        ad->thread_running = true;

	return true;

}


static Eina_Bool
_rotary_handler_cb(void *data, Eext_Rotary_Event_Info *ev)
{
	float zoom_fator = 1.0;
	Evas_Object *image = data;
	int xi;
	int yi;
	int x;
	int y;
	int image_w;
	int image_h;
	int w;
	int h;
	int current_w = 0;
	int current_h = 0;


	//elm_image_orient_set(image , orient);

	// get displayed image size
	evas_object_size_hint_min_get(image, &current_w, &current_h);
	// get visible region
	elm_scroller_region_get(scroller, &x, &y, &w, &h);

	// get image true size
	// FIXME: store this value somewhere do not get it every time
	elm_image_object_size_get(image , &image_w, &image_h);
	EINA_LOG_INFO("image size, width = %d , height = %d\n", w, h);

    if (ev->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE)
    {
    	EINA_LOG_DBG("Rotary device rotated in clockwise direction");
    	// If all ready over zoomed return
    	if (current_w >= image_w || current_h >= image_h)
    	{
    		return EINA_FALSE;
    	}
        zoom_fator = 2;
    }
    else
    {
    	EINA_LOG_DBG("Rotary device rotated in counter-clockwise direction");
    	// if image smaller then scroller region return
    	if (current_w < w || current_h < h)
    	{
    		return EINA_FALSE;
    	}
        zoom_fator = 1.0/2.0;
    }
	// compute new image size
	current_w *= zoom_fator;
	current_h *= zoom_fator;
	// set layout size to image size
	evas_object_size_hint_min_set(layout, current_w, current_h);
	evas_object_size_hint_max_set(layout, current_w, current_h);
	// set new image size
	evas_object_size_hint_min_set(image, current_w, current_h);
	evas_object_size_hint_max_set(image, current_w, current_h);

	// compute image center
	xi = x + screen_size_w / 2;
	yi = y + screen_size_h / 2;
	// compute new up left corner in zoomed image
	x = xi * zoom_fator - screen_size_w / 2;
	y = yi * zoom_fator - screen_size_h / 2;

	// if image size small than screen size keep image size for scroll region
	w = (current_w > screen_size_w) ? screen_size_w : w;
	h = (current_h > screen_size_h) ? screen_size_h : h;
	// set scroll region
	EINA_LOG_DBG("SCROLL REGION %d %d %d %d",  x, y, w, h);

	elm_scroller_region_show(scroller, x, y, w, h);

	evas_object_show(image);

	return EINA_FALSE;
}

static Eina_Bool
_image_pop_cb(void *data, Elm_Object_Item *it)
{
	appdata_s *ad = data;

	EINA_LOG_DBG("pop the video frame");

	if (efl_util_set_window_screen_mode(ad->win, EFL_UTIL_SCREEN_MODE_DEFAULT) != EFL_UTIL_ERROR_NONE)
	{
		EINA_LOG_ERR("Can't set screen in default on mode");
	}
	eext_rotary_event_handler_del(_rotary_handler_cb);
	ad->cancel_requested = true;

	return EINA_TRUE;
}

/*
 * @brief Function to create gui object
 * @param[in] ad The data structure to manage gui object
 */
void
create_video_view(appdata_s *ad)
{
	Evas_Object *image_jpg;
	//Evas_Object *circle_scroller;
	Elm_Object_Item *nf_image;
	char edj_path[PATH_MAX] = {0, };
	int ret = 0;
	int x = -1;
	int y = -1;
	int w = 0;
	int h = 0;

	_appdata = ad;
	_app_naviframe = ad->naviframe;

	ad->downloading = false;
	ad->cancel_requested = false;
	ad->cleanup_done = false;
	//
	// init mjpeg decoder
	//
	jpeg_buffer = NULL;
	jpeg_frame_index = 0;
	jpeg_frame_position = 0;
	jpeg_num_headers = 0;
	block_counter = 0;
	headerline[0] = 0;
	total_bytes = 0;

	system_info_get_platform_int("tizen.org/feature/screen.width", &screen_size_w);
	system_info_get_platform_int("tizen.org/feature/screen.height", &screen_size_h);

	scroller = elm_scroller_add(_app_naviframe);

	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);

	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

	layout = elm_layout_add(scroller);
	ad->layout = layout;

	ret = elm_layout_file_set(layout, edj_path, "image_layout");
	if (! ret)
	{
		EINA_LOG_ERR("LAYOUT SET ERROR path:<%s> group:<%s>", edj_path, "image_layout");
	}
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	image_jpg = elm_image_add(layout);
	ad->image_jpg = image_jpg;

	//elm_image_memfile_set(image_jpg, &downloaded_image.image_file_buf, downloaded_image.image_size, "jpg", NULL);

	elm_object_part_content_set(layout, "image1", image_jpg);

	elm_image_smooth_set(image_jpg, EINA_FALSE);
	elm_image_aspect_fixed_set(image_jpg, EINA_TRUE);

	elm_image_fill_outside_set(image_jpg, EINA_FALSE);
	elm_image_editable_set(image_jpg, EINA_TRUE);

	elm_image_object_size_get(image_jpg, &w, &h);

	EINA_LOG_INFO("IMAGE SIZE w <%d> h <%d>", w, h);


	evas_object_size_hint_min_set(layout, w, h);
	evas_object_size_hint_max_set(layout, w, h);

	evas_object_size_hint_min_set(image_jpg, screen_size_w, screen_size_h);
	evas_object_size_hint_max_set(image_jpg, screen_size_w, screen_size_h);

	/* center image */
	x= (w - screen_size_w) / 2 >= 0 ? (w - screen_size_w) / 2 : 0;
	y =(h - screen_size_h) / 2 >= 0 ? (w - screen_size_h) / 2 : 0;
	w = (w > screen_size_w) ? screen_size_w : w;
	h = (h > screen_size_h) ? screen_size_h : h;


	elm_object_content_set(scroller, layout);


	elm_scroller_region_show(scroller, x, y, w, h);

	//evas_object_smart_callback_add(image_jpg, "clicked", _image_clicked_cb , image_jpg);
	eext_rotary_event_handler_add(_rotary_handler_cb, image_jpg);

	nf_image = elm_naviframe_item_push(_app_naviframe, NULL, NULL, NULL, scroller, "empty");
	elm_naviframe_item_pop_cb_set(nf_image, _image_pop_cb, ad);

	if (efl_util_set_window_screen_mode(ad->win, EFL_UTIL_SCREEN_MODE_ALWAYS_ON) != EFL_UTIL_ERROR_NONE)
	{
		EINA_LOG_ERR("Can't set screen in always on mode");
	}

	if (! _start_video_streaming(_appdata))
	{
		popup_text_1button(ad, "Can\'t get video :(");
		return;
	}
}
