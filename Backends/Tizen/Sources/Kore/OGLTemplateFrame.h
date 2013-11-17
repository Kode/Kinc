#ifndef _OGLTEMPLATEFRAME_H_
#define _OGLTEMPLATEFRAME_H_

#include <FApp.h>
#include <FBase.h>
#include <FSystem.h>
#include <FUi.h>
#include <FUiIme.h>
#include <FGraphics.h>
#include <gl.h>

class OGLTemplateFrame
	: public Tizen::Ui::Controls::Frame
{
public:
	OGLTemplateFrame(void);
	virtual ~OGLTemplateFrame(void);

public:
	virtual result OnInitializing(void);
	virtual result OnTerminating(void);
};

#endif  //_OGLTEMPLATEFRAME_H_
