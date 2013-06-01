package com.ktxsoftware.kore;

public class KoreLib {
    static {
        System.loadLibrary("Kore");
    }

    public static native void init(int width, int height, String apkPath);
    public static native void step();
    public static native void touchDown(int x, int y);
    public static native void touchUp(int x, int y);
    public static native void touchMove(int x, int y);
}
