// Includes snippets from https://surma.dev/things/c-to-webassembly/ and https://developer.mozilla.org/en-US/docs/WebAssembly/Using_the_JavaScript_API -->

let memory = null;
let heapu8 = null;
let heapu32 = null;
let mod = null;
let instance = null;

function create_thread(func) {
  console.log('Creating thread');
  const thread_starter = new Worker('thread_starter.js');
  const arr = new Uint8Array(memory.buffer, func, 256);
  let str = '';
  for (let i = 0; arr[i] != 0; ++i) {
    str += String.fromCharCode(arr[i]);
  }
  thread_starter.postMessage({ mod, memory, func: str });
}

async function start_audio_thread() {
  const audioContext = new AudioContext();
  await audioContext.audioWorklet.addModule('audio-thread.js');
  const audioThreadNode = new AudioWorkletNode(audioContext, 'audio-thread');
  audioThreadNode.port.postMessage({ mod, memory });
  audioThreadNode.connect(audioContext.destination);
}

function read_string(ptr) {
  let str = '';
  for (let i = 0; heapu8[ptr + i] != 0; ++i) {
    str += String.fromCharCode(heapu8[ptr + i]);
  }
  return str;
}

async function init() {
  memory = new WebAssembly.Memory({ initial: 18, maximum: 18, shared: true }); // has to match what's defined in the wasm-module when shared is true
  heapu8 = new Uint8Array(memory.buffer);
  heapu32 = new Uint32Array(memory.buffer);

  const kanvas = document.getElementById('kanvas');
  const gl = kanvas.getContext('webgl2', { majorVersion: 2, minorVersion: 0, antialias: true, alpha: false });

  let file_buffer = null;
  let gl_programs = [null];
  let gl_shaders = [null];
  let gl_buffers = [null];
  let gl_framebuffers = [null];

  const result = await WebAssembly.instantiateStreaming(
    fetch("./ShaderTest.wasm"), {
      env: { memory },
      imports: {
        create_thread,
        glViewport: function(x, y, width, height) {
          gl.viewport(x, y, width, height);
        },
        glGetIntegerv: function(pname, data) {
          // heapu32[data / 4] = gl.getParameter(pname);
        },
        glGetString: function(name) {
          // return gl.getParameter(name);
        },
        glUniform1i: function(location, v0) {
          gl.uniform1i(location, v0);
        },
        glDrawElements: function(mode, count, type, offset) {
          gl.drawElements(mode, count, type, offset);
        },
        glBindFramebuffer: function(target, framebuffer) {
          // gl.bindFramebuffer(target, gl_framebuffers[framebuffer]);
          gl.bindFramebuffer(target, gl_framebuffers[0]);
        },
        glEnable: function(cap) {
          gl.enable(cap);
        },
        glDisable: function(cap) {
          gl.disable(cap);
        },
        glColorMask: function(red, green, blue, alpha) {
          gl.colorMask(red, green, blue, alpha);
        },
        glClearColor: function(red, green, blue, alpha) {
          gl.clearColor(red, green, blue, alpha);
        },
        glDepthMask: function(flag) {
          gl.depthMask(flag);
        },
        glClearDepthf: function(depth) {
          gl.clearDepth(depth);
        },
        glStencilMask: function(mask) {
          gl.stencilMask(mask);
        },
        glClearStencil: function(s) {
          gl.clearStencil(s);
        },
        glClear: function(mask) {
          gl.clear(mask);
        },
        glBindBuffer: function(target, buffer) {
          gl.bindBuffer(target, gl_buffers[buffer]);
        },
        glUseProgram: function(program) {
          gl.useProgram(gl_programs[program]);
        },
        glStencilMaskSeparate: function(face, mask) {
          gl.stencilMaskSeparate(face, mask);
        },
        glStencilOpSeparate: function(face, fail, zfail, zpass) {
          gl.stencilOpSeparate(face, fail, zfail, zpass);
        },
        glStencilFuncSeparate: function(face, func, ref, mask) {
          gl.stencilFuncSeparate(face, func, ref, mask);
        },
        glDepthFunc: function(func) {
          gl.depthFunc(func);
        },
        glCullFace: function(mode) {
          gl.cullFace(mode);
        },
        glBlendFuncSeparate: function(src_rgb, dst_rgb, src_alpha, dst_alpha) {
          gl.blendFuncSeparate(src_rgb, dst_rgb, src_alpha, dst_alpha);
        },
        glBlendEquationSeparate: function(mode_rgb, mode_alpha) {
          gl.blendEquationSeparate(mode_rgb, mode_alpha);
        },
        glGenBuffers: function(n, buffers) {
          gl_buffers.push(gl.createBuffer());
          heapu32[buffers / 4] = gl_buffers.length - 1;
        },
        glBufferData: function(target, size, data, usage) {
          gl.bufferData(target, heapu8.subarray(data, data + Number(size)), usage);
        },
        glCreateProgram: function() {
          gl_programs.push(gl.createProgram());
          return gl_programs.length - 1;
        },
        glAttachShader: function(program, shader) {
          gl.attachShader(gl_programs[program], gl_shaders[shader]);
        },
        glBindAttribLocation: function(program, index, name) {
          gl.bindAttribLocation(gl_programs[program], index, read_string(name));
        },
        glLinkProgram: function(program) {
          gl.linkProgram(gl_programs[program]);
        },
        glGetProgramiv: function(program, pname, params) {
          heapu32[params / 4] = gl.getProgramParameter(gl_programs[program], pname);
        },
        glGetProgramInfoLog: function(program) {
          gl.getProgramInfoLog(gl_programs[program]);
        },
        glCreateShader: function(type) {
          gl_shaders.push(gl.createShader(type));
          return gl_shaders.length - 1;
        },
        glShaderSource: function(shader, count, source, length) {
          gl.shaderSource(gl_shaders[shader], read_string(heapu32[source / 4]));
        },
        glCompileShader: function(shader) {
          gl.compileShader(gl_shaders[shader]);
        },
        glGetShaderiv: function(shader, pname, params) {
          heapu32[params / 4] = gl.getShaderParameter(gl_shaders[shader], pname);
        },
        glGetShaderInfoLog: function(shader) {
          gl.getShaderInfoLog(gl_shaders[shader]);
        },
        glBufferSubData: function(target, offset, size, data) {
          gl.bufferSubData(target, Number(offset), heapu8.subarray(data, data + Number(size)), 0);
        },
        glEnableVertexAttribArray: function(index) {
          gl.enableVertexAttribArray(index);
        },
        glVertexAttribPointer: function(index, size, type, normalized, stride, offset) {
          gl.vertexAttribPointer(index, size, type, normalized, stride, offset);
        },
        glDisableVertexAttribArray: function(index) {
          gl.disableVertexAttribArray(index);
        },

        js_fprintf: function(format) {
          console.log(read_string(format));
        },
        js_fopen: function(filename) {
          const req = new XMLHttpRequest();
          req.open("GET", read_string(filename), false);
          req.send();
          let str = req.response;
          file_buffer = new ArrayBuffer(str.length);
          let buf_view = new Uint8Array(file_buffer);
          for (let i = 0; i < str.length; ++i) {
            buf_view[i] = str.charCodeAt(i);
          }
          return 1;
        },
        js_ftell: function(stream) {
          return file_buffer.byteLength;
        },
        js_fread: function(ptr, size, count, stream) {
          let buf_view = new Uint8Array(file_buffer);
          for (let i = 0; i < count; ++i) {
            heapu8[ptr + i] = buf_view[i];
          }
          return file_buffer.byteLength;
        }
      }
    }
  );

  mod = result.module;
  instance = result.instance;
  instance.exports._start();

  kanvas.addEventListener('click', (event) => {
  //  start_audio_thread();
  });
}

init();
