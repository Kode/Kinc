/*class TextShaderPainter {
    var projectionMatrix : FastMatrix4;

    static var standardTextPipeline : PipelineCache = null;
    static var structure : VertexStructure = null;
    static inline var bufferSize : Int = 1000;
    static var bufferIndex : Int;
    static var rectVertexBuffer : VertexBuffer;
    static var rectVertices : Float32Array;
    static var indexBuffer : IndexBuffer;

    var font : Kravur;

    static var lastTexture : Image;

    var g : Graphics;
    var myPipeline : PipelineCache = null;

public
    var pipeline(get, set) : PipelineCache;
public
    var fontSize : Int;

    var bilinear : Bool = false;

public
    function new(g4 : Graphics) {
        this.g = g4;
        bufferIndex = 0;
        initShaders();
        myPipeline = standardTextPipeline;
        initBuffers();
    }

    function get_pipeline() : PipelineCache {
        return myPipeline;
    }

    function set_pipeline(pipe : PipelineCache) : PipelineCache {
        myPipeline = pipe != null ? pipe : standardTextPipeline;
        return myPipeline;
    }

public
    function setProjection(projectionMatrix : FastMatrix4) : Void {
        this.projectionMatrix = projectionMatrix;
    }

    static function initShaders() : Void {
        if (structure == null) {
            structure = Graphics2.createTextVertexStructure();
        }
        if (standardTextPipeline == null) {
            var pipeline = Graphics2.createTextPipeline(structure);
            standardTextPipeline = new PerFramebufferPipelineCache(pipeline, true);
        }
    }

    function initBuffers() : Void {
        if (rectVertexBuffer == null) {
            rectVertexBuffer = new VertexBuffer(bufferSize * 4, structure, Usage.DynamicUsage);
            rectVertices = rectVertexBuffer.lock();

            indexBuffer = new IndexBuffer(bufferSize * 3 * 2, Usage.StaticUsage);
            var indices = indexBuffer.lock();
            for (i in 0...bufferSize) {
                indices[i * 3 * 2 + 0] = i * 4 + 0;
                indices[i * 3 * 2 + 1] = i * 4 + 1;
                indices[i * 3 * 2 + 2] = i * 4 + 2;
                indices[i * 3 * 2 + 3] = i * 4 + 0;
                indices[i * 3 * 2 + 4] = i * 4 + 2;
                indices[i * 3 * 2 + 5] = i * 4 + 3;
            }
            indexBuffer.unlock();
        }
    }

    function setRectVertices(bottomleftx
                             : Float, bottomlefty
                             : Float, topleftx
                             : Float, toplefty
                             : Float, toprightx
                             : Float, toprighty
                             : Float, bottomrightx
                             : Float, bottomrighty
                             : Float)
        : Void {
        var baseIndex : Int = bufferIndex * 9 * 4;
        rectVertices.set(baseIndex + 0, bottomleftx);
        rectVertices.set(baseIndex + 1, bottomlefty);
        rectVertices.set(baseIndex + 2, -5.0);

        rectVertices.set(baseIndex + 9, topleftx);
        rectVertices.set(baseIndex + 10, toplefty);
        rectVertices.set(baseIndex + 11, -5.0);

        rectVertices.set(baseIndex + 18, toprightx);
        rectVertices.set(baseIndex + 19, toprighty);
        rectVertices.set(baseIndex + 20, -5.0);

        rectVertices.set(baseIndex + 27, bottomrightx);
        rectVertices.set(baseIndex + 28, bottomrighty);
        rectVertices.set(baseIndex + 29, -5.0);
    }

    function setRectTexCoords(left : Float, top : Float, right : Float, bottom : Float) : Void {
        var baseIndex : Int = bufferIndex * 9 * 4;
        rectVertices.set(baseIndex + 3, left);
        rectVertices.set(baseIndex + 4, bottom);

        rectVertices.set(baseIndex + 12, left);
        rectVertices.set(baseIndex + 13, top);

        rectVertices.set(baseIndex + 21, right);
        rectVertices.set(baseIndex + 22, top);

        rectVertices.set(baseIndex + 30, right);
        rectVertices.set(baseIndex + 31, bottom);
    }

    function setRectColors(opacity : FastFloat, color : Color) : Void {
        var baseIndex : Int = bufferIndex * 9 * 4;
        var a : FastFloat = opacity * color.A;
        rectVertices.set(baseIndex + 5, color.R);
        rectVertices.set(baseIndex + 6, color.G);
        rectVertices.set(baseIndex + 7, color.B);
        rectVertices.set(baseIndex + 8, a);

        rectVertices.set(baseIndex + 14, color.R);
        rectVertices.set(baseIndex + 15, color.G);
        rectVertices.set(baseIndex + 16, color.B);
        rectVertices.set(baseIndex + 17, a);

        rectVertices.set(baseIndex + 23, color.R);
        rectVertices.set(baseIndex + 24, color.G);
        rectVertices.set(baseIndex + 25, color.B);
        rectVertices.set(baseIndex + 26, a);

        rectVertices.set(baseIndex + 32, color.R);
        rectVertices.set(baseIndex + 33, color.G);
        rectVertices.set(baseIndex + 34, color.B);
        rectVertices.set(baseIndex + 35, a);
    }

    function drawBuffer() : Void {
        if (bufferIndex == 0) {
            return;
        }

        rectVertexBuffer.unlock(bufferIndex * 4);
        var pipeline = myPipeline.get(null, Depth24Stencil8);
        g.setPipeline(pipeline.pipeline);
        g.setVertexBuffer(rectVertexBuffer);
        g.setIndexBuffer(indexBuffer);
        g.setMatrix(pipeline.projectionLocation, projectionMatrix);
        g.setTexture(pipeline.textureLocation, lastTexture);
        g.setTextureParameters(pipeline.textureLocation, TextureAddressing.Clamp, TextureAddressing.Clamp,
                               bilinear ? TextureFilter.LinearFilter : TextureFilter.PointFilter,
                               bilinear ? TextureFilter.LinearFilter : TextureFilter.PointFilter, MipMapFilter.NoMipFilter);

        g.drawIndexedVertices(0, bufferIndex * 2 * 3);

        g.setTexture(pipeline.textureLocation, null);
        bufferIndex = 0;
        rectVertices = rectVertexBuffer.lock();
    }

public
    function setBilinearFilter(bilinear : Bool) : Void {
        end();
        this.bilinear = bilinear;
    }

public
    function setFont(font : Font) : Void {
        this.font = cast(font, Kravur);
    }

    static function findIndex(charCode : Int) : Int {
        // var glyphs = kha.graphics2.Graphics.fontGlyphs;
        var blocks = Kravur.KravurImage.charBlocks;
        var offset = 0;
        for (i in 0...Std.int(blocks.length / 2)) {
            var start = blocks[i * 2];
            var end = blocks[i * 2 + 1];
            if (charCode >= start && charCode <= end)
                return offset + charCode - start;
            offset += end - start + 1;
        }
        return 0;
    }

    var bakedQuadCache = new kha.Kravur.AlignedQuad();

public
    function drawString(text : String, opacity : FastFloat, color : Color, x : Float, y : Float, transformation : FastMatrix3) : Void {
        var font = this.font._get(fontSize);
        var tex = font.getTexture();
        if (lastTexture != null && tex != lastTexture)
            drawBuffer();
        lastTexture = tex;

        var xpos = x;
        var ypos = y;
        for (i in 0...text.length) {
            var charCode = StringTools.fastCodeAt(text, i);
            var q = font.getBakedQuad(bakedQuadCache, findIndex(charCode), xpos, ypos);
            if (q != null) {
                if (bufferIndex + 1 >= bufferSize)
                    drawBuffer();
                setRectColors(opacity, color);
                setRectTexCoords(q.s0 * tex.width / tex.realWidth, q.t0 * tex.height / tex.realHeight, q.s1 * tex.width / tex.realWidth,
                                 q.t1 * tex.height / tex.realHeight);
                var p0 = transformation.multvec(new FastVector2(q.x0, q.y1)); // bottom-left
                var p1 = transformation.multvec(new FastVector2(q.x0, q.y0)); // top-left
                var p2 = transformation.multvec(new FastVector2(q.x1, q.y0)); // top-right
                var p3 = transformation.multvec(new FastVector2(q.x1, q.y1)); // bottom-right
                setRectVertices(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
                xpos += q.xadvance;
                ++bufferIndex;
            }
        }
    }

public
    function drawCharacters(text
                            : Array<Int>, start
                            : Int, length
                            : Int, opacity
                            : FastFloat, color
                            : Color, x
                            : Float, y
                            : Float, transformation
                            : FastMatrix3)
        : Void {
        var font = this.font._get(fontSize);
        var tex = font.getTexture();
        if (lastTexture != null && tex != lastTexture)
            drawBuffer();
        lastTexture = tex;

        var xpos = x;
        var ypos = y;
        for (i in start... start + length) {
            var q = font.getBakedQuad(bakedQuadCache, findIndex(text[i]), xpos, ypos);
            if (q != null) {
                if (bufferIndex + 1 >= bufferSize)
                    drawBuffer();
                setRectColors(opacity, color);
                setRectTexCoords(q.s0 * tex.width / tex.realWidth, q.t0 * tex.height / tex.realHeight, q.s1 * tex.width / tex.realWidth,
                                 q.t1 * tex.height / tex.realHeight);
                var p0 = transformation.multvec(new FastVector2(q.x0, q.y1)); // bottom-left
                var p1 = transformation.multvec(new FastVector2(q.x0, q.y0)); // top-left
                var p2 = transformation.multvec(new FastVector2(q.x1, q.y0)); // top-right
                var p3 = transformation.multvec(new FastVector2(q.x1, q.y1)); // bottom-right
                setRectVertices(p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, p3.x, p3.y);
                xpos += q.xadvance;
                ++bufferIndex;
            }
        }
    }

public
    function end() : Void {
        if (bufferIndex > 0)
            drawBuffer();
        lastTexture = null;
    }
}*/
