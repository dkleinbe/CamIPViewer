/*
 * utils.c
 *
 *  Created on: Jan 16, 2019
 *      Author: denis
 */
#include <unistd.h>
#include <sys/syscall.h>

#include <wifi-manager.h>
#include <privacy_privilege_manager.h>
#include <storage.h>
#include "settings.h"
#include "utils.h"


static int _log_dom = -1;
static FILE *_log_fp = NULL;

static void _request_permission_cb(ppm_call_cause_e cause,
		ppm_request_result_e result, const char *privilege, void *user_data)
{
	if (cause == PRIVACY_PRIVILEGE_MANAGER_CALL_CAUSE_ERROR)
	{
		/* Log and handle errors */

		return;
	}

	switch (result)
	{
		case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_ALLOW_FOREVER:
			/* Update UI and start accessing protected functionality */
			break;
		case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_FOREVER:
			/* Show a message and terminate the application */
			break;
		case PRIVACY_PRIVILEGE_MANAGER_REQUEST_RESULT_DENY_ONCE:
			/* Show a message with explanation */
			break;
	}
}

static bool
_check_storage_permission()
{
	ppm_check_result_e result;
	const char *privilege = "http://tizen.org/privilege/mediastorage";

	int ret = ppm_check_permission(privilege, &result);
	if (ret != PRIVACY_PRIVILEGE_MANAGER_ERROR_NONE)
	{
		EINA_LOG_ERR("ERROR checking privilege");
		return false;
	}
	switch (result)
	{
		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ALLOW:
			return true;
		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_DENY:
			return false;
		case PRIVACY_PRIVILEGE_MANAGER_CHECK_RESULT_ASK:
			ret = ppm_request_permission(privilege, _request_permission_cb, NULL);
			return false;
		default:
			return false;

	}
	return false;
}

static bool
_storage_info_cb(int storage_id, storage_type_e type, storage_state_e state,
                            const char *path, void *user_data)
{
    int *id = (int *) user_data;
    if (type == STORAGE_TYPE_INTERNAL)
    {
        *id = storage_id;
        return false;
    }
    return true;
}

static int
_get_internal_storage_id(int *id)
{
    int error;
    int found_id;
    error = storage_foreach_device_supported(_storage_info_cb, &found_id);
    if (error != STORAGE_ERROR_NONE)
    {
    	EINA_LOG_ERR("storage_foreach_device_supported() failed: %d", error);
        return error;
    }
    *id = found_id;
    return STORAGE_ERROR_NONE;
}

static FILE *
_open_log_file()
{
	int internal_storage_id = -1;
    char write_filepath[1000] = {0,};
    char log_file[] = "camipviewer.log";
    char *root_dir = NULL;

    if(! _check_storage_permission())
	{
    	return NULL;
	}

    int error = _get_internal_storage_id(&internal_storage_id);
	if (error != STORAGE_ERROR_NONE)
	{
		EINA_LOG_ERR("Could not get internal storage ID");
		return NULL;
	}
    if (storage_get_directory(internal_storage_id, STORAGE_DIRECTORY_DOWNLOADS, &root_dir) != STORAGE_ERROR_NONE)
	{
    	EINA_LOG_ERR("Could not get root directory");
    	return NULL;
	}

    snprintf(write_filepath, strlen(root_dir) + strlen(log_file) + 2, "%s/%s", root_dir, log_file);

    EINA_LOG_INFO("LOG FILE: %s", write_filepath);

    free(root_dir);

    _log_fp = fopen(write_filepath, "w");
    if (_log_fp == NULL)
    {
    	EINA_LOG_ERR("ERROR OPENING FILE");
    	EINA_LOG_ERR("fopen(): %d, errno: %d, strerror(): %s",
    			(int)_log_fp, errno, strerror(errno));
    	return NULL;
    }

    if (fputs("STARTING LOG\n", _log_fp) == EOF)
    {
    	EINA_LOG_ERR("ERROR WRITING FILE");
    	return NULL;
    }

    return (_log_fp);

#ifdef AZE
    char *data_path = app_get_data_path(); // get the application data directory path


    if(data_path)
    {
        snprintf(write_filepath, 999, "%s/%s", data_path, "log.log");
        //free(resource_path);
    }

/*
    _log_fp = fopen(write_filepath, "w");
    if (_log_fp == NULL)
    {
    	dlog_print(DLOG_ERROR, LOG_TAG, "ERROR OPENING FILE");
    	return NULL;
    }

    if (fputs("COUCOU", _log_fp) == EOF)
    	dlog_print(DLOG_ERROR, LOG_TAG, "ERROR WRITING FILE");

    fclose(_log_fp);
*/
    _log_fp = fopen(write_filepath, "r");

    char str[256] = {0, };
    fgets(str, 50, _log_fp);

    if (strlen(str))
    	dlog_print(DLOG_ERROR, LOG_TAG, "SUCCES READING FILE");

    return _log_fp;
#endif

}
static void
_print_cb(const Eina_Log_Domain *domain,
              Eina_Log_Level level,
              const char *file,
              const char *fnc,
              int line,
              const char *fmt,
              void *data,
              va_list args)
{
	int log_level;
	FILE *fp = (FILE *)data;
	const char buf[512];
	char *log_level_str[32] = { "DLOG_FATAL", "DLOG_ERROR", "DLOG_WARN",  "DLOG_INFO", "DLOG_DEBUG", "DLOG_VERBOSE"};
	char *log_level_ptr = NULL;

	switch (level)
	{
		case EINA_LOG_LEVEL_CRITICAL:
			log_level = DLOG_FATAL;
			log_level_ptr = log_level_str[0];
			break;
		case EINA_LOG_LEVEL_ERR:
			log_level = DLOG_ERROR;
			log_level_ptr = log_level_str[1];
			break;
		case EINA_LOG_LEVEL_WARN:
			log_level = DLOG_WARN;
			log_level_ptr = log_level_str[2];
			break;
		case EINA_LOG_LEVEL_INFO:
			log_level = DLOG_INFO;
			log_level_ptr = log_level_str[3];
			break;
		case EINA_LOG_LEVEL_DBG:
			log_level = DLOG_DEBUG;
			log_level_ptr = log_level_str[4];
			break;
		default:
			log_level = DLOG_VERBOSE;
			log_level_ptr = log_level_str[5];
			break;
	}

	vsnprintf((char *) buf, sizeof(buf), fmt, args);

	//
	// print to regular output
	//
	dlog_print(log_level, domain->name,
			"%s:%d %s()> %s",
			file, line, fnc, buf);

	//
	// print to log file
	//
	if (fp)
	{
#ifdef SYS_gettid
		pid_t tid = syscall(SYS_gettid);
#else
#error "SYS_gettid unavailable on this system"
#endif
		//fputs(buf, fp);
		fprintf(fp,"[%s] <%d> <%d> [%s] %s:%d %s()> %s\n",
				log_level_ptr,
				getpid(),
				tid,
				domain->name,
				file, line, fnc, buf);

		fflush(fp);
	}
}

/**
 * Initialize utility with application data ptr
 * must be called before any call to utility function
 *
 */
void
utils_init(void)
{

	eina_init();
	_log_dom = eina_log_domain_register(LOG_TAG, EINA_COLOR_CYAN);
	EINA_LOG_ERR("using my own domain");

	eina_log_threads_enable();
	eina_log_domain_level_set(LOG_TAG, EINA_LOG_LEVEL_DBG);
	_log_fp = _open_log_file();
	eina_log_print_cb_set(_print_cb, _log_fp);

	EINA_LOG_DBG("Utilities initialized");

}

void
utils_cleanup(void)
{
	if (_log_fp)
	{
		EINA_LOG_INFO("Closing log file");

		fclose(_log_fp);
		_log_fp = NULL;
	}
}

int
get_log_dom(void)
{
	return _log_dom;
}


/*
 * Net work utils
 */

/**
 * Checks if Wifi is activated
 * @return
 */
static bool
_is_wifi_activated()
{
	wifi_manager_h wifi;
	int error_code;
	bool wifi_manager_activated = false;

	error_code = wifi_manager_initialize(&wifi);
	EINA_LOG_INFO("wifi_manager_initialize");

	if (error_code == WIFI_MANAGER_ERROR_NONE )
	{

		wifi_manager_is_activated(wifi, &wifi_manager_activated);

		wifi_manager_deinitialize(wifi);

		if (wifi_manager_activated)
			EINA_LOG_INFO("Wi-Fi manager activated.");
		else
			EINA_LOG_INFO("Wi-Fi manager NOT activated..");
	}
	else
	{
		EINA_LOG_ERR("Cant' initialize wifi manager: %s (%d)", curl_easy_strerror(error_code), error_code);
	}


	return (wifi_manager_activated);

}


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
 		EINA_LOG_INFO("test_curl_connection url: [%s] proxy: [%s]", url, proxy_address);
 		ret = curl_easy_perform(curl);
 		if (ret == CURLE_OK)
 		{
 			curl_easy_cleanup(curl);
 			return true;
 		}
 		curl_easy_cleanup(curl);
 	}
 	EINA_LOG_ERR("test_curl_connection");

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
 	    			EINA_LOG_ERR("CONNECTION_TYPE_DISCONNECTED");

 	    			break;
 	    		case CONNECTION_TYPE_WIFI:
 	    			EINA_LOG_INFO("CONNECTION_TYPE_WIFI");

 	    			internet_available = true;

 	    			break;
 	    		case CONNECTION_TYPE_CELLULAR:
 	    			EINA_LOG_INFO("CONNECTION_TYPE_CELLULAR");

 	    			internet_available = true;

 					break;
 	    		case CONNECTION_TYPE_ETHERNET:
 	    			EINA_LOG_INFO("CONNECTION_TYPE_ETHERNET");

 	    			internet_available = true;

 					break;
 	    		case CONNECTION_TYPE_BT:
 	    			EINA_LOG_INFO("CONNECTION_TYPE_BT");

 	    			internet_available = true;

 					break;
 	    		case CONNECTION_TYPE_NET_PROXY:
 	    			EINA_LOG_INFO("CONNECTION_TYPE_NET_PROXY");

 	    			internet_available = true;

 					break;
 	    		default:
 	    			EINA_LOG_INFO("CONNECTION_TYPE_UNKNOWN");

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
/**
 *
 * @param connection
 * @param url
 * @param write_callback
 * @param write_data
 * @param progress_callback
 * @param progress_data
 * @return
 */
CURL *
init_curl_connection(
		connection_h connection,
		char *url,
		void *write_callback,
		void *write_data,
		void *progress_callback,
		void *progress_data)
{
	static char no_proxy = 0;
	CURL *curl;
	CURLcode error_code;
	char *proxy_address;
	int conn_err = -1;
	bool is_wifi_activated = false;

	//
	// manage proxy settings
	//
	int msg_level = EINA_LOG_LEVEL_INFO;
	is_wifi_activated = _is_wifi_activated();
	if (is_wifi_activated)
	{
		//
		// if wifi connection exist set NO proxy
		//
		proxy_address = &no_proxy;
	}
	else
	{
		//
		// if NO wifi connection set the proxy
		//
		// get proxy info
		conn_err = connection_get_proxy(connection, CONNECTION_ADDRESS_FAMILY_IPV4, &proxy_address);

		if ( ! (conn_err == CONNECTION_ERROR_NONE && proxy_address))
		{
			EINA_LOG_ERR("connection_get_proxy: CONNECTION_ERROR_NONE");
			return NULL;
		}
	}
	// test connection to url
	if (test_curl_connection(url, proxy_address) != true)
	{
		EINA_LOG_ERR("test_curl_connection: ERROR");
		connection_destroy(connection);
		return NULL;
	}
	// init curl connection
	curl = curl_easy_init();
	if (! curl)
	{
		EINA_LOG_ERR("curl_easy_init: CURL ERROR");

		return NULL;

	}
	// set url
	error_code = curl_easy_setopt(curl, CURLOPT_URL, url);
	if (error_code != CURLE_OK)
	{
		msg_level = EINA_LOG_LEVEL_ERR;
	}
	EINA_LOG_DOM_DEFAULT(msg_level,
	                   "error_code = curl_easy_setopt(curl, CURLOPT_URL, url): %s (%d)",
	                   curl_easy_strerror(error_code), error_code);

	// set proxy
	error_code = curl_easy_setopt(curl, CURLOPT_PROXY, proxy_address);
	if (error_code != CURLE_OK)
	{
		msg_level = EINA_LOG_LEVEL_ERR;
	}
	EINA_LOG_DOM_DEFAULT(msg_level,
					"curl_easy_setopt(curl, CURLOPT_PROXY, <%s>): %s (%d)", proxy_address,
					curl_easy_strerror(error_code), error_code);
	// set timeout
	error_code = curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 6L);
	if (error_code != CURLE_OK)
	{
		msg_level = EINA_LOG_LEVEL_ERR;
	}
	EINA_LOG_DOM_DEFAULT(msg_level,
					   "curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 6L): %s (%d)",
					   curl_easy_strerror(error_code), error_code);
	// set user name
	error_code = curl_easy_setopt(curl, CURLOPT_USERNAME, get_setting(CAM_USER));
	if (error_code != CURLE_OK)
	{
		msg_level = EINA_LOG_LEVEL_ERR;
	}
	EINA_LOG_DOM_DEFAULT(msg_level,
					   "curl_easy_setopt(curl, CURLOPT_USERNAME, get_setting(CAM_USER)): %s (%d)",
					   curl_easy_strerror(error_code), error_code);
	// set password
	error_code = curl_easy_setopt(curl, CURLOPT_PASSWORD, get_setting(CAM_PASSWORD));
	if (error_code != CURLE_OK)
	{
		msg_level = EINA_LOG_LEVEL_ERR;
	}
	EINA_LOG_DOM_DEFAULT(msg_level,
					   "curl_easy_setopt(curl, CURLOPT_PASSWORD, get_setting(CAM_PASSWORD)): %s (%d)",
					   curl_easy_strerror(error_code), error_code);
	// set write callback data
	error_code = curl_easy_setopt(curl, CURLOPT_WRITEDATA, write_data);
	if (error_code != CURLE_OK)
	{
		msg_level = EINA_LOG_LEVEL_ERR;
	}
	EINA_LOG_DOM_DEFAULT(msg_level,
					   "curl_easy_setopt(curl, CURLOPT_WRITEDATA, write_data): %s (%d)",
					   curl_easy_strerror(error_code), error_code);
	// set write callback
	error_code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
	if (error_code != CURLE_OK)
	{
		msg_level = EINA_LOG_LEVEL_ERR;
	}
	EINA_LOG_DOM_DEFAULT(msg_level,
					   "curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback): %s (%d)",
					   curl_easy_strerror(error_code), error_code);

	if (progress_callback)
	{
		// Enable the built-in progress meter
		error_code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
		if (error_code != CURLE_OK)
		{
			msg_level = EINA_LOG_LEVEL_ERR;
		}
		EINA_LOG_DOM_DEFAULT(msg_level,
						   "curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L): %s (%d)",
						   curl_easy_strerror(error_code), error_code);
		// Set progress callback data
		error_code = curl_easy_setopt(curl, CURLOPT_XFERINFODATA, progress_data);
		if (error_code != CURLE_OK)
		{
			msg_level = EINA_LOG_LEVEL_ERR;
		}
		EINA_LOG_DOM_DEFAULT(msg_level,
						   "curl_easy_setopt(curl, CURLOPT_XFERINFODATA, progress_data): %s (%d)",
						   curl_easy_strerror(error_code), error_code);
		// Set the progress callback
		error_code = curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progress_callback);
		if (error_code != CURLE_OK)
		{
			msg_level = EINA_LOG_LEVEL_ERR;
		}
		EINA_LOG_DOM_DEFAULT(msg_level,
						   "curl_easy_setopt(curl, CURLOPT_XFERINFODATA, progress_callback): %s (%d)",
						   curl_easy_strerror(error_code), error_code);
	}

	if (! is_wifi_activated)
	{
		free(proxy_address);
	}
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
