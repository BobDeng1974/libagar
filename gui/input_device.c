/*	Public domain	*/
/*
 * Abstract Input Device Class.
 * Parent class to AG_Mouse and AG_Keyboard.
 */

#include <agar/core/core.h>
#include <agar/gui/window.h>

AG_Object agInputDevices;		/* Input devices VFS */

static void
Init(void *obj)
{
	AG_InputDevice *id = obj;

	id->flags = 0;
	id->drv = NULL;
	id->desc = NULL;
	AG_InitEventQ(&id->events);
}

static void
Destroy(void *obj)
{
	AG_InputDevice *id = obj;

	Free(id->desc);
	AG_FreeEventQ(&id->events);
}

AG_ObjectClass agInputDeviceClass = {
	"AG_InputDevice",
	sizeof(AG_InputDevice),
	{ 0,0 },
	Init,
	NULL,		/* reset */
	Destroy,
	NULL,		/* load */
	NULL,		/* save */
	NULL		/* edit */
};
