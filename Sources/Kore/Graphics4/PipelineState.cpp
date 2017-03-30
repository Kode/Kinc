#include "pch.h"

#include "PipelineState.h"

using namespace Kore;

Graphics4::PipelineState::PipelineState() : Program() {
	// TODO
	// program = new Program;
}

void Graphics4::PipelineState::compile() {
	link(PipelineStateBase::inputLayout, 0);
}

// ConstantLocation PipelineState::getConstantLocation(const char* name) {
//    return new ConstantLocation(glGetUniformLocation(program, name));
//}
