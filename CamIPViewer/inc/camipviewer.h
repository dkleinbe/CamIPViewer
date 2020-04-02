#ifndef __camipviewer_H__
#define __camipviewer_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>
#include <net_connection.h>
#include <curl/curl.h>

#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "0CAMPIPVIEWER"

#if !defined(PACKAGE)
#define PACKAGE "org.example.camipviewer"
#endif

#define EDJ_FILE "edje/camipviewer.edj"
#define GRP_MAIN "main"

typedef struct appdata{
	Ecore_Thread *thread;
	Evas_Object* win;
	Evas_Object* layout;
	Evas_Object* conform;
	Evas_Object* naviframe;
	Evas_Object *image_jpg;
	Eext_Circle_Surface *circle_surface;

	connection_h connection;
	CURL *curl;
	Eina_Lock mutex;

	bool downloading;
	bool cancel_requested;
	bool thread_running;
	bool global_cleanup_needed;
	bool cleanup_done;
	bool exiting;

} appdata_s;


#endif /* __camipviewer_H__ */
