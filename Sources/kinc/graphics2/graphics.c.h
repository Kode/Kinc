#include "graphics.h"

#include <kinc/image.h>
#include <kinc/math/matrix.h>
#include <kinc/video.h>

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

/*typedef struct internal_pipeline {
    PipelineState *pipeline;
    ConstantLocation projectionLocation;
    TextureUnit textureLocation;
} internal_pipeline;

void init_internal_pipeline(internal_pipeline *internal_pipe, PipelineState *pipeline, ConstantLocation projectionLocation, TextureUnit textureLocation) {
    internal_pipe->pipeline = pipeline;
    internal_pipe->projectionLocation = projectionLocation;
    internal_pipe->textureLocation = textureLocation;
}

interface PipelineCache {
    function get(colorFormat : Array<TextureFormat>, depthStencilFormat : DepthStencilFormat) : InternalPipeline;
}

class SimplePipelineCache implements PipelineCache {
    var pipeline : InternalPipeline;

public
    function new(pipeline : PipelineState, texture : Bool) {
        var projectionLocation : ConstantLocation = null;
        try {
            projectionLocation = pipeline.getConstantLocation("projectionMatrix");
        } catch (x : Dynamic) {
            trace(x);
        }

        var textureLocation : TextureUnit = null;
        if (texture) {
            try {
                textureLocation = pipeline.getTextureUnit("tex");
            } catch (x : Dynamic) {
                trace(x);
            }
        }

        this.pipeline = new InternalPipeline(pipeline, projectionLocation, textureLocation);
    }

public
    function get(colorFormats : Array<TextureFormat>, depthStencilFormat : DepthStencilFormat) : InternalPipeline {
        return pipeline;
    }
}

class PerFramebufferPipelineCache implements PipelineCache {
    var pipelines : Array<InternalPipeline> = [];

public
    function new(pipeline : PipelineState, texture : Bool) {
        pipeline.compile();

        var projectionLocation : ConstantLocation = null;
        try {
            projectionLocation = pipeline.getConstantLocation("projectionMatrix");
        } catch (x : Dynamic) {
            trace(x);
        }

        var textureLocation : TextureUnit = null;
        if (texture) {
            try {
                textureLocation = pipeline.getTextureUnit("tex");
            } catch (x : Dynamic) {
                trace(x);
            }
        }

        pipelines.push(new InternalPipeline(pipeline, projectionLocation, textureLocation));
    }

public
    function get(colorFormats : Array<TextureFormat>, depthStencilFormat : DepthStencilFormat) : InternalPipeline {
        return pipelines[hash(colorFormats, depthStencilFormat)];
    }

    function hash(colorFormats : Array<TextureFormat>, depthStencilFormat : DepthStencilFormat) {
        return 0;
    }
}*/

static uint32_t my_color;
// static Font *myFont;
static kinc_matrix4x4_t projection_matrix;

// PipelineState videoPipeline;

static kinc_matrix3x3_t transformations[16];
static uint8_t transformations_size;
static uint8_t transformation_index;
static float opacities[16];
static uint8_t opacities_size;
static int my_font_size;

static void set_projection(void);

void kinc_g2_init(void) {
	transformations[0] = kinc_matrix3x3_identity();
	transformations_size = 1;
	transformation_index = 0;
	opacities[0] = 1;
	opacities_size = 1;
	my_font_size = 12;
	// pipe = NULL;

	my_color = 0xffffffff;
	image_init();
	colored_init();
	// init_g2_text_painter();
	// textPainter.fontSize = fontSize;
	projection_matrix = kinc_matrix4x4_identity();
	set_projection();

	/*if (videoPipeline == null) {
	    videoPipeline = createImagePipeline(createImageVertexStructure());
	    videoPipeline.fragmentShader = Shaders.painter_video_frag;
	    videoPipeline.vertexShader = Shaders.painter_video_vert;
	    videoPipeline.compile();
	}*/
}

static uint32_t upper_power_of_two(uint32_t v) {
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;
	return v;
}

static kinc_matrix4x4_t orthogonal_projection(float left, float right, float bottom, float top, float zn, float zf) {
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	float tz = -(zf + zn) / (zf - zn);

	kinc_matrix4x4_t m = kinc_matrix4x4_identity();
	kinc_matrix4x4_set(&m, 0, 0, 2 / (right - left));
	kinc_matrix4x4_set(&m, 0, 1, 0);
	kinc_matrix4x4_set(&m, 0, 2, 0);
	kinc_matrix4x4_set(&m, 0, 3, 0);
	kinc_matrix4x4_set(&m, 1, 0, 0);
	kinc_matrix4x4_set(&m, 1, 1, 2 / (top - bottom));
	kinc_matrix4x4_set(&m, 1, 2, 0);
	kinc_matrix4x4_set(&m, 1, 3, 0);
	kinc_matrix4x4_set(&m, 2, 0, 0);
	kinc_matrix4x4_set(&m, 2, 1, 0);
	kinc_matrix4x4_set(&m, 2, 2, -2 / (zf - zn));
	kinc_matrix4x4_set(&m, 2, 3, 0);
	kinc_matrix4x4_set(&m, 3, 0, tx);
	kinc_matrix4x4_set(&m, 3, 1, ty);
	kinc_matrix4x4_set(&m, 3, 2, tz);
	kinc_matrix4x4_set(&m, 3, 3, 1);
	return m;
}

#define CANVAS_WIDTH 1024
#define CANVAS_HEIGHT 768

static void set_projection(void) {
	int width = CANVAS_WIDTH;
	int height = CANVAS_HEIGHT;
	// if (Std.isOfType(canvas, Framebuffer)) {
	projection_matrix = orthogonal_projection(0, (float)width, (float)height, 0, 0.1f, 1000.0f);

	//}
	/*else {
	    if (!Image.nonPow2Supported) {
	        width = upperPowerOfTwo(width);
	        height = upperPowerOfTwo(height);
	    }
	    if (Image.renderTargetsInvertedY()) {
	        projectionMatrix.setFrom(FastMatrix4.orthogonalProjection(0, width, 0, height, 0.1, 1000));
	    }
	    else {
	        projectionMatrix.setFrom(FastMatrix4.orthogonalProjection(0, width, height, 0, 0.1, 1000));
	    }
	}*/
	image_set_projection(projection_matrix);
	colored_set_projection(projection_matrix);
	// textPainter.setProjection(projectionMatrix);
}

/*function drawImage(img : kha.Image, x : FastFloat, y : FastFloat) : Void {
    coloredPainter.end();
    textPainter.end();
    var xw : FastFloat = x + img.width;
    var yh : FastFloat = y + img.height;

    var xx = Float32x4.loadFast(x, x, xw, xw);
    var yy = Float32x4.loadFast(yh, y, y, yh);

    var _00 = Float32x4.loadAllFast(transformation._00);
    var _01 = Float32x4.loadAllFast(transformation._01);
    var _02 = Float32x4.loadAllFast(transformation._02);
    var _10 = Float32x4.loadAllFast(transformation._10);
    var _11 = Float32x4.loadAllFast(transformation._11);
    var _12 = Float32x4.loadAllFast(transformation._12);
    var _20 = Float32x4.loadAllFast(transformation._20);
    var _21 = Float32x4.loadAllFast(transformation._21);
    var _22 = Float32x4.loadAllFast(transformation._22);

    // matrix multiply
    var w = Float32x4.add(Float32x4.add(Float32x4.mul(_02, xx), Float32x4.mul(_12, yy)), _22);
    var px = Float32x4.div(Float32x4.add(Float32x4.add(Float32x4.mul(_00, xx), Float32x4.mul(_10, yy)), _20), w);
    var py = Float32x4.div(Float32x4.add(Float32x4.add(Float32x4.mul(_01, xx), Float32x4.mul(_11, yy)), _21), w);

    imagePainter.drawImage(img, Float32x4.get(px, 0), Float32x4.get(py, 0), Float32x4.get(px, 1), Float32x4.get(py, 1), Float32x4.get(px, 2),
                           Float32x4.get(py, 2), Float32x4.get(px, 3), Float32x4.get(py, 3), opacity, this.color);
}*/

static kinc_vector2_t multiply_matrix_vector(kinc_matrix3x3_t m, kinc_vector2_t vec) {
	float w = kinc_matrix3x3_get(&m, 0, 2) * vec.x + kinc_matrix3x3_get(&m, 1, 2) * vec.y + kinc_matrix3x3_get(&m, 2, 2) * 1;

	kinc_vector2_t ret;
	ret.x = (kinc_matrix3x3_get(&m, 0, 0) * vec.x + kinc_matrix3x3_get(&m, 1, 0) * vec.y + kinc_matrix3x3_get(&m, 2, 0) * 1) / w;
	ret.y = (kinc_matrix3x3_get(&m, 0, 1) * vec.x + kinc_matrix3x3_get(&m, 1, 1) * vec.y + kinc_matrix3x3_get(&m, 2, 1) * 1) / w;

	return ret;
}

static kinc_vector2_t kinc_vector2_create(float x, float y) {
	kinc_vector2_t vec;
	vec.x = x;
	vec.y = y;
	return vec;
}

void kinc_g2_draw_image(kinc_g4_texture_t *img, float x, float y) {
	colored_end();
	// textPainter.end();
	float xw = x + img->tex_width;
	float yh = y + img->tex_height;
	kinc_vector2_t p1 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x, yh));
	kinc_vector2_t p2 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x, y));
	kinc_vector2_t p3 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(xw, y));
	kinc_vector2_t p4 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(xw, yh));
	image_draw_image(img, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, kinc_g2_get_opacity(), kinc_g2_get_color());
}

void kinc_g2_draw_scaled_sub_image(kinc_g4_texture_t *img, float sx, float sy, float sw, float sh, float dx, float dy, float dw, float dh) {
	colored_end();
	// textPainter.end();
	kinc_vector2_t p1 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(dx, dy + dh));
	kinc_vector2_t p2 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(dx, dy));
	kinc_vector2_t p3 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(dx + dw, dy));
	kinc_vector2_t p4 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(dx + dw, dy + dh));
	image_draw_image2(img, sx, sy, sw, sh, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y, kinc_g2_get_opacity(), kinc_g2_get_color());
}

void kinc_g2_draw_sub_image(kinc_g4_texture_t *img, float x, float y, float sx, float sy, float sw, float sh) {
	kinc_g2_draw_scaled_sub_image(img, sx, sy, sw, sh, x, y, sw, sh);
}

void kinc_g2_draw_scaled_image(kinc_g4_texture_t *img, float dx, float dy, float dw, float dh) {
	kinc_g2_draw_scaled_sub_image(img, 0, 0, (float)img->tex_width, (float)img->tex_height, dx, dy, dw, dh);
}

uint32_t kinc_g2_get_color(void) {
	return my_color;
}

void kinc_g2_set_color(uint32_t color) {
	my_color = color;
}

void kinc_g2_draw_rect(float x, float y, float width, float height, float strength /*= 1.0*/) {
	image_end();
	// textPainter.end();

	kinc_vector2_t p1 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x - strength / 2, y + strength / 2));         // bottom-left
	kinc_vector2_t p2 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x - strength / 2, y - strength / 2));         // top-left
	kinc_vector2_t p3 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + width + strength / 2, y - strength / 2)); // top-right
	kinc_vector2_t p4 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + width + strength / 2, y + strength / 2)); // bottom-right
	colored_fill_rect(kinc_g2_get_opacity(), kinc_g2_get_color(), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);                             // top

	p1 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x - strength / 2, y + height - strength / 2));
	p2 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x - strength / 2, y + strength / 2));
	p3 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + strength / 2, y + strength / 2));
	p4 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + strength / 2, y + height - strength / 2));
	colored_fill_rect(kinc_g2_get_opacity(), kinc_g2_get_color(), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y); // left

	p1 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x - strength / 2, y + height + strength / 2));
	p2 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x - strength / 2, y + height - strength / 2));
	p3 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + width + strength / 2, y + height - strength / 2));
	p4 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + width + strength / 2, y + height + strength / 2));
	colored_fill_rect(kinc_g2_get_opacity(), kinc_g2_get_color(), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y); // bottom

	p1 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + width - strength / 2, y + height - strength / 2));
	p2 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + width - strength / 2, y + strength / 2));
	p3 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + width + strength / 2, y + strength / 2));
	p4 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + width + strength / 2, y + height - strength / 2));
	colored_fill_rect(kinc_g2_get_opacity(), kinc_g2_get_color(), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y); // right
}

void kinc_g2_fill_rect(float x, float y, float width, float height) {
	image_end();
	// textPainter.end();

	kinc_vector2_t p1 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x, y + height));
	kinc_vector2_t p2 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x, y));
	kinc_vector2_t p3 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + width, y));
	kinc_vector2_t p4 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x + width, y + height));
	colored_fill_rect(kinc_g2_get_opacity(), kinc_g2_get_color(), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y, p4.x, p4.y);
}

/*void draw_string(const char *text, float x, float y) {
    imagePainter.end();
    coloredPainter.end();

    textPainter.drawString(text, opacity, color, x, y, transformation);
}*/

/*void drawCharacters(const char *text, int start, int length, float x, float y) {
    imagePainter.end();
    coloredPainter.end();

    textPainter.drawCharacters(text, start, length, opacity, color, x, y, transformation);
}*/

/*override function get_font() : Font {
    return myFont;
}

override function set_font(font : Font) : Font {
    textPainter.setFont(font);
    return myFont = font;
}

override function set_fontSize(value : Int) : Int {
    return super.fontSize = textPainter.fontSize = value;
}*/

static kinc_vector2_t sub_vectors(kinc_vector2_t a, kinc_vector2_t b) {
	return kinc_vector2_create(a.x - b.x, a.y - b.y);
}

static float get_vector_length(kinc_vector2_t vec) {
	return sqrtf(vec.x * vec.x + vec.y * vec.y);
}

static void set_vector_length(kinc_vector2_t *vec, float length) {
	float currentLength = get_vector_length(*vec);
	if (currentLength == 0) {
		return;
	}
	float mul = length / currentLength;
	vec->x *= mul;
	vec->y *= mul;
}

void kinc_g2_draw_line(float x1, float y1, float x2, float y2, float strength /*= 1.0*/) {
	image_end();
	// textPainter.end();

	kinc_vector2_t vec;
	if (y2 == y1) {
		vec.x = 0.0f;
		vec.y = -1.0f;
	}
	else {
		vec.x = 1.0f;
		vec.y = -(x2 - x1) / (y2 - y1);
	}
	set_vector_length(&vec, strength);
	kinc_vector2_t p1;
	p1.x = x1 + 0.5f * vec.x;
	p1.y = y1 + 0.5f * vec.y;
	kinc_vector2_t p2;
	p2.x = x2 + 0.5f * vec.x;
	p2.y = y2 + 0.5f * vec.y;
	kinc_vector2_t p3 = sub_vectors(p1, vec);
	kinc_vector2_t p4 = sub_vectors(p2, vec);

	p1 = multiply_matrix_vector(kinc_g2_get_transformation(), p1);
	p2 = multiply_matrix_vector(kinc_g2_get_transformation(), p2);
	p3 = multiply_matrix_vector(kinc_g2_get_transformation(), p3);
	p4 = multiply_matrix_vector(kinc_g2_get_transformation(), p4);

	colored_fill_triangle(kinc_g2_get_opacity(), kinc_g2_get_color(), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
	colored_fill_triangle(kinc_g2_get_opacity(), kinc_g2_get_color(), p3.x, p3.y, p2.x, p2.y, p4.x, p4.y);
}

void kinc_g2_fill_triangle(float x1, float y1, float x2, float y2, float x3, float y3) {
	image_end();
	// textPainter.end();

	kinc_vector2_t p1 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x1, y1));
	kinc_vector2_t p2 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x2, y2));
	kinc_vector2_t p3 = multiply_matrix_vector(kinc_g2_get_transformation(), kinc_vector2_create(x3, y3));
	colored_fill_triangle(kinc_g2_get_opacity(), kinc_g2_get_color(), p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
}

static kinc_g2_image_scale_quality myImageScaleQuality = KINC_G2_IMAGE_SCALE_QUALITY_LOW;

kinc_g2_image_scale_quality kinc_g2_get_image_scale_quality(void) {
	return myImageScaleQuality;
}

void kinc_g2_set_image_scale_quality(kinc_g2_image_scale_quality value) {
	if (value == myImageScaleQuality) {
		return;
	}
	image_set_bilinear_filter(value == KINC_G2_IMAGE_SCALE_QUALITY_HIGH);
	// textPainter.setBilinearFilter(value == ImageScaleQuality.High);
	myImageScaleQuality = value;
}

static kinc_g2_image_scale_quality myMipmapScaleQuality = KINC_G2_IMAGE_SCALE_QUALITY_LOW;

kinc_g2_image_scale_quality kinc_g2_get_mipmap_scale_quality(void) {
	return myMipmapScaleQuality;
}

void kinc_g2_set_mipmap_scale_quality(kinc_g2_image_scale_quality value) {
	image_set_bilinear_mipmap_filter(value == KINC_G2_IMAGE_SCALE_QUALITY_HIGH);
	// textPainter.setBilinearMipmapFilter(value == ImageScaleQuality.High); // TODO (DK) implement for fonts as well?
	myMipmapScaleQuality = value;
}

// var pipelineCache = new Map<PipelineState, PipelineCache>();
// var lastPipeline : PipelineState = null;

/*override function setPipeline(pipeline : PipelineState) : Void {
    if (pipeline == lastPipeline) {
        return;
    }
    lastPipeline = pipeline;
    flush();
    if (pipeline == null) {
        imagePainter.pipeline = null;
        coloredPainter.pipeline = null;
        textPainter.pipeline = null;
    }
    else {
        var cache = pipelineCache[pipeline];
        if (cache == null) {
            cache = new SimplePipelineCache(pipeline, true);
            pipelineCache[pipeline] = cache;
        }
        imagePainter.pipeline = cache;
        coloredPainter.pipeline = cache;
        textPainter.pipeline = cache;
    }
}*/

static bool scissorEnabled = false;
static int scissorX = -1;
static int scissorY = -1;
static int scissorW = -1;
static int scissorH = -1;

void kinc_g2_scissor(int x, int y, int width, int height) {
	// if (!scissorEnabled || x != scissorX || y != scissorY || width != scissorW || height != scissorH) {
	scissorEnabled = true;
	scissorX = x;
	scissorY = y;
	scissorW = width;
	scissorH = height;
	kinc_g2_flush();
	kinc_g4_scissor(x, y, width, height);
	// }
}

void kinc_g2_disable_scissor(void) {
	// if (scissorEnabled) {
	scissorEnabled = false;
	kinc_g2_flush();
	kinc_g4_disable_scissor();
	// }
}

void kinc_g2_begin(bool clear /*= true*/, uint32_t clear_color) {
	/*if (current == NULL) {
	    current = this;
	}
	else {
	    // End before you begin
	    assert(false);
	}*/

	kinc_g4_begin(0);
	if (clear) {
		kinc_g2_clear(clear_color);
	}
	set_projection();
}

void kinc_g2_clear(uint32_t color) {
	kinc_g2_flush();
	kinc_g4_clear(KINC_G4_CLEAR_COLOR, color, 0.0f, 0);
}

void kinc_g2_flush(void) {
	image_end();
	// textPainter.end();
	colored_end();
}

void kinc_g2_end(void) {
	kinc_g2_flush();
	kinc_g4_end(0);

	/*if (current == this) {
	    current = null;
	}
	else {
	    // Begin before you end
	    assert(false);
	}*/
}

// void draw_video_internal(Video *video, float x, float y, float width, float height) {}

/*void draw_video(Video *video, float x, float y, float width, float height) {
    setPipeline(videoPipeline);
    drawVideoInternal(video, x, y, width, height);
    setPipeline(null);
}*/

// font_t *get_font(void) {
// return NULL;
// }

// void set_font(font_t *font) {}

// int get_font_size(void) {
// return myFontSize;
// }

// void set_font_size(int size) {
// myFontSize = size;
// }

static int font_glyphs[256]; // [for (i in 32...256) i]

kinc_matrix3x3_t kinc_g2_get_transformation(void) {
	return transformations[transformation_index];
}

void kinc_g2_set_transformation(kinc_matrix3x3_t transformation) {
	transformations[transformation_index] = transformation;
}

void kinc_g2_push_transformation(kinc_matrix3x3_t trans) {
	transformation_index++;
	assert(transformation_index < transformations_size);
	transformations[transformation_index] = trans;
}

kinc_matrix3x3_t kinc_g2_pop_transformation() {
	transformation_index--;
	assert(transformation_index != -1);
	return transformations[transformation_index + 1];
}

static kinc_matrix3x3_t scale(float x, float y) {
	kinc_matrix3x3_t m;
	m.m[0] = x;
	m.m[1] = 0;
	m.m[2] = 0;
	m.m[3] = 0;
	m.m[4] = y;
	m.m[5] = 0;
	m.m[6] = 0;
	m.m[7] = 0;
	m.m[8] = 1;
	return m;
}

void kinc_g2_scale(float x, float y) {
	kinc_matrix3x3_t m1 = scale(x, y);
	kinc_matrix3x3_t m2 = kinc_g2_get_transformation();
	kinc_g2_set_transformation(kinc_matrix3x3_multiply(&m1, &m2));
}

void kinc_g2_push_scale(float x, float y) {
	kinc_matrix3x3_t m1 = scale(x, y);
	kinc_matrix3x3_t m2 = kinc_g2_get_transformation();
	kinc_matrix3x3_t mat = kinc_matrix3x3_multiply(&m1, &m2);
	kinc_g2_push_transformation(mat);
}

kinc_matrix3x3_t kinc_g2_translation(float tx, float ty) {
	kinc_matrix3x3_t m1 = kinc_matrix3x3_translation(tx, ty);
	kinc_matrix3x3_t m2 = kinc_g2_get_transformation();
	return kinc_matrix3x3_multiply(&m1, &m2);
}

void kinc_g2_translate(float tx, float ty) {
	kinc_g2_set_transformation(kinc_g2_translation(tx, ty));
}

void kinc_g2_push_translation(float tx, float ty) {
	kinc_g2_push_transformation(kinc_g2_translation(tx, ty));
}

static kinc_matrix3x3_t rotation(float alpha) {
	kinc_matrix3x3_t m;
	m.m[0] = cosf(alpha);
	m.m[1] = -sinf(alpha);
	m.m[2] = 0;
	m.m[3] = sinf(alpha);
	m.m[4] = cosf(alpha);
	m.m[5] = 0;
	m.m[6] = 0;
	m.m[7] = 0;
	m.m[8] = 1;
	return m;
}

kinc_matrix3x3_t kinc_g2_rotation(float angle, float centerx, float centery) {
	kinc_matrix3x3_t m1 = kinc_matrix3x3_translation(centerx, centery);
	kinc_matrix3x3_t m2 = rotation(angle);
	kinc_matrix3x3_t m3 = kinc_matrix3x3_translation(-centerx, -centery);
	kinc_matrix3x3_t m4 = kinc_g2_get_transformation();

	kinc_matrix3x3_t m = kinc_matrix3x3_multiply(&m1, &m2);
	m = kinc_matrix3x3_multiply(&m, &m3);
	m = kinc_matrix3x3_multiply(&m, &m4);
	return m;
}

void kinc_g2_rotate(float angle, float centerx, float centery) {
	kinc_g2_set_transformation(kinc_g2_rotation(angle, centerx, centery));
}

void kinc_g2_push_rotation(float angle, float centerx, float centery) {
	kinc_g2_push_transformation(kinc_g2_rotation(angle, centerx, centery));
}

void kinc_g2_push_opacity(float opacity) {
	opacities[opacities_size] = opacity;
	opacities_size += 1;
}

float kinc_g2_pop_opacity() {
	opacities_size -= 1;
	float ret = opacities[opacities_size];
	return ret;
}

float kinc_g2_get_opacity() {
	return opacities[opacities_size - 1];
}

void kinc_g2_set_opacity(float opacity) {
	opacities[opacities_size - 1] = opacity;
}

// PipelineState *pipe;

// PipelineState *get_pipeline(void) {
//	return pipe;
// }

// void set_pipeline(PipelineState *pipeline) {
//	setPipeline(pipeline);
//	return pipe = pipeline;
// }
