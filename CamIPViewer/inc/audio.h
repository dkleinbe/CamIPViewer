/*
 * image.h
 *
 *  Created on: Mar 2, 2019
 *      Author: denis
 */

#if !defined(_AUDIO_H_)
#define _AUDIO_H_

#include "camipviewer.h"

#define IMAGE_DIR "/opt/usr/apps/org.example.camipviewer/res/images"

#ifdef __cplusplus
extern "C" {
#endif
	void create_audio_view(appdata_s *ad);
	bool on_app_create_audio(void *user_data);
#ifdef __cplusplus
}
#endif

#endif /* _AUDIO_H_ */
