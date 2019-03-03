/*
 * image.h
 *
 *  Created on: Mar 2, 2019
 *      Author: denis
 */

#if !defined(_IMAGE_H_)
#define _IMAGE_H_

#include "camipviewer.h"

#define IMAGE_DIR "/opt/usr/apps/org.example.camipviewer/res/images"

#ifdef __cplusplus
extern "C" {
#endif
	void create_image_view(appdata_s *ad);
#ifdef __cplusplus
}
#endif

#endif /* _IMAGE_H_ */
