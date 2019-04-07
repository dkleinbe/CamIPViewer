/*
 * image.c
 *
 *  Created on: Mar 2, 2019
 *      Author: denis
 */

#include <net_connection.h>
#include <curl/curl.h>
#include <system_info.h>
#include "utils.h"
#include "image.h"
#include "settings.h"

#define IMAGE_WIDTH 1024
#define IMAGE_HEIGHT 768
#define BUF_SIZE IMAGE_WIDTH * IMAGE_HEIGHT * 4

static Evas_Object *_app_naviframe;
/* FIXME: move to a structure */
static appdata_s *_appdata;
static Evas_Object *scroller;
static Evas_Object *layout;
static int screen_size_w = 0;
static int screen_size_h = 0;

//#define MY_TEST
#ifndef MY_TEST

typedef struct image_data {
	char image_file_buf[BUF_SIZE];
	size_t image_size;
} image_data_s;


static image_data_s downloaded_image;



static size_t
write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	dlog_print(DLOG_INFO, LOG_TAG, "data read size: %d", nmemb);
	memcpy(downloaded_image.image_file_buf + downloaded_image.image_size, ptr, nmemb);
	downloaded_image.image_size += nmemb;
	return nmemb;
}


static bool
app_init_curl()
{
	CURL *curl;
	CURLcode curl_err;

	bool internet_available = false;
	connection_h connection;
	connection_type_e type;
	char *proxy_address;
	char url[1024];

	int ret = 0;
	int conn_err = -1;

	curl = curl_easy_init();

	if ((ret = connection_create(&connection)) == CONNECTION_ERROR_NONE)
	{

		ret = connection_get_type(connection, &type);
	    if(ret == CONNECTION_ERROR_NONE)
	    {
	    	internet_available = false;
	    	switch (type)
			{
	    		case CONNECTION_TYPE_DISCONNECTED:
	    			dlog_print(DLOG_ERROR, LOG_TAG, "CONNECTION_TYPE_DISCONNECTED");
	    			popup_text_1button(_appdata, "CONNECTION_TYPE_DISCONNECTED");
	    			break;
	    		case CONNECTION_TYPE_WIFI:
	    			dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_WIFI");
	    			popup_text_1button(_appdata, "CONNECTION_TYPE_WIFI");
	    			internet_available = true;
	    			break;
	    		case CONNECTION_TYPE_CELLULAR:
					dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_CELLULAR");
					popup_text_1button(_appdata, "CONNECTION_TYPE_CELLULAR");
					break;
	    		case CONNECTION_TYPE_ETHERNET:
					dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_ETHERNET");
					popup_text_1button(_appdata, "CONNECTION_TYPE_ETHERNET");
					internet_available = true;
					break;
	    		case CONNECTION_TYPE_BT:
					dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_BT");
					popup_text_1button(_appdata, "CONNECTION_TYPE_BT");

					break;
	    		case CONNECTION_TYPE_NET_PROXY:
					dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_NET_PROXY");
					popup_text_1button(_appdata, "CONNECTION_TYPE_NET_PROXY");

					break;
	    		default:
	    			dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_UNKNOWN");
	    			popup_text_1button(_appdata, "CONNECTION_TYPE_UNKNOWN");
	    			break;
			}
	    	downloaded_image.image_size = 0;

	    	// set URL to fetch
	    	snprintf(url, 1023, "http://%s:%s/shot.jpg", get_setting(CAM_IP), get_setting(CAM_PORT));
	    	// get proxy settings
			conn_err = connection_get_proxy(connection, CONNECTION_ADDRESS_FAMILY_IPV4, &proxy_address);
			if (conn_err == CONNECTION_ERROR_NONE && proxy_address)
			{
				curl_easy_setopt(curl, CURLOPT_PROXY, proxy_address);
				internet_available = true;
			}
			else
			{
				popup_text_1button(_appdata, "connection_get_proxy error");
			}
	    	curl_easy_setopt(curl, CURLOPT_URL, url);
	    	curl_easy_setopt(curl, CURLOPT_USERNAME, get_setting(CAM_USER));
	    	curl_easy_setopt(curl, CURLOPT_PASSWORD, get_setting(CAM_PASSWORD));
	    	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	    	curl_err = curl_easy_perform(curl);
	    	if (curl_err != CURLE_OK)
	    	{
	    		dlog_print(DLOG_ERROR, LOG_TAG, "CURL ERROR");
	    		return false;
	    	}
		    curl_easy_cleanup(curl);
		    connection_unset_proxy_address_changed_cb(connection);
		    connection_destroy(connection);
		    return true;
	    }
	}
	else {
		dlog_print(DLOG_ERROR, LOG_TAG, "CONNECTION ERROR");
		return false;
	}
	return false;
}
#endif /* MY_TEST */

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
	dlog_print(DLOG_INFO, LOG_TAG, "image clicked!, width = %d , height = %d\n", w, h);

    if (ev->direction == EEXT_ROTARY_DIRECTION_CLOCKWISE)
    {
        dlog_print(DLOG_DEBUG, LOG_TAG,
                   "Rotary device rotated in clockwise direction");
    	// If all ready over zoomed return
    	if (current_w >= image_w || current_h >= image_h)
    	{
    		return EINA_FALSE;
    	}
        zoom_fator = 2;
    }
    else
    {
        dlog_print(DLOG_DEBUG, LOG_TAG,
                   "Rotary device rotated in counter-clockwise direction");
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
	elm_scroller_region_show(scroller, x, y, w, h);

	evas_object_show(image);

	return EINA_FALSE;
}

static Eina_Bool
_image_pop_cb(void *data, Elm_Object_Item *it)
{
	eext_rotary_event_handler_del(_rotary_handler_cb);
	return EINA_TRUE;
}

/*
 * @brief Function to create gui object
 * @param[in] ad The data structure to manage gui object
 */
void
create_image_view(appdata_s *ad)
{
	Evas_Object *image_jpg;
	//Evas_Object *circle_scroller;
	Elm_Object_Item *nf_image;
	char edj_path[PATH_MAX] = {0, };
	char buf[256];
	int ret = 0;
	int x = -1;
	int y = -1;
	int w = 0;
	int h = 0;

	_appdata = ad;
	_app_naviframe = ad->naviframe;

	system_info_get_platform_int("tizen.org/feature/screen.width", &screen_size_w);
	system_info_get_platform_int("tizen.org/feature/screen.height", &screen_size_h);

#ifndef MY_TEST
	if (! app_init_curl())
	{
		return;
	}
#endif
	popup_text_1button(ad, "Feel good until now...");

	scroller = elm_scroller_add(_app_naviframe);

	elm_scroller_bounce_set(scroller, EINA_TRUE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_AUTO, ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);
/*
	circle_scroller = eext_circle_object_scroller_add(scroller, ad->circle_surface);
	eext_circle_object_scroller_policy_set(circle_scroller, ELM_SCROLLER_POLICY_ON, ELM_SCROLLER_POLICY_ON);
	eext_rotary_object_event_activated_set(circle_scroller, EINA_TRUE);
*/
#define MY_LAYOUT
#ifdef MY_LAYOUT
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);

	layout = elm_layout_add(scroller);
	ret = elm_layout_file_set(layout, edj_path, "image_layout");
	if (! ret)
	{
		dlog_print(DLOG_ERROR, LOG_TAG, "LAYOUT SET ERROR path:<%> group:<%s>", edj_path, "image_layout");
	}
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	image_jpg = elm_image_add(layout);
#else
	image_jpg = elm_image_add(scroller);
#endif /* MY_LAYOUT */

#ifdef MY_TEST
	snprintf(buf, sizeof(buf), "%s/faustine.jpg",IMAGE_DIR);
	ret = elm_image_file_set(image_jpg, buf, NULL);
	if (! ret)
	{
		dlog_print(DLOG_ERROR, LOG_TAG, "elm_image_file_set ERROR image:<%> ", buf);
	}
	evas_object_show(image_jpg);
#else
	elm_image_memfile_set(image_jpg, &downloaded_image.image_file_buf, downloaded_image.image_size, "jpg", NULL);
#endif /* MY_TEST */

#ifdef MY_LAYOUT
	elm_object_part_content_set(layout, "image1", image_jpg);
#else
	elm_object_content_set(scroller, image_jpg);
#endif /* MY_LAYOUT */

	//elm_image_no_scale_set(image_jpg, EINA_TRUE);
	//elm_image_resizable_set(image_jpg, EINA_FALSE, EINA_FALSE);
	elm_image_smooth_set(image_jpg, EINA_FALSE);
	elm_image_aspect_fixed_set(image_jpg, EINA_TRUE);

	elm_image_fill_outside_set(image_jpg, EINA_FALSE);
	elm_image_editable_set(image_jpg, EINA_TRUE);
	//elm_image_orient_set(image_jpg , ELM_IMAGE_ORIENT_90);

	elm_image_object_size_get(image_jpg, &w, &h);

	dlog_print(DLOG_INFO, LOG_TAG, "IMAGE SIZE w <%d> h <%d>", w, h);

#ifdef MY_LAYOUT
	evas_object_size_hint_min_set(layout, w, h);
	evas_object_size_hint_max_set(layout, w, h);
#endif /* MY_LAYOUT */
	evas_object_size_hint_min_set(image_jpg, screen_size_w, screen_size_h);
	evas_object_size_hint_max_set(image_jpg, screen_size_w, screen_size_h);

	/* center image */
	x= (w - screen_size_w) / 2 >= 0 ? (w - screen_size_w) / 2 : 0;
	y =(h - screen_size_h) / 2 >= 0 ? (w - screen_size_h) / 2 : 0;
	w = (w > screen_size_w) ? screen_size_w : w;
	h = (h > screen_size_h) ? screen_size_h : h;


#ifdef MY_LAYOUT
	elm_object_content_set(scroller, layout);
#endif /* MY_LAYOUT */

	elm_scroller_region_show(scroller, x, y, w, h);

	//evas_object_smart_callback_add(image_jpg, "clicked", _image_clicked_cb , image_jpg);
	eext_rotary_event_handler_add(_rotary_handler_cb, image_jpg);

	nf_image = elm_naviframe_item_push(_app_naviframe, NULL, NULL, NULL, scroller, "empty");
	elm_naviframe_item_pop_cb_set(nf_image, _image_pop_cb, NULL);
}
