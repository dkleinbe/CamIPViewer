/*
 * utils.h
 *
 *  Created on: Jan 16, 2019
 *      Author: denis
 */

#if !defined(_UTILS_H_)
#define _UTILS_H_


#include "camipviewer.h"

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
	bool init_net_connection(connection_h *connection);
	bool test_curl_connection(const char *url, const char *proxy_address);
	CURL *init_curl_connection(connection_h connection, char *url, void *write_callback, void *data);
	void app_get_resource(const char *edj_file_in, char *edj_path_out, int edj_path_max);
	void popup_text_1button(void *data, const char *txt);
#ifdef __cplusplus
}
#endif

#endif /* _UTILS_H_ */
