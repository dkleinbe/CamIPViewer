/*
 * settings.h
 *
 *  Created on: Jan 15, 2019
 *      Author: denis
 */

#if !defined(_SETTINGS_H_)
#define _SETTINGS_H_

#include "camipviewer.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _Settings_enum
{
	CAM_IP = 0,
	CAM_PORT,
	CAM_USER,
	CAM_PASSWORD,
	NB_SETTINGS
} Settings_enum;


typedef struct _App_setting
{
	char *key; /**< name of the setting key */
	char *name; /**< display name of the setting */
	Elm_Input_Panel_Layout input_panel_layout; /**< input panel layout @see Elm_Input_Panel_Layout */
	char *value; /**< value of the setting, should be free */
	char *guide; /**< guide value for setting entry */
} App_setting;

	void create_settings_list_view(appdata_s *ad);
	const char *get_setting(int index);
#ifdef __cplusplus
}
#endif

#endif /* _SETTINGS_H_ */
