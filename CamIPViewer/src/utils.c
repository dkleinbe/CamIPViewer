/*
 * utils.c
 *
 *  Created on: Jan 16, 2019
 *      Author: denis
 */

#include "settings.h"
#include "utils.h"


/*
 * Net work utils
 */


 bool
 test_curl_connection(const char *url, const char *proxy_address)
 {
 	CURL *curl;
 	curl = curl_easy_init();
 	if (curl)
 	{
 		CURLcode ret;
 		curl_easy_setopt(curl, CURLOPT_URL, url);
 		curl_easy_setopt(curl, CURLOPT_PROXY, proxy_address);
 		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 2L);
 		curl_easy_setopt(curl, CURLOPT_CONNECT_ONLY, 1L);
 		ret = curl_easy_perform(curl);
 		if (ret == CURLE_OK)
 		{
 			curl_easy_cleanup(curl);
 			return true;
 		}
 		curl_easy_cleanup(curl);
 	}
 	return false;
 }


bool
init_net_connection(connection_h *connection)
 {
 	bool internet_available = false;
 	connection_type_e type;

 	int ret = 0;

 	if ((ret = connection_create(connection)) == CONNECTION_ERROR_NONE)
 	{

 		ret = connection_get_type(*connection, &type);
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

 	    			internet_available = true;

 					break;
 	    		case CONNECTION_TYPE_ETHERNET:
 					dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_ETHERNET");

 	    			internet_available = true;

 					break;
 	    		case CONNECTION_TYPE_BT:
 					dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_BT");

 	    			internet_available = true;

 					break;
 	    		case CONNECTION_TYPE_NET_PROXY:
 					dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_NET_PROXY");

 	    			internet_available = true;

 					break;
 	    		default:
 	    			dlog_print(DLOG_INFO, LOG_TAG, "CONNECTION_TYPE_UNKNOWN");

 	    			break;
 			}

 	    	if (internet_available != true)
 	    	{
 	    		// No connection available
 	    		connection_destroy(*connection);
 	    		return false;
 	    	}
 	    	return true;
 	    }
 	}
 	return false;
 }

CURL *
init_curl_connection(
		connection_h connection,
		char *url,
		void *write_callback,
		void *data)
{
	CURL *curl;
	char *proxy_address;

	int conn_err = -1;
	// get proxy info
	conn_err = connection_get_proxy(connection, CONNECTION_ADDRESS_FAMILY_IPV4, &proxy_address);
	if ( ! (conn_err == CONNECTION_ERROR_NONE && proxy_address))
	{
		return false;
	}
	// test connection to url
	if (test_curl_connection(url, proxy_address) != true)
	{
		connection_destroy(connection);
		return NULL;
	}
	// curl connection
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	curl_easy_setopt(curl, CURLOPT_PROXY, proxy_address);
	curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 6L);
	curl_easy_setopt(curl, CURLOPT_USERNAME, get_setting(CAM_USER));
	curl_easy_setopt(curl, CURLOPT_PASSWORD, get_setting(CAM_PASSWORD));
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, data);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);

	return curl;
}
/*
 * @brief Function to get absolute edje resource path
 * @param[in] edj_file_in The relative path of edje resource
 * @param[in] edj_path_out The pointer variable to write absolute path of edje resource
 * @param[in] edj_path_max The maximum size of path_out variable
 */
void
app_get_resource(const char *edj_file_in, char *edj_path_out, int edj_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(edj_path_out, edj_path_max, "%s%s", res_path, edj_file_in);
		free(res_path);
	}
}

static void
_popup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	if(!obj) return;
	elm_popup_dismiss(obj);
}

static void
_popup_hide_finished_cb(void *data, Evas_Object *obj, void *event_info)
{
	if(!obj) return;
	evas_object_del(obj);
}

static void _response_cb(void *data, Evas_Object *obj, void *event_info)
{
	if(!data) return;
	elm_popup_dismiss(data);
}

void
popup_text_1button(void *data, const char *txt)
{
	Evas_Object *popup;
	Evas_Object *btn;
	Evas_Object *layout;
	appdata_s *ad = (appdata_s *) data;

	popup = elm_popup_add(ad->win);
	elm_object_style_set(popup, "circle");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, _popup_hide_cb, NULL);
	evas_object_smart_callback_add(popup, "dismissed", _popup_hide_finished_cb, NULL);

	layout = elm_layout_add(popup);
	elm_layout_theme_set(layout, "layout", "popup", "content/circle");

	elm_object_part_text_set(layout, "elm.text", txt);
	elm_object_content_set(popup, layout);

	btn = elm_button_add(popup);
	elm_object_style_set(btn, "bottom");
	elm_object_text_set(btn, "OK");
	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", _response_cb, popup);

	evas_object_show(popup);
}
