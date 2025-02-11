#include "metalunit.h"

#include <kope/graphics5/device.h>

#include <assert.h>

#import <MetalKit/MTKView.h>

CAMetalLayer *getMetalLayer(void);

#include "buffer.m"
#include "commandlist.m"
#include "descriptorset.m"
#include "device.m"
#include "pipeline.m"
#include "sampler.m"
#include "texture.m"
