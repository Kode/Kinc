#pragma once

#include "PipelineStateBase.h"
#include "Shader.h"

namespace Kore {
    class PipelineState : public PipelineStateBase, public Program {
    
    private:
        Program* program;
        //const char** textures;
        //int* textureValues;
        
        void compileShader(Shader* shader);
        //int findTexture(char* name);
    
    public:
        
        PipelineState();
        void compile();
        
        //ConstantLocation getConstantLocation(const char* name);
        //TextureUnit getTextureUnit(char** name);
    
    };
}
