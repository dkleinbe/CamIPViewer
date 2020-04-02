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

#define IMAGE_WIDTH 1024
#define IMAGE_HEIGHT 768
#define BUF_SIZE IMAGE_WIDTH * IMAGE_HEIGHT * 5

typedef struct image_data {
	char image_file_buf[BUF_SIZE];
	size_t image_size;
} image_data_s;

/*
 * Command identifiers for messages sent from the download thread
 * to the main thread.
 */
typedef enum feedback_cmd_e {
    FEEDBACK_CMD_NONE,
	FEEDBACK_CMD_NEW_FRAME,
    FEEDBACK_CMD_RESET_ALL,
    FEEDBACK_CMD_RESET_PROGRESS
} feedback_cmd_e;

// Message structure, which holds the command and its details.
typedef struct feedback_msg_s {
    appdata_s *ad;
    feedback_cmd_e command;
    image_data_s *frame;
    Evas_Object *object;
    char *string;
    double double_value;
    Eina_Bool eina_bool_value;
} feedback_msg_s;


#ifdef __cplusplus
extern "C" {
#endif
	void create_video_view(appdata_s *ad);
#ifdef __cplusplus
}
#endif


#endif /* _VIDEO_H_ */
