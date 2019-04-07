/*
 * utils.h
 *
 *  Created on: Jan 16, 2019
 *      Author: denis
 */

#if !defined(_UTILS_H_)
#define _UTILS_H_

#include "camipviewer.h"

#ifdef __cplusplus
extern "C" {
#endif
	void app_get_resource(const char *edj_file_in, char *edj_path_out, int edj_path_max);
	void popup_text_1button(void *data, Evas_Object *obj, void *event_info);
#ifdef __cplusplus
}
#endif

#endif /* _UTILS_H_ */
