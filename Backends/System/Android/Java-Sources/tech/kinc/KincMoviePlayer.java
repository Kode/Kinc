package tech.kinc;

import java.util.List;

import android.view.Surface;

public class KincMoviePlayer {
	public static List<KincMoviePlayer> players = new java.util.ArrayList<KincMoviePlayer>();
	private KincMovieTexture movieTexture;
	private int id;
	private String path;

	public KincMoviePlayer(String path) {
		this.path = path;
		id = players.size();
		players.add(this);
	}
	
	public void init() {
		movieTexture = new KincMovieTexture();
		Surface surface = new Surface(movieTexture.surfaceTexture);
		nativeCreate(path, surface, id);
		surface.release();
	}

	public KincMovieTexture getMovieTexture() {
		return movieTexture;
	}

	public boolean update() {
		return movieTexture.update();
	}

	public static void updateAll() {
		for (KincMoviePlayer player : KincMoviePlayer.players) {
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
