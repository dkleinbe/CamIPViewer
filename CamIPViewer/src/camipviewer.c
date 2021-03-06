#include "camipviewer.h"
#include "settings.h"
#include "image.h"
#include "video.h"
#include "audio.h"
#include "utils.h"

static char *main_menu_names[] = {
	"Settings", "Image", "Video", "Audio", NULL
};

typedef struct _item_data
{
	int index;
	Elm_Object_Item *item;
} item_data;


static char *
_gl_menu_title_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];

	snprintf(buf, 1023, "%s", "Cam viewer");
	return strdup(buf);
}

static char *
_gl_menu_text_get(void *data, Evas_Object *obj, const char *part)
{
	char buf[1024];
	item_data *id = (item_data *)data;
	int index = id->index;

	if (!strcmp(part, "elm.text")) {
		snprintf(buf, 1023, "%s", main_menu_names[index]);
		return strdup(buf);
	}
	return NULL;
}

static void
_gl_menu_del(void *data, Evas_Object *obj)
{
	// FIXME: Unrealized callback can be called after this.
	// Accessing Item_Data can be dangerous on unrealized callback.
	item_data *id = (item_data *)data;
	if (id) free(id);
}

static void
gl_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *it = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set(it, EINA_FALSE);
}

static void
win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}
#ifdef AZE
static void
layout_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = data;
	/* Let window go to hide state. */
	elm_win_lower(ad->win);
}
#endif /* AZE */
static void
settings_cb(void *data, Evas_Object *obj, void *event_info)
{
	create_settings_list_view(data);
}

static void
image_cb(void *data, Evas_Object *obj, void *event_info)
{
	create_image_view(data);
}

static void
video_cb(void *data, Evas_Object *obj, void *event_info)
{
	create_video_view(data);
}

static void
audio_cb(void *data, Evas_Object *obj, void *event_info)
{
	create_audio_view(data);
}

static Eina_Bool
naviframe_pop_cb(void *data, Elm_Object_Item *it)
{
	ui_app_exit();
	return EINA_FALSE;
}

static void
create_list_view(appdata_s *ad)
{
	Evas_Object *genlist;
	Evas_Object *circle_genlist;
	Evas_Object *btn;
	Evas_Object *nf = ad->naviframe;
	Elm_Object_Item *nf_it;
	Elm_Genlist_Item_Class *itc = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *ttc = elm_genlist_item_class_new();
	Elm_Genlist_Item_Class *ptc = elm_genlist_item_class_new();
	item_data *id;
	int index = 0;

	/* Genlist */
	genlist = elm_genlist_add(nf);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(genlist, "selected", gl_selected_cb, NULL);

	circle_genlist = eext_circle_object_genlist_add(genlist, ad->circle_surface);
	eext_circle_object_genlist_scroller_policy_set(circle_genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	eext_rotary_object_event_activated_set(circle_genlist, EINA_TRUE);

	/* Genlist Title Item style */
	ttc->item_style = "title";
	ttc->func.text_get = _gl_menu_title_text_get;
	ttc->func.del = _gl_menu_del;

	/* Genlist Item style */
	itc->item_style = "default";
	itc->func.text_get = _gl_menu_text_get;
	itc->func.del = _gl_menu_del;

	/* Genlist Padding Item style */
	ptc->item_style = "padding";
	ptc->func.del = _gl_menu_del;

	/* Title Items Here */
	elm_genlist_item_append(genlist, ttc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	/* Main Menu Items Here */
	// settings
	id = calloc(sizeof(item_data), 1);
	id->index = index++;
	id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, settings_cb, ad);
	// image
	id = calloc(sizeof(item_data), 1);
	id->index = index++;
	id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, image_cb, ad);
	// video
	id = calloc(sizeof(item_data), 1);
	id->index = index++;
	id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, video_cb, ad);
	// audio
	id = calloc(sizeof(item_data), 1);
	id->index = index++;
	id->item = elm_genlist_item_append(genlist, itc, id, NULL, ELM_GENLIST_ITEM_NONE, audio_cb, ad);
	// padding
	elm_genlist_item_append(genlist, ptc, NULL, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);

	elm_genlist_item_class_free(itc);
	elm_genlist_item_class_free(ttc);
	elm_genlist_item_class_free(ptc);

	/* This button is set for devices which doesn't have H/W back key. */
	btn = elm_button_add(nf);
	elm_object_style_set(btn, "naviframe/end_btn/default");
	nf_it = elm_naviframe_item_push(nf, NULL, btn, NULL, genlist, "empty");
	elm_naviframe_item_pop_cb_set(nf_it, naviframe_pop_cb, ad->win);
}

/*
 * @brief Create base gui structure
 * @param[in] ad The data structureto manage gui object
 */
static void
create_base_gui(appdata_s *ad)
{
	//char edj_path[PATH_MAX] = {0, };
	/*
	 * - win
	 *   - conformant
	 *     - layout
	 *       - circle_surface
	 *         - list view
	 */
	/* Window */
	/* Create and initialize elm_win.
	   elm_win is mandatory to manipulate window. */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_conformant_set(ad->win, EINA_TRUE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", win_delete_request_cb, NULL);

	/* Conformant */
	/* Create and initialize elm_conformant.
	   elm_conformant is mandatory for base gui to have proper size
	   when indicator or virtual keypad is visible. */
	ad->conform = elm_conformant_add(ad->win);
	//elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	//elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

#ifdef AAA
	// Eext Circle Surface Creation
	ad->circle_surface = eext_circle_surface_conformant_add(ad->conform);

	/* Base Layout */
	/* Create an actual view of the base gui.
	   Modify this part to change the view. */
	app_get_resource(EDJ_FILE, edj_path, (int)PATH_MAX);
	ad->layout = elm_layout_add(ad->conform);
	elm_layout_file_set(ad->layout, edj_path, GRP_MAIN);
	evas_object_size_hint_weight_set(ad->layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_layout_theme_set(ad->layout, "layout", "application", "default");
	//eext_object_event_callback_add(ad->layout, EEXT_CALLBACK_BACK, layout_back_cb, ad);
	evas_object_show(ad->layout);
	elm_object_content_set(ad->conform, ad->layout);

	/* Naviframe */
	ad->naviframe = elm_naviframe_add(ad->layout);
#endif

	/* Naviframe */
	ad->naviframe = elm_naviframe_add(ad->conform);
	evas_object_show(ad->naviframe);
	elm_object_content_set(ad->conform, ad->naviframe);

	/* Eext Circle Surface*/
	ad->circle_surface = eext_circle_surface_naviframe_add(ad->naviframe);

	/* Create main menu list */
	create_list_view(ad);
	//create_image_view(ad);

	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(ad->naviframe, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);


	/* Show window after base gui is set up */
	evas_object_show(ad->win);

}
/**
 * Hook to take necessary actions before main event loop starts
 *  Initialize UI resources and application's data
 *  If this function returns true, the main loop of application starts
 *  If this function returns false, the application is terminated
 *
 * @param data
 * @return
 */
static bool
_app_create_cb(void *data)
{

	appdata_s *ad = data;
	ad->global_cleanup_needed = false;

	//
	// Create base UI
	//
	create_base_gui(ad);
	//
    // Initialize libcurl
    // Should be done when there is only one thread running in the app,
    // as the function is not thread safe.
	//
    CURLcode error_code = curl_global_init(CURL_GLOBAL_ALL);
    if (CURLE_OK != error_code)
    {
        EINA_LOG_ERR("cURL global initialization failed: %s", curl_easy_strerror(error_code));
        return false;
    }

    ad->global_cleanup_needed = true;

    eina_lock_new(&ad->mutex);
    ad->thread_running = false;
    ad->exiting = false;


	return true;
}

static void
app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void
app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void
app_resume(void *data)
{
	/* Take necessary actions when application becomes visible. */
}

static void
_app_terminate_cb(void *data)
{
	/* Release all resources. */
    appdata_s *ad = data;

    ad->exiting = true;
    ad->cancel_requested = true;

    EINA_LOG_DBG("app_terminate(): taking lock");
    eina_lock_take(&ad->mutex);
    EINA_LOG_DBG("app_terminate(): lock taken");

    if (ad->global_cleanup_needed && !ad->thread_running)
    {
        curl_global_cleanup();
        ad->global_cleanup_needed = false;
        EINA_LOG_DBG("app_terminate(): curl_global_cleanup() called");
    }

    EINA_LOG_DBG("app_terminate(): freeing lock");
    eina_lock_release(&ad->mutex);

    utils_cleanup();
}

static void
ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LANGUAGE_CHANGED*/

	int ret;
	char *language;

	ret = app_event_get_language(event_info, &language);
	if (ret != APP_ERROR_NONE) {
		EINA_LOG_ERR("app_event_get_language() failed. Err = %d.", ret);
		return;
	}

	if (language != NULL) {
		elm_language_set(language);
		free(language);
	}
}

static void
ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_DEVICE_ORIENTATION_CHANGED*/
	return;
}

static void
ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_REGION_FORMAT_CHANGED*/
}

static void
ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_BATTERY*/
}

static void
ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/*APP_EVENT_LOW_MEMORY*/
}

int
main(int argc, char *argv[])
{
	appdata_s ad = {0,};
	int ret = 0;

	utils_init();

	memset(&ad, 0x00, sizeof(appdata_s));

	ui_app_lifecycle_callback_s event_callback = {0,};
	app_event_handler_h handlers[5] = {NULL, };

	EINA_LOG_INFO("ENTERING main() PID: <%d>", getpid());

	event_callback.create = _app_create_cb;
	event_callback.terminate = _app_terminate_cb;
	event_callback.pause = app_pause;
	event_callback.resume = app_resume;
	event_callback.app_control = app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, ui_app_region_changed, &ad);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		EINA_LOG_ERR("ui_app_main() is failed. err = %d", ret);
	}

	utils_cleanup();

	return ret;
}
