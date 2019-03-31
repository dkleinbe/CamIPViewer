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

#define MY_TEST
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
	char url[1024];

	int ret = 0;

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
	    			break;
	    		case CONNECTION_TYPE_WIFI:
	    			dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_WIFI");
	    			internet_available = true;
	    			break;
	    		case CONNECTION_TYPE_CELLULAR:
					dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_CELLULAR");
					break;
	    		case CONNECTION_TYPE_ETHERNET:
					dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_ETHERNET");
					internet_available = true;
					break;
	    		case CONNECTION_TYPE_BT:
					dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_BT");
					break;
	    		case CONNECTION_TYPE_NET_PROXY:
					dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_NET_PROXY");
					break;
	    		default:
	    			dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_UNKNOWN");
	    			break;
			}
	    	downloaded_image.image_size = 0;
	    	//curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.22:8080/shot.jpg");
	    	snprintf(url, 1023, "http://%s:%s/shot.jpg", get_setting(CAM_IP), get_setting(CAM_PORT));

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

	    }
	}
	else {
		dlog_print(DLOG_ERROR, LOG_TAG, "CONNECTION ERROR");
		return false;
	}
	return false;
}
#endif /* MY_TEST */

static void
_image_clicked_cb(void *data, Evas_Object *obj EINA_UNUSED, void *event_info EINA_UNUSED)
{
	Evas_Object *image = data;
	Elm_Image_Orient orient;
	int w,h;

	orient = elm_image_orient_get(image);

	if (orient >= 7)
		orient = 0;
	else
		orient++;

	elm_image_object_size_get(image , &w, &h);
	dlog_print(DLOG_INFO, LOG_TAG, "image clicked! orient = %d , width = %d , height = %d\n", orient, w, h);
	//elm_image_orient_set(image , orient);
	evas_object_size_hint_min_set(image, w/2, h/2);
	evas_object_size_hint_max_set(image, w/2, h/2);
	evas_object_show(image);
}

/*
 * @brief Function to create gui object
 * @param[in] ad The data structure to manage gui object
 */
void
create_image_view(appdata_s *ad)
{
	Evas_Object *image_jpg;
	Evas_Object *scroller;
	//Evas_Object *circle_scroller;
	Evas_Object *layout;
	char edj_path[PATH_MAX] = {0, };
	char buf[256];
	int ret = 0;
	int x = -1;
	int y = -1;
	int w = 0;
	int h = 0;
	int screen_size_w = 0;
	int screen_size_h = 0;

	_app_naviframe = ad->naviframe;

	system_info_get_platform_int("tizen.org/feature/screen.width", &screen_size_w);
	system_info_get_platform_int("tizen.org/feature/screen.height", &screen_size_h);

#ifndef MY_TEST
	app_init_curl();
#endif

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
	evas_object_size_hint_min_set(image_jpg, w, h);
	evas_object_size_hint_max_set(image_jpg, w, h);

	/* center image */
	x= (w - screen_size_w) / 2 >= 0 ? (w - screen_size_w) / 2 : 0;
	y =(h - screen_size_h) / 2 >= 0 ? (w - screen_size_h) / 2 : 0;
	w = (w > screen_size_w) ? screen_size_w : w;
	h = (h > screen_size_h) ? screen_size_h : h;


#ifdef MY_LAYOUT
	elm_object_content_set(scroller, layout);
#endif /* MY_LAYOUT */

	elm_scroller_region_show(scroller, x, y, w, h);

	evas_object_smart_callback_add(image_jpg, "clicked", _image_clicked_cb , image_jpg);

	elm_naviframe_item_push(_app_naviframe, NULL, NULL, NULL, scroller, "empty");
}
