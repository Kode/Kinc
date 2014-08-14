package com.ktxsoftware.kore;

public class KoreTouchEvent {
	public int index;
	public int x;
	public int y;
	public int action;
	
	public static int ACTION_DOWN = 0;
	public static int ACTION_MOVE = 1;
	public static int ACTION_UP = 2;
	
	public KoreTouchEvent(int index, int x, int y, int action) {
		this.index = index;
		this.x = x;
		this.y = y;
		this.action = action;
	}
}
