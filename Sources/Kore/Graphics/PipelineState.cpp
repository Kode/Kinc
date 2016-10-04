#include "pch.h"

#include "PipelineState.h"

using namespace Kore;

PipelineState::PipelineState() : Program() {
    // TODO
}


void PipelineState::compile() {
    //compileShader(vertexShader);
    //compileShader(fragmentShader);
    
}

void PipelineState::compileShader(Kore::Shader *shader) {
    if (shader != nullptr) return;
    // TODO
}

//ConstantLocation PipelineState::getConstantLocation(const char* name) {
//    return new ConstantLocation(glGetUniformLocation(program, name));
//}
