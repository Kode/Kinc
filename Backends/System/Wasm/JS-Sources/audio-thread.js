// from https://developer.mozilla.org/en-US/docs/Web/API/AudioWorkletNode

function create_thread(func) {

}

class AudioThread extends AudioWorkletProcessor {
  constructor(...args) {
    super(...args);

    const self = this;

    this.port.onmessageerror = (e) => {
      console.error('Error', e);
    };

    this.port.onmessage = (e) => {
      console.log('Audio onmessage');

      const mod = e.data.mod;
      const memory = e.data.memory;
      
      const importObject = {
        env: { memory },
        imports: {
          imported_func: arg => console.log('thread: ' + arg),
          create_thread
        }
      };

      WebAssembly.instantiate(mod, importObject).then((instance) => {
        console.log('Running audio thread');
        self.audio_func = instance.exports.audio_func;
        self.audio_pointer = instance.exports.malloc(16 * 1024);
        console.log('Audio pointer: ' + self.audio_pointer);
        console.log('Memory byteLength: ' + memory.buffer.byteLength);
        self.audio_data = new Float32Array(
          memory.buffer,
          self.audio_pointer,
          16 * 256
        );
      });
    };
  }

  process(inputs, outputs, parameters) {
    const output = outputs[0];

    const data = output[0];

    //for (let i = 0; i < data.length; ++i) {
    //  data[i] = Math.random() * 2 - 1;
    //}

    if (this.audio_func) {
      let offset = 0;
      for (;;) {
        const length = Math.min(data.length - offset, this.audio_data.length);
        this.audio_func(this.audio_pointer + offset, length);
        for (let i = 0; i < length; ++i) {
          data[offset + i] = this.audio_data[i];
        }

        if (offset + this.audio_data.length >= data.length) {
          break;
        }
        offset += this.audio_data.length;
      }
    }

    return true;
  }
}

registerProcessor('audio-thread', AudioThread);
