#pragma once

#include "VertexStructure.h"
#include "Shader.h"
#include "Graphics.h"

namespace Kore {
    class PipelineStateBase {
    public:
        VertexStructure** inputLayout;
        /*Shader* vertexShader;
        Shader* fragmentShader;
        Shader* geometryShader;
        Shader* tesselationControlShader;
        Shader* tesselationEvaluationShader;*/
    
        CullMode cullMode;
    
        bool depthWrite;
        ZCompareMode depthMode;
    
        ZCompareMode stencilMode;
        StencilAction stencilBothPass;
        StencilAction stencilDepthFail;
        StencilAction stencilFail;
        int stencilReferenceValue;
        int stencilReadMask;
        int stencilWriteMask;
    
        // One, Zero deactivates blending
        // TODO: BlendingFactor is BlencingOperation?
        BlendingOperation blendSource;
        BlendingOperation blendDestination;
        //BlendingOperation blendOperation;
        BlendingOperation alphaBlendSource;
        BlendingOperation alphaBlendDestination;
        //BlendingOperation alphaBlendOperation;
    
        bool colorWriteMask;
        bool colorWriteMaskRed;
        bool colorWriteMaskGreen;
        bool colorWriteMaskBlue;
        bool colorWriteMaskAlpha;
    
        PipelineStateBase() {
            inputLayout = nullptr;
            //vertexShader = nullptr;
            //fragmentShader = nullptr;
            //geometryShader = nullptr;
            //tesselationControlShader = nullptr;
            //tesselationEvaluationShader = nullptr;
            
            //cullMode = None;
            
            depthWrite = false;
            depthMode = ZCompareAlways;
            
            stencilMode = ZCompareAlways;
            stencilBothPass = Keep;
            stencilDepthFail = Keep;
            stencilFail = Keep;
            stencilReferenceValue = 0;
            stencilReadMask = 0xff;
            stencilWriteMask = 0xff;
            
            blendSource = BlendOne;
            blendDestination = BlendZero;
            //blendOperation = Add;
            alphaBlendSource = BlendOne;
            alphaBlendDestination = BlendZero;
            //alphaBlendOperation = Add;
            
            colorWriteMask = true;
        }
        
        inline bool set_colorWriteMask(bool value) {
            return colorWriteMaskRed = colorWriteMaskBlue = colorWriteMaskGreen = colorWriteMaskAlpha = value;
        }
    
    };
}
