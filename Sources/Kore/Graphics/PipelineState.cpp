#include "pch.h"

#include "PipelineState.h"

using namespace Kore;

PipelineState::PipelineState() : Program() {
	// TODO
	// program = new Program;
}

void PipelineState::compile() {
	link(PipelineStateBase::inputLayout, 0);
}

// ConstantLocation PipelineState::getConstantLocation(const char* name) {
//    return new ConstantLocation(glGetUniformLocation(program, name));
//}
