/*
 * settings.c
 *
 *  Created on: Jan 13, 2019
 *      Author: denis
 */
// Functions using between application restarts for loading and saving values by its keys
#include <app_preference.h>
#include "settings.h"
#include "utils.h"

#define SETTING_CAM_IP "setting.cam.ip"
#define SETTING_CAM_PORT "setting.cam.port"
#define CAM_IP_STRLEN 256
#define CAM_PORT_STRLEN 256

typedef struct _App_settings
{
	char cam_ip[CAM_IP_STRLEN];
	char cam_port[CAM_PORT_STRLEN];
} App_settings;

char *main_menu_names[] = {
	"Cam IP", "Port", "User", "Password",
	NULL
};

static App_settings _app_settings;

/*
 * @brief Function to get string on genlist title item's text part
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] part The name of text part
 * @param[out] char* A string with the characters to use as genlist title item's text part
 */
static char*
_gl_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];

	snprintf(buf, 1023, "%s", "Setting");

	return strdup(buf);
}

typedef struct _item_data {
	int index;
	Elm_Object_Item *item;
} item_data;

/*
 * @brief Function to get string on genlist item's text part
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] part The name of text part
 * @param[out] char* A string with the characters to use as genlist item's text part
 */
static char *
_gl_main_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	item_data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, 1023, "%s", main_menu_names[index]);
		return strdup(buf);
	}
	return NULL;
}

/*
 * @brief Function will be operated when genlist is deleted.
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 */
static void _gl_del(void *data, Evas_Object *obj)
{
	// FIXME: Unrealized callback can be called after this.
	// Accessing Item_Data can be dangerous on unrealized callback.
	item_data *id = data;
	if (id) free(id);
}

static void
_entry_ip_enter_click(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = (Evas_Object *)data;

	const char *txt = elm_entry_entry_get(obj);

	preference_set_string(SETTING_CAM_IP, txt);
	//evas_object_hide(genlist);
	elm_naviframe_item_pop(genlist);

}

static void
_entry_port_enter_click(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = (Evas_Object *)data;

	const char *txt = elm_entry_entry_get(obj);

	preference_set_string(SETTING_CAM_PORT, txt);
	//evas_object_hide(genlist);
	elm_naviframe_item_pop(genlist);

}

/*
 * @brief Function will be operated when manipulating setting value is finished
 * @param[in] data The data to be passed to the callback function
 * @param[in] it The item to be popped from naviframe
 */
static Eina_Bool
_setting_finished_cb(void *data, Elm_Object_Item *it)
{
	Evas_Object *genlist = (Evas_Object *)data;

	evas_object_hide(genlist);
	//elm_naviframe_item_pop(genlist);
	return EINA_TRUE;
}

/*
 *
 */
static bool
cam_ip_load()
{
	bool is_existing = false;
	char *ip;

	preference_is_existing(SETTING_CAM_IP, &is_existing);
	if (is_existing)
	{
		preference_get_string(SETTING_CAM_IP, &ip);
		strcpy(_app_settings.cam_ip, ip);
	}
	else
	{
		_app_settings.cam_ip[0] = 0x0;
		return false;
	}


	return true;
}
/*
 * @brief Function will be operated when "Cam IP" menu is clicked
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void
_setting_cam_ip_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
	char edj_path[PATH_MAX] = {0, };
	appdata_s *ad = data;
	Evas_Object *naviframe = ad->naviframe;
	Evas_Object *layout = NULL;
	Evas_Object *entry = NULL;
	Evas_Object *scroller = NULL;
	Elm_Object_Item *nf_it = NULL;
	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	/* Unhighlight Item */
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);
	scroller = elm_scroller_add(naviframe);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(scroller);
	if (! elm_layout_file_set(layout, edj_path, "entry_layout"))
	{
		dlog_print(DLOG_ERROR, "SETTINGS_UI", "Nananan");
	}
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, 0.0);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, 0.0);

	entry = elm_entry_add(layout);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_scroller_policy_set(entry, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	//evas_object_smart_callback_add(entry, "maxlength,reached", maxlength_reached, nf);

	limit_filter_data.max_char_count = 0;
	limit_filter_data.max_byte_count = 100;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_IP);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

	elm_object_part_text_set(entry, "elm.guide", "192.168.255.255");
	elm_entry_cursor_end_set(entry);
	/*
	 * load cap ip setting
	 */
	cam_ip_load();
	elm_entry_entry_set(entry, _app_settings.cam_ip);

	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(entry, "activated", _entry_ip_enter_click, naviframe);

	elm_object_part_content_set(layout, "entry_part", entry);
	elm_object_content_set(scroller, layout);

	nf_it = elm_naviframe_item_push(naviframe, _("Single line entry"), NULL, NULL, scroller, "empty");
	elm_naviframe_item_pop_cb_set(nf_it, _setting_finished_cb, scroller);
}
/*
 *
 */
static bool
cam_port_load()
{
	bool is_existing = false;
	char *port;

	preference_is_existing(SETTING_CAM_PORT, &is_existing);
	if (is_existing)
	{
		preference_get_string(SETTING_CAM_PORT, &port);
		strcpy(_app_settings.cam_port, port);
	}
	else
	{
		_app_settings.cam_port[0] = 0x0;
		return false;
	}


	return true;
}
/*
 * @brief Function will be operated when "Port" menu is clicked
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void
_setting_cam_ip_port_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
	char edj_path[PATH_MAX] = {0, };
	appdata_s *ad = data;
	Evas_Object *naviframe = ad->naviframe;
	Evas_Object *layout = NULL;
	Evas_Object *entry = NULL;
	Evas_Object *scroller = NULL;
	Elm_Object_Item *nf_it = NULL;
	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	/* Unhighlight Item */
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);

	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);
	scroller = elm_scroller_add(naviframe);
	evas_object_size_hint_align_set(scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = elm_layout_add(scroller);
	if (! elm_layout_file_set(layout, edj_path, "entry_layout"))
	{
		dlog_print(DLOG_ERROR, "SETTINGS_UI", "Nananan");
	}
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, 0.0);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, 0.0);

	entry = elm_entry_add(layout);
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_scroller_policy_set(entry, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	//evas_object_smart_callback_add(entry, "maxlength,reached", maxlength_reached, nf);

	limit_filter_data.max_char_count = 0;
	limit_filter_data.max_byte_count = 100;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_IP);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

	elm_object_part_text_set(entry, "elm.guide", "437");
	elm_entry_cursor_end_set(entry);
	/*
	 * load port ip setting
	 */
	cam_port_load();
	elm_entry_entry_set(entry, _app_settings.cam_port);

	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(entry, "activated", _entry_port_enter_click, naviframe);

	elm_object_part_content_set(layout, "entry_part", entry);
	elm_object_content_set(scroller, layout);

	nf_it = elm_naviframe_item_push(naviframe, _("Single line entry"), NULL, NULL, scroller, "empty");
	elm_naviframe_item_pop_cb_set(nf_it, _setting_finished_cb, scroller);
}

/*
 * @brief Function will be operated when "Cam IP" menu is clicked
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void
_setting_cam_users_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{

}

/*
 * @brief Function will be operated when "Cam IP" menu is clicked
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void
_setting_cam_passwd_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{

}

/*
 * @brief Function will be operated when naviframe pop its own item
 * @param[in] data The data to be passed to the callback function
 * @param[in] it The item to be popped from naviframe
 */
static Eina_Bool
_naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	Evas_Object *genlist = (Evas_Object *)data;

	//evas_object_hide(genlist);
	elm_naviframe_item_pop(genlist);
	return EINA_TRUE;
}

/*
 * @brief Function to create gui object
 * @param[in] ad The data structure to manage gui object
 */
void
create_settings_list_view(appdata_s *ad)
{
	Evas_Object *genlist = NULL;
	Evas_Object *circle_genlist;
	Evas_Object *naviframe = ad->naviframe;
	Elm_Object_Item *nf_it = NULL;
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *titc = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *pitc = elm_genlist_item_class_new();
	item_data *id = NULL;
	int index = 0;


	/* Create Genlist */
	genlist = elm_genlist_add(naviframe);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(genlist, "selected", NULL, NULL);

	/* Create Circle Genlist */
	circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);

	/* Set Scroller Policy */
	eext_circle_object_genlist_scroller_policy_set(circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	/* Activate Rotary Event */
	eext_rotary_object_event_activated_set(circle_genlist, EINA_TRUE);

	/* Genlist Item Style */
	itc->item_style = "default";
	itc->func.text_get = _gl_main_text_get;
	itc->func.del = _gl_del;

	/* Genlist Title Item Style */
	titc->item_style = "title";
	titc->func.text_get = _gl_title_text_get;
	titc->func.del = _gl_del;

	/* Genlist Padding Item Style */
	pitc->item_style = "padding";

	/* Title Item Here */
	elm_genlist_item_append(genlist, titc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, ad);

	/* Main Menu Items Here */
	id = calloc(sizeof(item_data), 1);
	id->index = index++;
	id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _setting_cam_ip_cb, ad);
	id = calloc(sizeof(item_data), 1);
	id->index = index++;
	id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _setting_cam_ip_port_cb, ad);
	id = calloc(sizeof(item_data), 1);
	id->index = index++;
	id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _setting_cam_users_cb, ad);
	id = calloc(sizeof(item_data), 1);
	id->index = index++;
	id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _setting_cam_passwd_cb, ad);

	/* Padding Item Here */
	elm_genlist_item_append(genlist, pitc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, ad);

	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(titc);
	elm_genlist_item_class_free(pitc);

	nf_it = elm_naviframe_item_push(naviframe, NULL, NULL, NULL, genlist, "empty");
	elm_naviframe_item_pop_cb_set(nf_it, _naviframe_pop_cb, genlist);
}

