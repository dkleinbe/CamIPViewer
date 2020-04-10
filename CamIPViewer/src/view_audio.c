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

#include <tizen.h>
#include <Elementary.h>
#include <efl_extension.h>
#include <app.h>
#include <dlog.h>
#include "camipviewer.h"
#include "view_audio.h"

#define LABEL_STYLE_START "<font=Tizen:style=Regular><font_size=36><align=center><color=#FAFAFA><wrap=mixed>"
#define LABEL_STYLE_END "</wrap></color></align></font_size></font>"

static struct view_info {
	Evas_Object *win;
	Evas_Object *conform;
} s_info = {
	.win = NULL,
	.conform = NULL,
};

static void _win_delete_request_cb(void *user_data, Evas_Object *obj, void *event_info);

/*
 * @brief: Get window
 */
Evas_Object *view_get_window(void)
{
	return s_info.win;
}

/*
 * @brief: Get conformant
 */
Evas_Object *view_get_conformant(void)
{
	return s_info.conform;
}

/*
 * @brief: Create Essential Object window, conformant and layout
 */
void view_create(void)
{
	/* Create window */
	s_info.win = view_create_win(PACKAGE);
	if (s_info.win == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create a window.");
		return;
	}

	/* Create conformant */
	s_info.conform = view_create_conformant_without_indicator(s_info.win);
	if (s_info.conform == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create a conformant");
		return;
	}

	/* Show window after main view is set up */
	evas_object_show(s_info.win);
}

/*
 * @brief: Make a basic window named package
 * @param[pkg_name]: Name of the window
 */
Evas_Object *view_create_win(const char *pkg_name)
{
	Evas_Object *win = NULL;

	/*
	 * Window
	 * Create and initialize elm_win.
	 * elm_win is mandatory to manipulate window
	 */
	win = elm_win_util_standard_add(pkg_name, pkg_name);
	elm_win_conformant_set(win, EINA_TRUE);
	elm_win_autodel_set(win, EINA_TRUE);

	/* Rotation setting */
	if (elm_win_wm_rotation_supported_get(win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(win, "delete,request", _win_delete_request_cb, NULL);

	return win;
}

/*
 * @brief: Make a conformant without indicator for wearable app
 * @param[win]: The object to which you want to set this conformant
 * Conformant is mandatory for base GUI to have proper size
 */
Evas_Object *view_create_conformant_without_indicator(Evas_Object *win)
{
	/*
	 * Conformant
	 * Create and initialize elm_conformant.
	 * elm_conformant is mandatory for base GUI to have proper size
	 * when indicator or virtual keypad is visible.
	 */
	Evas_Object *conform = NULL;

	if (win == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "window is NULL.");
		return NULL;
	}

	conform = elm_conformant_add(win);
	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(win, conform);

	evas_object_show(conform);

	return conform;
}

/*
 * @brief: Make a layout with theme.
 * @param[parent]: Object to which you want to add this layout
 * @param[class_name]: The class of the group
 * @param[group_name]: Group name in EDJ that you want to set to layout
 * @param[style]: The style to use
 */
Evas_Object *view_create_layout_by_theme(Evas_Object *parent, const char *class_name, const char *group_name, const char *style)
{
	/*
	 * Layout
	 * Create and initialize elm_layout.
	 * view_create_layout_by_theme() is used to create layout by using premade edje file.
	 */
	Evas_Object *layout = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return NULL;
	}

	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, class_name, group_name, style);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_show(layout);

	return layout;
}

/*
 * @brief: Destroy window and free important data to finish this application
 */
void view_destroy(void)
{
	if (s_info.win == NULL) {
		return;
	}

	evas_object_del(s_info.win);
}

/*
 * @brief: Set image to given part
 * @param[parent]: Object has part to which you want to set this image
 * @param[part_name]: Part name to which you want to set this image
 * @param[image_path]: Path of the image file
 */
void view_set_image(Evas_Object *parent, const char *part_name, const char *image_path)
{
	Evas_Object *image = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	image = elm_object_part_content_get(parent, part_name);
	if (image == NULL) {
		image = elm_image_add(parent);
		if (image == NULL) {
			dlog_print(DLOG_ERROR, LOG_TAG, "failed to create an image object.");
			return;
		}

		elm_object_part_content_set(parent, part_name, image);
	}

	if (EINA_FALSE == elm_image_file_set(image, image_path, NULL)) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to set image.");
		return;
	}

	evas_object_show(image);

	return;
}

/*
 * @brief: Set text to the part
 * @param[parent]: Object has part to which you want to set text
 * @param[part_name]: Part name to which you want to set the text
 * @param[text]: Text you want to set to the part
 */
void view_set_text(Evas_Object *parent, const char *part_name, const char *text)
{
	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	/* Set text of target part object */
	elm_object_part_text_set(parent, part_name, text);
}

/*
 * @brief: Set color of the part
 * @param[parent]: Object has part to which you want to set color
 * @param[part_name]: Name of part to which you want to set color
 * @param[r]: R of RGBA you want to set to the part
 * @param[g]: G of RGBA you want to set to the part
 * @param[b]: B of RGBA you want to set to the part
 * @param[a]: A of RGBA you want to set to the part
 */
void view_set_color(Evas_Object *parent, const char *part_name, int r, int g, int b, int a)
{
	Evas_Object *obj = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	obj = elm_object_part_content_get(parent, part_name);
	if (obj == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get parent.");
		return;
	}

	/* Set color of target part object */
	evas_object_color_set(obj, r, g, b, a);
}

/*
 * @brief: Set the color of circle line and font in a given circle object
 * @param[parent]: Object has part to which you want to set color
 * @param[part_name]: Name of part to which you want to set color
 * @param[r] The red component of the given color
 * @param[g] The green component of the given color
 * @param[b] The blue component of the given color
 * @param[a] The alpha component of the given color
 */
void view_set_color_of_circle_object(Evas_Object *parent, const char *part_name, int r, int g, int b, int a)
{
	Evas_Object *obj = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	/* if the "part" is NULL, this function will get the content from "default" part */
	obj = elm_object_part_content_get(parent, part_name);
	if (obj == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to get parent.");
		return;
	}

	/* Set color of target part object */
	eext_circle_object_color_set(obj, r, g, b, a);
}

/*
 * @brief: Make a label and set label options.
 * @param[parent]: The object to which you want to add this label
 */
Evas_Object *view_create_label(Evas_Object *parent)
{
	Evas_Object *label = elm_label_add(parent);

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return NULL;
	}

	elm_object_style_set(label, "slide_short");
	elm_label_wrap_width_set(label, 230);
	elm_label_ellipsis_set(label, EINA_TRUE);
	elm_label_slide_duration_set(label, 2);
	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_NONE);
	elm_label_slide_go(label);
	evas_object_show(label);

	return label;
}

/*
 * @brief: Set a text to label object
 * @param[parent]: Object has part to which you want to set text
 * @param[part_name]: Part name to which you want to set text
 * @param[text]: Text you want to set to the part
 */
void view_set_label_text(Evas_Object *parent, const char *part_name, const char *text)
{
	Evas_Object *label = NULL;
	char *markup_text = NULL;
	char buf[256] = { 0, };

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	markup_text = elm_entry_utf8_to_markup(text);
	snprintf(buf, sizeof(buf), "%s%s%s", LABEL_STYLE_START, markup_text, LABEL_STYLE_END);
	free(markup_text);

	label = elm_object_part_content_get(parent, part_name);
	if (label == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "label is NULL.");
		return;
	}

	elm_object_text_set(label, buf);
}

/*
 * @brief: Set a label to given part
 * @param[parent]: Object has part to which you want to set this label
 * @param[part_name]: Part name to which you want to set this label
 */
void view_set_label(Evas_Object *parent, const char *part_name)
{
	Evas_Object *label = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	label = view_create_label(parent);
	if (label == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "label is NULL.");
		return;
	}

	elm_object_part_content_set(parent, part_name, label);
}

/*
 * @brief: Register rotary event callback function.
 * @param[obj]: Object that will receive rotary event
 * @param[rotary_cb]: Function will be operated when rotary event happens
 * @param[user_data]: Data needed in this function
 */
void view_set_rotary_event_callback(Evas_Object *obj, Eext_Rotary_Event_Cb rotary_cb, void *user_data)
{
	if (obj == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	eext_rotary_object_event_activated_set(obj, EINA_TRUE);
	eext_rotary_object_event_callback_add(obj, rotary_cb, user_data);
}

/*
 * @brief: Create a progressbar
 * @param[parent]: Object has part to which you want to set this progressbar
 * @param[radius]: The radius value of a given circle object
 * @param[line_width]: The line width value of the circle object
 */
Evas_Object *view_create_progressbar(Evas_Object *parent, int radius, int line_width)
{
	Evas_Object *progressbar = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return NULL;
	}

	progressbar = eext_circle_object_progressbar_add(parent, NULL);

	eext_circle_object_value_min_max_set(progressbar, 0.0, 100.0);
	eext_circle_object_radius_set(progressbar, radius);
	eext_circle_object_line_width_set(progressbar, line_width);
	evas_object_show(progressbar);

	return progressbar;
}

/*
 * @brief: Set a progressbar to given part
 * @param[parent]: Object has part to which you want to set
 * @param[part_name]: Part name to which you want to set
 * @param[radius]: The radius value of a given circle object
 * @param[line_width]: The line width value of the circle object
 */
void view_set_progressbar(Evas_Object *parent, const char *part_name, int radius, int line_width)
{
	Evas_Object *progressbar = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	progressbar = view_create_progressbar(parent, radius, line_width);
	if (progressbar == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	elm_object_part_content_set(parent, part_name, progressbar);
}

/*
 * @brief: Set value to a progressbar
 * @param[parent]: Object has part to which you want to set
 * @param[part_name]: Part name to which you want to set
 * @param[val]: Value to which you want to set
 */
void view_set_progressbar_val(Evas_Object *parent, const char *part_name, int val)
{
	Evas_Object *progressbar = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	if (part_name == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "part is NULL.");
		return;
	}

	progressbar = elm_object_part_content_get(parent, part_name);
	if (progressbar == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "progressbar is NULL.");
		return;
	}

	eext_circle_object_value_set(progressbar, val);
}

/*
 * @brief: Make and set button.
 * @param[parent]: Object to which you want to set the button
 * @param[part_name]: Name of part to which you want to set the button
 * @param[style]: Style of the button
 * @param[image_path]: Path of image file will be used as button icon
 * @param[text]: The text will be written on the button
 * @param[down_cb]: Function will be operated when the button is pressed
 * @param[up_cb]: Function will be operated when the button is released
 * @param[clicked_cb]: Function will be operated when the button is clicked
 * @param[user_data]: Data passed to the 'clicked_cb' function
 */
void view_set_button(Evas_Object *parent, const char *part_name, const char *style, const char *image_path, const char *text,
		        Evas_Object_Event_Cb down_cb, Evas_Object_Event_Cb up_cb, Evas_Smart_Cb clicked_cb, void *user_data)
{
	Evas_Object *btn = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	btn = elm_button_add(parent);
	if (btn == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "failed to create button.");
		return;
	}

	if (style)
		elm_object_style_set(btn, style);

	evas_object_size_hint_weight_set(btn, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(parent, part_name, btn);

	if (text)
		elm_object_text_set(btn, text);

	if (image_path)
		view_set_image(btn, NULL, image_path);

	if (down_cb)
		evas_object_event_callback_add(btn , EVAS_CALLBACK_MOUSE_DOWN, down_cb, user_data);
	if (up_cb)
		evas_object_event_callback_add(btn, EVAS_CALLBACK_MOUSE_UP, up_cb, user_data);
	if (clicked_cb)
		evas_object_smart_callback_add(btn, "clicked", clicked_cb, user_data);

	evas_object_show(btn);
}

/*
 * @brief: Set a more button
 * @param[parent]: Object has part to which you want to set
 * @param[part_name]: Part name to which you want to set
 * @param[opened_cb]: Function will be operated when the more button is opened
 * @param[closed_cb]: Function will be operated when the more button is closed
 * @param[seleted_cb]: Function will be operated when the more button item is selected
 * @param[user_data]: Data needed in this function
 */
void view_set_more_button(Evas_Object *parent, const char *part_name, Evas_Smart_Cb opened_cb, Evas_Smart_Cb closed_cb, Evas_Smart_Cb seleted_cb, void *user_data)
{
	Evas_Object *more_btn = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	more_btn = eext_more_option_add(parent);
	if (more_btn == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "more option is NULL.");
		return;
	}

	/* Add smart callback */
	evas_object_smart_callback_add(more_btn, "more,option,opened", opened_cb, user_data);
	evas_object_smart_callback_add(more_btn, "more,option,closed", closed_cb, user_data);
	evas_object_smart_callback_add(more_btn, "item,selected", seleted_cb, user_data);

	elm_object_part_content_set(parent, part_name, more_btn);
}

/*
 * @brief: Add a more button item
 * @param[parent]: Object that contains more button
 * @param[part_name]: Part name to which more button is set
 * @param[main_txt]: Text will be written in the middle of the selector
 * @param[sub_txt] Text will be written under the main_txt
 * @param[image_path]: Path of image file will be used as more button item icon
 * @param[clicked_cb]: Function will be operated when the more button item is clicked
 * @param[user_data]: Data needed in this function
 */
void view_add_more_button_item(Evas_Object *parent, const char *part_name, const char *main_txt, const char *sub_txt, const char *image_path, Evas_Smart_Cb clicked_cb, void *user_data)
{
	Evas_Object *img = NULL;
	Evas_Object *more_btn = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return;
	}

	more_btn = elm_object_part_content_get(parent, part_name);
	if (more_btn == NULL) {
		return;
	}

	/* Create the new item */
	Eext_Object_Item *item = eext_more_option_item_append(more_btn);

	/* Set the text in item text part */
	eext_more_option_item_part_text_set(item, "selector,main_text", main_txt);
	eext_more_option_item_part_text_set(item, "selector,sub_text", sub_txt);

	img = elm_image_add(more_btn);
	elm_image_file_set(img, image_path, NULL);

	/* Set the content in item content part */
	eext_more_option_item_part_content_set(item, "item,icon", img);

	evas_object_smart_callback_add(more_btn, "item,clicked", clicked_cb, user_data);
}

/*
 * @brief: Make a layout to target parent object with edje file
 * @param[parent]: The object to which you want to add this layout
 * @param[file_path]: File path of EDJ file will be used
 * @param[group_name]: Name of group in EDJ you want to set to
 * @param[cb_function]: The function will be called when back event is detected
 * @param[user_data]: The user data to be passed to the callback functions
 */
Evas_Object *view_create_layout(Evas_Object *parent, const char *file_path, const char *group_name, Eext_Event_Cb cb_function, void *user_data)
{
	Evas_Object *layout = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return NULL;
	}

	/* Create layout by EDC(edje file) */
	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, file_path, group_name);

	/* Layout size setting */
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if (cb_function)
		eext_object_event_callback_add(layout, EEXT_CALLBACK_BACK, cb_function, user_data);

	evas_object_show(layout);

	return layout;
}

/*
 * @brief: Make and set a layout to conformant
 * @param[parent]: Target conformant object
 * @param[file_path]: File path of EDJ will be used
 * @param[group_name]: Group name in EDJ you want to set to layout
 * @param[cb_function]: The function will be called when the back event is detected
 * @param[user_data]: The user data to be passed to the callback functions
 */
Evas_Object *view_create_layout_for_conformant(Evas_Object *parent, const char *file_path, const char *group_name, Eext_Event_Cb cb_function, void *user_data)
{
	Evas_Object *layout = NULL;

	if (parent == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "parent is NULL.");
		return NULL;
	}

	/* Create layout for conformant */
	if (file_path == NULL) {
		layout = view_create_layout_by_theme(parent, "layout", "application", "default");
	} else {
		layout = view_create_layout(parent, file_path, group_name, cb_function, user_data);
	}

	if (layout == NULL) {
		dlog_print(DLOG_ERROR, LOG_TAG, "layout is NULL.");
		return NULL;
	}

	elm_object_content_set(parent, layout);

	return layout;
}


/*
 * Below functions are static functions.
 * You don't need to modify them.
 */

/*
 * @brief: Function will be operated when window is deleted
 * @param[user_data]: The data passed to the function
 * @param[obj]: The object, normally window
 * @param[event_info]: Information regarding with the event
 */
static void _win_delete_request_cb(void *user_data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

/*
 * @brief: Set title layout that will be shown
 * @param[parent]: Object has part to which you want to set
 * @param[part_name]: Part name to which you want to set
 * @param[edj_path]: Edje path to which you want to set
 * @param[image_path]: Path of image file you want to set
 * @param[title]: Text you want to set
 */
void view_music_set_title_layout(Evas_Object *parent, const char *part_name, const char *edj_path, const char *image_path, const char *title)
{
	Evas_Object *title_layout = NULL;

	title_layout = elm_object_part_content_get(parent, part_name);
	if (title_layout == NULL) {
		title_layout = view_create_layout(parent, edj_path, "title-layout", NULL, NULL);
		elm_object_part_content_set(parent, part_name, title_layout);
	}

	if (image_path) {
		view_set_image(title_layout, "sw.icon.device", image_path);
		view_set_color(title_layout, "sw.icon.device", 250, 250, 250, 255);
	}

	if (title) {
		Evas_Object *label = elm_object_part_content_get(title_layout, "sw.title");
		if (label == NULL) {
			view_set_label(title_layout, "sw.title");
		}
		view_set_label_text(title_layout, "sw.title", title);
	}
}

/*
 * @brief: Set album art image that will be shown
 * @param[parent]: Object has part to which you want to set
 * @param[part_name]: Part name to which you want to set
 * @param[image_path]: Path of album art file you want to set
 * @param[default_image_path]: Path of default album art file
 */
void view_music_set_album_art(Evas_Object *parent, const char *part_name, const char *image_path, const char *default_image_path)
{
	if (!strcmp(image_path, "NULL")) {
		view_set_image(parent, "sw.bg", default_image_path);
	} else {
		Evas_Object *img = elm_object_part_content_get(parent, part_name);
		if (img == NULL) {
			img = elm_image_add(parent);
			evas_object_size_hint_weight_set(img, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_show(img);
			elm_object_part_content_set(parent, part_name, img);
		}

		elm_image_file_set(img, image_path, NULL);
	}
}

/*
 * @brief: Change a image of more option item
 * @param[item]: Object to which you want to change image
 * @param[main_txt]: Text will be written in the middle of the selector
 * @param[sub_txt]: Text will be written under the main_txt
 * @param[image_path]: Path of image file will be used
 */
void view_music_change_more_item(Eext_Object_Item *item, const char *main_txt, const char *sub_txt, const char *image_path)
{
	Evas_Object *img = NULL;

	/* Set the text in item text part */
	eext_more_option_item_part_text_set(item, "selector,main_text", main_txt);
	eext_more_option_item_part_text_set(item, "selector,sub_text", sub_txt);

	img = eext_more_option_item_part_content_get(item, "item,icon");
	elm_image_file_set(img, image_path, NULL);

	/* Set the content in item content part */
	eext_more_option_item_part_content_set(item, "item,icon", img);
}

