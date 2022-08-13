// Includes snippets from https://surma.dev/things/c-to-webassembly/ and https://developer.mozilla.org/en-US/docs/WebAssembly/Using_the_JavaScript_API -->

let memory = null;
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

async function init() {
  memory = new WebAssembly.Memory({ initial: 2, maximum: 2, shared: true }); // has to match what's defined in the wasm-module when shared is true

  const result = await WebAssembly.instantiateStreaming(
    fetch("./add.wasm"), {
      env: { memory },
      imports: {
        imported_func: arg => console.log(arg),
        create_thread
      }
    }
  );

  mod = result.module;
  instance = result.instance;

  instance.exports._start();

  const kanvas = document.getElementById('kanvas');
  const context = kanvas.getContext('2d');
  const grd = context.createRadialGradient(75, 50, 5, 90, 60, 100);
  grd.addColorStop(0, 'red');
  grd.addColorStop(1, 'white');
  context.fillStyle = grd;
  context.fillRect(10, 10, 150, 80);
  kanvas.addEventListener('click', (event) => {
    start_audio_thread();
  });
}
init();
