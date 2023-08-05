package tech.kinc

import android.app.NativeActivity
import android.content.Context
import android.content.Intent
import android.content.pm.ApplicationInfo
import android.content.pm.PackageManager
import android.net.Uri
import android.os.Bundle
import android.os.Handler
import android.os.Message
import android.os.Vibrator
import android.os.VibrationEffect
import android.os.Build
import android.view.KeyEvent
import android.view.View
import android.view.WindowManager
import android.view.inputmethod.InputMethodManager
import kotlin.system.exitProcess

class KincActivity: NativeActivity(), KeyEvent.Callback {
	companion object {
		var instance: KincActivity? = null

		private fun getInstance(): KincActivity {
			return instance!!
		}

		fun showKeyboard() {
			getInstance().inputManager!!.showSoftInput(getInstance().window.decorView, 0)
		}

		fun hideKeyboard() {
			getInstance().inputManager!!.hideSoftInputFromWindow(getInstance().window.decorView.windowToken, 0)
			getInstance().delayedHideSystemUI()
		}

		fun loadURL(url: String) {
			val i = Intent(Intent.ACTION_VIEW, Uri.parse(url))
			getInstance().startActivity(i)
		}

		fun getLanguage(): String {
			return java.util.Locale.getDefault().language
		}

		fun vibrate(ms: Int) {
			val v: Vibrator = getInstance().getSystemService(Context.VIBRATOR_SERVICE) as Vibrator
			if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
				v.vibrate(VibrationEffect.createOneShot(ms.toLong(), VibrationEffect.DEFAULT_AMPLITUDE))
			}
			else {
				// deprecated in API 26
				v.vibrate(ms.toLong())
			}
		}

		fun getRotation(): Int {
			val context: Context = getInstance().applicationContext
			val manager: WindowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
			return manager.defaultDisplay.rotation
		}

		fun getScreenDpi(): Int {
			val context: Context = getInstance().applicationContext
			val manager: WindowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
			val metrics: android.util.DisplayMetrics = android.util.DisplayMetrics()
			manager.defaultDisplay.getMetrics(metrics)
			return metrics.xdpi.toInt()
		}

		fun getRefreshRate(): Int {
			val context: Context = getInstance().applicationContext
			val manager: WindowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
			return manager.defaultDisplay.refreshRate.toInt()
		}

		fun getDisplayWidth(): Int {
			val context: Context = getInstance().applicationContext
			val manager: WindowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
			val size: android.graphics.Point = android.graphics.Point()
			manager.defaultDisplay.getRealSize(size)
			return size.x
		}

		fun getDisplayHeight(): Int {
			val context: Context = getInstance().applicationContext
			val manager: WindowManager = context.getSystemService(Context.WINDOW_SERVICE) as WindowManager
			val size: android.graphics.Point = android.graphics.Point()
			manager.defaultDisplay.getRealSize(size)
			return size.y
		}

		fun stop() {
			getInstance().runOnUiThread {
				fun run() {
					getInstance().finish()
					exitProcess(0)
				}
			}
		}

		class MyHandler(private val kincActivity: KincActivity) : Handler() {
			override fun handleMessage(msg: Message) {
				kincActivity.hideSystemUI()
			}
		}
	}

	var inputManager: InputMethodManager? = null
	private var isDisabledStickyImmersiveMode = false

	private val hideSystemUIHandler = MyHandler(this)

	override fun onCreate(state: Bundle?) {
		super.onCreate(state)
		hideSystemUI()
		instance = this
		inputManager = getSystemService(Context.INPUT_METHOD_SERVICE) as InputMethodManager
		isDisabledStickyImmersiveMode = try {
			val ai: ApplicationInfo = packageManager.getApplicationInfo(packageName, PackageManager.GET_META_DATA)
			val bundle: Bundle = ai.metaData
			bundle.getBoolean("disableStickyImmersiveMode")
		} catch (e: PackageManager.NameNotFoundException) {
			false
		} catch (e: NullPointerException) {
			false
		}
	}

    private fun hideSystemUI() {
        window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_LAYOUT_STABLE or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
    }

    private fun delayedHideSystemUI() {
        hideSystemUIHandler.removeMessages(0)
        if (!isDisabledStickyImmersiveMode) {
            hideSystemUIHandler.sendEmptyMessageDelayed(0, 300)
        }
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) {
            delayedHideSystemUI()
        }
        else {
            hideSystemUIHandler.removeMessages(0)
        }
    }

	override fun onKeyMultiple(keyCode: Int, count: Int, event: KeyEvent): Boolean {
		this.nativeKincKeyPress(event.characters)
		return false
	}

	private external fun nativeKincKeyPress(chars: String)
}
