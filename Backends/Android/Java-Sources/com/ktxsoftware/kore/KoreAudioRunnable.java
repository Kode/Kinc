package com.ktxsoftware.kore;

import android.media.AudioTrack;

public class KoreAudioRunnable implements Runnable {
    private final Object pauseLock;
    private boolean paused;
    private boolean finished;

    private AudioTrack audio;
    private int bufferSize;

    public KoreAudioRunnable(AudioTrack audio, int bufferSize) {
        this.audio = audio;
        this.bufferSize = bufferSize;
        pauseLock = new Object();
        paused = false;
        finished = false;
    }

    public void run() {
        Thread.currentThread().setPriority(Thread.MIN_PRIORITY);
        byte[] audioBuffer = new byte[bufferSize];

        while (!finished) {
            KoreLib.writeAudio(audioBuffer, audioBuffer.length);
            int written = 0;
            while (written < audioBuffer.length) {
                written += audio.write(audioBuffer, written, audioBuffer.length);
            }

            synchronized (pauseLock) {
                while (paused) {
                    try {
                        pauseLock.wait();
                    }
                    catch (InterruptedException e) {
                    }
                }
            }
        }
    }

    public void onPause() {
        synchronized (pauseLock) {
            paused = true;
        }
    }

    public void onResume() {
        synchronized (pauseLock) {
            paused = false;
            pauseLock.notifyAll();
        }
    }
}
