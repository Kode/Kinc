package com.ktxsoftware.kore;

import java.util.List;

import android.view.Surface;

public class KoreMoviePlayer {
	public static List<KoreMoviePlayer> players = new java.util.ArrayList<KoreMoviePlayer>();
	private KoreMovieTexture movieTexture;
	private int id;
	private String path;

	public KoreMoviePlayer(String path) {
		this.path = path;
		id = players.size();
		players.add(this);
	}
	
	public void init() {
		movieTexture = new KoreMovieTexture();
		Surface surface = new Surface(movieTexture.surfaceTexture);
		nativeCreate(path, surface, id);
		surface.release();
	}

	public KoreMovieTexture getMovieTexture() {
		return movieTexture;
	}

	public boolean update() {
		return movieTexture.update();
	}

	public static void updateAll() {
		for (KoreMoviePlayer player : KoreMoviePlayer.players) {
			player.update();
		}
	}
	
	public int getTextureId() {
		return movieTexture.textureId;
	}
	
	public int getId() {
		return id;
	}
	
	public static void remove(int id) {
		players.set(id, null);
	}

	private native void nativeCreate(String path, Surface surface, int id);
}
