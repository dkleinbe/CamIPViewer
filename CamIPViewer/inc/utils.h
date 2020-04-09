/*
 * utils.h
 *
 *  Created on: Jan 16, 2019
 *      Author: denis
 */

#if !defined(_UTILS_H_)
#define _UTILS_H_


#include "camipviewer.h"
#include <Eina.h>

#undef EINA_LOG_DOMAIN_DEFAULT
#define EINA_LOG_DOMAIN_DEFAULT get_log_dom()

#define EINA_LOG_DOM_DEFAULT(level, fmt, ...) \
  EINA_LOG(EINA_LOG_DOMAIN_DEFAULT, level, fmt, ## __VA_ARGS__)


#define MAX_LENGTH_PATH 1024

static inline const char *get_resource_path(const char *file_path)
{
	static char absolute_path[MAX_LENGTH_PATH] = {'\0'};

	static char *res_path_buff = NULL;
	if(res_path_buff == NULL)
	{
		res_path_buff = app_get_resource_path();
	}

	snprintf(absolute_path, MAX_LENGTH_PATH, "%s%s", res_path_buff, file_path);

	return absolute_path;
}

#ifdef __cplusplus
extern "C" {
#endif
	void utils_init(void);
	void utils_cleanup(void);
	void utils_set_app_data(void);
	int get_log_dom(void);
	bool init_net_connection(connection_h *connection);
	bool test_curl_connection(const char *url, const char *proxy_address);
	CURL *init_curl_connection(connection_h connection, char *url, void *write_callback, void *write_data, void *progress_callback, void *progress_data);
	void app_get_resource(const char *edj_file_in, char *edj_path_out, int edj_path_max);
	void popup_text_1button(void *data, const char *txt);
#ifdef __cplusplus
}
#endif

#endif /* _UTILS_H_ */
