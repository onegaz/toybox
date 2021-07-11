package com.example.myclock;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageInfo;
import android.content.pm.PackageManager.NameNotFoundException;
import android.graphics.PixelFormat;
import android.os.Build;
import android.os.Handler;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.TextView;
import com.google.common.flogger.FluentLogger;
import java.text.SimpleDateFormat;
import java.util.Calendar;

import static android.content.Context.WINDOW_SERVICE;
import static java.util.concurrent.TimeUnit.SECONDS;

public class SoftwareInfoWindow {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private Context context;
  private View popupWindow;
  private TextView textView;
  private WindowManager.LayoutParams popupLayoutParams;
  private WindowManager windowManager;
  private int invisibleCount;

  final Handler handler = new Handler();

  public SoftwareInfoWindow(Context context){
    this.context=context;

    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
      // set the layout parameters of the window
      popupLayoutParams = new WindowManager.LayoutParams(
          // Shrink the window to wrap the content rather
          // than filling the screen
          WindowManager.LayoutParams.WRAP_CONTENT, WindowManager.LayoutParams.WRAP_CONTENT,
          // Display it on top of other application windows
          WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY,
          // Don't let it grab the input focus
          WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE,
          // Make the underlying application window visible
          // through any transparent parts
          PixelFormat.TRANSLUCENT);
    }

    LayoutInflater layoutInflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);
    // inflating the view with the custom layout we created
    popupWindow = layoutInflater.inflate(R.layout.popup_window, null);

    refreshPopup();

    // set onClickListener on the remove button, which removes
    // the view from the window
    popupWindow.findViewById(R.id.window_close).setOnClickListener(new View.OnClickListener() {
      @Override
      public void onClick(View view) {
        logger.atInfo().log("close floating window");
        close();
        android.os.Process.killProcess(android.os.Process.myPid());
      }
    });
    // Define the position of the
    // window within the screen
    popupLayoutParams.gravity = Gravity.CENTER;
    popupLayoutParams.gravity = Gravity.TOP;
    windowManager = (WindowManager)context.getSystemService(WINDOW_SERVICE);

    handler.postDelayed(new Runnable() {
      public void run() {
        refreshPopup();
        if (textView !=null && textView.getVisibility() == View.VISIBLE) {
          invisibleCount = 0;
        } else {
          invisibleCount++;
        }
        if (invisibleCount < 20) {
          handler.postDelayed(this, 250);
        }
      }
    }, 250);
  }

  private String getGoogleAppVersion() {
    final String googleAppPackageName = "com.google.android.googlequicksearchbox";
    try {
      PackageInfo packageInfo = context.getPackageManager().getPackageInfo(googleAppPackageName, 0);
      return packageInfo.versionName;
    } catch (NameNotFoundException e) {
      logger.atWarning().atMostEvery(8, SECONDS).withCause(e)
              .log("%s not found", googleAppPackageName);
    }
    return "";
  }

  private void refreshPopup() {
    if (textView == null) {
      textView = popupWindow.findViewById(R.id.titleText);
      if (textView == null) {
        logger.atWarning().atMostEvery(8, SECONDS).log("Can't find view");
        return;
      }
    }

    SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss.SSS");
    String formattedDate = Build.FINGERPRINT + "    GoogleApp:" + getGoogleAppVersion() + "  " +
            df.format(Calendar.getInstance().getTime());
    textView.setText(formattedDate);
  }

  public void open() {

    try {
      // check if the view is already
      // inflated or present in the window
      if(popupWindow.getWindowToken()==null) {
        if(popupWindow.getParent()==null) {
          windowManager.addView(popupWindow, popupLayoutParams);
        }
      }
    } catch (Exception e) {
      logger.atSevere().withCause(e).log("windowManager.addView error");
    }
  }

  public void close() {
    try {
      // remove the view from the window
      ((WindowManager)context.getSystemService(WINDOW_SERVICE)).removeView(popupWindow);
      // invalidate the view
      popupWindow.invalidate();
      // remove all views
      if (popupWindow.getParent() != null) {
        ((ViewGroup) popupWindow.getParent()).removeAllViews();
      }

      Intent intent = new Intent(context, ForegroundService.class);
      context.stopService(intent);
      // the above steps are necessary when you are adding and removing
      // the view simultaneously, it might give some exceptions
    } catch (Exception e) {
      logger.atSevere().withCause(e).log("removeAllViews error");
    }
  }
}