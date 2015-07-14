package uiuc.bioassay.tlc;

import android.app.Activity;
import android.app.Application;
import android.os.Environment;
import android.provider.Settings;
import android.view.inputmethod.InputMethodManager;

import java.math.BigDecimal;
import java.math.RoundingMode;

/**
 * Created by meowle on 7/1/15.
 */
public class TLCApplication extends Application {
    static {
        System.loadLibrary("tlc");
    }
    public static final String AUTO_FOCUS = "AUTO_FOCUS";
    public static final String FOLDER_EXTRA = "FOLDER_EXTRA";
    public static final String ROOT_FOLDER = Environment.getExternalStorageDirectory() + "/Android/data/uiuc.bioassay.tlc";


    /*----------------------------------------------------------------------------*/
    // If you change the following constants, remmember to change in tlc.h as well
    public static final String AVG_FILE_NAME = "avg.png";
    public static final String LOG_FILE = "log.txt";
    public static final String BG_FOLDER = "bg/";
    public static final String SAMPLE_FOLDER = "sample/";
    public static final int MAX_PICTURE = 8;
    /*----------------------------------------------------------------------------*/

    public static final String PILLS_FOLDER = "pills";

    /* Some helper functions */
    public static double round(double value, int places) {
        if (places < 0) throw new IllegalArgumentException();

        BigDecimal bd = new BigDecimal(value);
        bd = bd.setScale(places, RoundingMode.HALF_UP);
        return bd.doubleValue();
    }

    public static void hideSoftKeyboard(Activity activity) {
        InputMethodManager inputMethodManager = (InputMethodManager)  activity.getSystemService(Activity.INPUT_METHOD_SERVICE);
        inputMethodManager.hideSoftInputFromWindow(activity.getCurrentFocus().getWindowToken(), 0);
    }

    /**
     * set screen off timeout
     * @param screenOffTimeout int 0~6
     */
    public static void setTimeout(Activity activity, int screenOffTimeout) {
        int time;
        switch (screenOffTimeout) {
            case 0:
                time = 15000;
                break;
            case 1:
                time = 30000;
                break;
            case 2:
                time = 60000;
                break;
            case 3:
                time = 120000;
                break;
            case 4:
                time = 600000;
                break;
            case 5:
                time = 1800000;
                break;
            default:
                time = -1;
        }
        android.provider.Settings.System.putInt(activity.getContentResolver(),
                Settings.System.SCREEN_OFF_TIMEOUT, time);
    }


    /* Native signatures */
    public native static void cleanFolder(String folder);
    public native static double[] processTLC(String folder);
}
