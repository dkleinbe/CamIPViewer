/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd
 *
 * Licensed under the Flora License, Version 1.1 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://floralicense.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#if !defined(_VIEW_H)
#define _VIEW_H

//#define EDJ_FILE "edje/main.edj"
#define GRP_MAIN "main"

#ifdef __cplusplus
extern "C" {
#endif
Evas_Object *view_get_window(void);
Evas_Object *view_get_conformant(void);

/*
 * Create a view
 */
void view_create(void);
Evas_Object *view_create_win(const char *pkg_name);
Evas_Object *view_create_conformant_without_indicator(Evas_Object *win);
Evas_Object *view_create_layout(Evas_Object *parent, const char *file_path, const char *group_name, Eext_Event_Cb cb_function, void *user_data);
Evas_Object *view_create_layout_for_conformant(Evas_Object *parent, const char *file_path, const char *group_name, Eext_Event_Cb cb_function, void *user_data);
Evas_Object *view_create_layout_by_theme(Evas_Object *parent, const char *class_name, const char *group_name, const char *style);
void view_raise_window(void);
void view_lower_window(void);
void view_destroy(void);
void view_set_image(Evas_Object *parent, const char *part_name, const char *image_path);
void view_set_text(Evas_Object *parent, const char *part_name, const char *text);
void view_set_color(Evas_Object *parent, const char *part_name, int r, int g, int b, int a);
void view_set_color_of_circle_object(Evas_Object *parent, const char *part_name, int r, int g, int b, int a);
Evas_Object *view_create_label(Evas_Object *parent);
void view_set_label_text(Evas_Object *parent, const char *part_name, const char *text);
void view_set_label(Evas_Object *parent, const char *part_name);
void view_set_rotary_event_callback(Evas_Object *obj, Eext_Rotary_Event_Cb rotary_cb, void *user_data);
Evas_Object *view_create_progressbar(Evas_Object *parent, int radius, int line_width);
void view_set_progressbar(Evas_Object *parent, const char *part_name, int radius, int line_width);
void view_set_progressbar_val(Evas_Object *parent, const char *part_name, int val);
void view_set_button(Evas_Object *parent, const char *part_name, const char *style, const char *image_path, const char *text,
		        Evas_Object_Event_Cb down_cb, Evas_Object_Event_Cb up_cb, Evas_Smart_Cb clicked_cb, void *user_data);
void view_set_more_button(Evas_Object *parent, const char *part_name, Evas_Smart_Cb opened_cb, Evas_Smart_Cb closed_cb, Evas_Smart_Cb seleted_cb, void *user_data);
void view_add_more_button_item(Evas_Object *parent, const char *part_name, const char *main_txt, const char *sub_txt, const char *image_path, Evas_Smart_Cb clicked_cb, void *user_data);

void view_music_set_title_layout(Evas_Object *parent, const char *part_name, const char *edj_path, const char *image_path, const char *title);
void view_music_set_album_art(Evas_Object *parent, const char *part_name, const char *image_path, const char *default_image_path);
void view_music_change_more_item(Eext_Object_Item *item, const char *main_txt, const char *sub_txt, const char *image_path);

#ifdef __cplusplus
}
#endif

#endif
