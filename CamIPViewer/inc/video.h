/*
 * video.h
 *
 *  Created on: Apr 13, 2019
 *      Author: denis
 */


#if !defined(_VIDEO_H_)
#define _VIDEO_H_

#include "camipviewer.h"

#define VIDEO_DIR "/opt/usr/globalapps/org.example.media/res/videos"

#ifdef __cplusplus
extern "C" {
#endif
	void create_video_view(appdata_s *ad);
#ifdef __cplusplus
}
#endif


#endif /* _VIDEO_H_ */
