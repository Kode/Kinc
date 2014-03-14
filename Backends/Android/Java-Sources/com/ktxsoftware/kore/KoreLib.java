package com.ktxsoftware.kore;

public class KoreLib {
    static {
        System.loadLibrary("Kore");
    }

    public static native void init(int width, int height, String apkPath, String filesDir);
    public static native void step();
    public static native void touchDown(int x, int y);
    public static native void touchUp(int x, int y);
    public static native void touchMove(int x, int y);
    public static native boolean keyboardShown();
    public static native boolean keyUp(int code);
    public static native boolean keyDown(int code);
    public static native void writeAudio(byte[] buffer, int size);
}
