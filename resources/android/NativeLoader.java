// clang-format off
package ${APP_PACKAGE};

import android.os.Bundle;
import android.os.Build;
import android.app.NativeActivity;
import android.view.View;
import android.view.WindowManager.LayoutParams;

public
class NativeLoader extends NativeActivity
{
    static {
        System.loadLibrary("${LIBNAME}");
    }
    @Override protected void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
        CorrectScreen();
    }

    // Handling loss and regain of application focus
    @Override public void onWindowFocusChanged(boolean hasFocus)
    {
        super.onWindowFocusChanged(hasFocus);
        if (hasFocus)
        {
            CorrectScreen();
        }
    }

    @Override public void onResume()
    {
        super.onResume();
        FullScreencall();
    }

    // @Override public void onDestroy(){
    //     super.onDestroy();
    //     finish();
    // }

public
    void FullScreencall()
    {
        if (Build.VERSION.SDK_INT > 11 && Build.VERSION.SDK_INT < 19)
        {  // lower api
            View v = this.getWindow().getDecorView();
            v.setSystemUiVisibility(View.GONE);
        }
        else if (Build.VERSION.SDK_INT >= 19)
        {
            // for new api versions.
            View decorView = getWindow().getDecorView();
            int  uiOptions = View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
                            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY;
            decorView.setSystemUiVisibility(uiOptions);
        }
    }
public
    void CorrectScreen()
    {
        this.getWindow().addFlags(LayoutParams.FLAG_KEEP_SCREEN_ON);

        this.getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_FULLSCREEN | View.SYSTEM_UI_FLAG_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY |
            View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN |
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION |
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE);

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.P)
        {
            // API 28 andabove
            LayoutParams lp = this.getWindow().getAttributes();
            lp.layoutInDisplayCutoutMode = LayoutParams.LAYOUT_IN_DISPLAY_CUTOUT_MODE_SHORT_EDGES;
            this.getWindow().setAttributes(lp);
        }
    }
}