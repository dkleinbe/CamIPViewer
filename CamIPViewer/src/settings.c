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


App_setting _app_settings2[] = {
	{
		.key = "setting.cam.ip",
		.name = "Cam IP",
		.input_panel_layout = ELM_INPUT_PANEL_LAYOUT_IP,
		.guide = "192.168.255.255",
		.value = NULL
	},
	{
		.key = "setting.cam.port",
		.name = "Port",
		.input_panel_layout = ELM_INPUT_PANEL_LAYOUT_IP,
		.guide = "1234",
		.value = NULL
	},
	{
		.key = "setting.cam.user",
		.name = "User",
		.input_panel_layout = ELM_INPUT_PANEL_LAYOUT_NORMAL,
		.guide = "John",
		.value = NULL
	},
	{
		.key = "setting.cam.passord",
		.name = "Password",
		.input_panel_layout = ELM_INPUT_PANEL_LAYOUT_PASSWORD,
		.guide = "toto",
		.value = NULL
	}
};

typedef struct _item_data {
	int index;
	Elm_Object_Item *item;
} item_data;

static Evas_Object *_app_naviframe;
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
	item_data *id = data;
	int index = id->index;

	if (!strcmp(part, "elm.text"))
	{
		return strdup(_app_settings2[index].name);
	}
	if (!strcmp(part, "elm.text.1") && _app_settings2[index].value)
	{
		return strdup(_app_settings2[index].value);
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

static bool
load_setting(int index)
{
	bool is_existing = false;
	char *value;

	preference_is_existing(_app_settings2[index].key, &is_existing);
	if (is_existing)
	{
		preference_get_string(_app_settings2[index].key, &value);
		if (_app_settings2[index].value != NULL)
		{
			free(_app_settings2[index].value);
		}
		_app_settings2[index].value = strdup(value);
		return true;
	}
	else
	{
		_app_settings2[index].value = NULL;
		return false;
	}

	return false;
}
/*
 *
 */
static void
_entry_setting_enter_click(void *data, Evas_Object *obj, void *event_info)
{
	int setting_index = -1;
	item_data *id;

	/* get item data */
	id = (item_data *)elm_object_item_data_get((Elm_Object_Item *)data);
	setting_index = id->index;

	const char *txt = elm_entry_entry_get(obj);

	preference_set_string(_app_settings2[setting_index].key, txt);
	/* reload setting */
	load_setting(setting_index);
	elm_genlist_item_fields_update(id->item, "elm.text.1", ELM_GENLIST_ITEM_FIELD_TEXT);

	elm_naviframe_item_pop(_app_naviframe);

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
 * @brief Function will be operated when "Cam IP" menu is clicked
 * @param[in] data The data to be passed to the callback function
 * @param[in] obj The Evas object handle to be passed to the callback function
 * @param[in] event_info The system event information
 */
static void
_setting_item_cb(void *data, Evas_Object *obj, void *event_info EINA_UNUSED)
{
	char edj_path[PATH_MAX] = {0, };
	appdata_s *ad = data;
	item_data *id = NULL;
	Evas_Object *naviframe = ad->naviframe;
	Evas_Object *layout = NULL;
	Evas_Object *entry = NULL;
	Evas_Object *scroller = NULL;
	Elm_Object_Item *nf_it = NULL;
	Elm_Object_Item *setting_item = (Elm_Object_Item *)event_info;
	int setting_index = -1;
	static Elm_Entry_Filter_Limit_Size limit_filter_data;

	/* Unhighlight Item */
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info, EINA_FALSE);
	/* get item data */
	id = (item_data *)elm_object_item_data_get(setting_item);
	setting_index = id->index;

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
	elm_entry_input_panel_layout_set(entry, _app_settings2[setting_index].input_panel_layout);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

	elm_object_part_text_set(entry, "elm.guide", _app_settings2[setting_index].guide);
	elm_entry_cursor_end_set(entry);

	elm_entry_entry_set(entry, _app_settings2[setting_index].value);

	evas_object_size_hint_weight_set(entry, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(entry, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(entry, "activated", _entry_setting_enter_click, setting_item);

	elm_object_part_content_set(layout, "entry_part", entry);
	elm_object_content_set(scroller, layout);

	nf_it = elm_naviframe_item_push(naviframe, _("Single line entry"), NULL, NULL, scroller, "empty");
	elm_naviframe_item_pop_cb_set(nf_it, _setting_finished_cb, scroller);
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
	Elm_Object_Item *nf_it = NULL;
	Elm_Genlist_Item_Class *itc =  elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *titc = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *pitc = elm_genlist_item_class_new();
	item_data *id = NULL;
	int index = 0;

	_app_naviframe = ad->naviframe;
	/* Create Genlist */
	genlist = elm_genlist_add(_app_naviframe);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(genlist, "selected", NULL, NULL);

	/* Create Circle Genlist */
	circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);

	/* Set Scroller Policy */
	eext_circle_object_genlist_scroller_policy_set(circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	/* Activate Rotary Event */
	eext_rotary_object_event_activated_set(circle_genlist, EINA_TRUE);

	/* Genlist Item Style */
	itc->item_style = "2text";
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
	for (index = 0 ; index < NB_SETTINGS; ++index)
	{
		/*
		 * load setting
		 */
		load_setting(index);

		id = calloc(sizeof(item_data), 1);
		id->index = index;
		id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, _setting_item_cb, ad);
	}

	/* Padding Item Here */
	elm_genlist_item_append(genlist, pitc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, ad);

	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(titc);
	elm_genlist_item_class_free(pitc);

	nf_it = elm_naviframe_item_push(_app_naviframe, NULL, NULL, NULL, genlist, "empty");
	elm_naviframe_item_pop_cb_set(nf_it, _naviframe_pop_cb, genlist);
}

