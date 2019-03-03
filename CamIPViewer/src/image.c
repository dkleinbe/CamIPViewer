/*
 * image.c
 *
 *  Created on: Mar 2, 2019
 *      Author: denis
 */


#include "image.h"

static Evas_Object *_app_naviframe;

/*
 * @brief Function to create gui object
 * @param[in] ad The data structure to manage gui object
 */
void
create_image_view(appdata_s *ad)
{
	Evas_Object *image_jpg;
	char buf[256];

	_app_naviframe = ad->naviframe;

	image_jpg = elm_image_add(_app_naviframe);

	snprintf(buf, sizeof(buf), "%s/100_3.jpg", IMAGE_DIR);
	elm_image_file_set(image_jpg, buf, NULL);
	elm_object_part_content_set(_app_naviframe, "image_jpg", image_jpg);
	elm_image_no_scale_set(image_jpg, EINA_TRUE);
	elm_image_resizable_set(image_jpg, EINA_FALSE, EINA_FALSE);
	elm_image_smooth_set(image_jpg, EINA_FALSE);
	elm_image_aspect_fixed_set(image_jpg, EINA_TRUE);
	elm_image_fill_outside_set(image_jpg, EINA_TRUE);
	elm_image_editable_set(image_jpg, EINA_TRUE);

	elm_naviframe_item_push(_app_naviframe, NULL, NULL, NULL, image_jpg, "empty");
}
