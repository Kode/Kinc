#include "GlRendererTemplate.h"
#include "pch.h"
#include <Kore/Application.h>

GlRendererTemplate::GlRendererTemplate(void)
	: __controlWidth(0)
	, __controlHeight(0)
	, __angle(0)
{
}

GlRendererTemplate::~GlRendererTemplate(void)
{

}

extern int kore(int argc, char** argv);

bool
GlRendererTemplate::InitializeGl(void)
{
	kore(0, nullptr);
	return true;
}

bool
GlRendererTemplate::TerminateGl(void)
{
	// TODO:
	// Terminate and reset GL status. 
	return true;
}

bool
GlRendererTemplate::Draw(void)
{
	Kore::Application::the()->callback();
	return true;
}

bool
GlRendererTemplate::Pause(void)
{
	// TODO:
	// Do something necessary when Plyaer is paused. 

	return true;
}

bool
GlRendererTemplate::Resume(void)
{
	// TODO:
	// Do something necessary when Plyaer is resumed. 

	return true;
}

int
GlRendererTemplate::GetTargetControlWidth(void)
{
	// TODO:
	// Return target control width

	return __controlWidth;
}

int
GlRendererTemplate::GetTargetControlHeight(void)
{
	// TODO:
	// Return target control height

	return __controlHeight;
}

void
GlRendererTemplate::SetTargetControlWidth(int width)
{
	// TODO:
	// Assign target control width

	__controlWidth = width;
}

void
GlRendererTemplate::SetTargetControlHeight(int height)
{
	// TODO:
	// Assign target control height

	__controlHeight = height;
}
