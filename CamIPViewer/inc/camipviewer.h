#ifndef __camipviewer_H__
#define __camipviewer_H__

#include <app.h>
#include <Elementary.h>
#include <system_settings.h>
#include <efl_extension.h>
#include <dlog.h>

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
	Evas_Object* win;
	Evas_Object* layout;
	Evas_Object* conform;
	Evas_Object* naviframe;
	Eext_Circle_Surface *circle_surface;
} appdata_s;


#endif /* __camipviewer_H__ */
