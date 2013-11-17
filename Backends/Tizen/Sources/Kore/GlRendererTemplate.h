#ifndef _GLRENDERERTEMPLATE_H_
#define _GLRENDERERTEMPLATE_H_

#include <FApp.h>
#include <FBase.h>
#include <FSystem.h>
#include <FUi.h>
#include <FUiIme.h>
#include <FGraphics.h>
#include <gl.h>
#include <FGrpIGlRenderer.h>

class GlRendererTemplate :
	public Tizen::Graphics::Opengl::IGlRenderer
{
public:
	GlRendererTemplate(void);
	~GlRendererTemplate(void);

	virtual bool InitializeGl(void);
	virtual bool TerminateGl(void);

	virtual bool Draw(void);

	virtual bool Pause(void);
	virtual bool Resume(void);

	virtual int GetTargetControlWidth(void);
	virtual int GetTargetControlHeight(void);
	virtual void SetTargetControlWidth(int width);
	virtual void SetTargetControlHeight(int height);

private:
	int __controlWidth;
	int __controlHeight;
	int __angle;
};

#endif /* _GLRENDERERTEMPLATE_H_ */
