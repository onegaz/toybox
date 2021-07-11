package com.example.myclock;

import android.app.AppOpsManager;
import android.content.Context;
import android.graphics.PixelFormat;
import android.os.Binder;
import android.view.View;
import android.view.WindowManager;
import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;

import android.content.Intent;
import android.os.Build;
import android.provider.Settings;
import com.google.common.flogger.FluentLogger;

public class MainActivity extends AppCompatActivity {
    private static final FluentLogger logger = FluentLogger.forEnclosingClass();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
    }

    // method for starting the service
    public void startService() {
        // check if the user has already granted
        // the Draw over other apps permission
        if(canDrawOverlays(this)) {
            logger.atInfo().log("startService");
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
                startForegroundService(new Intent(this, ForegroundService.class));
            } else {
                startService(new Intent(this, ForegroundService.class));
            }
        } else {
            logger.atWarning().log("canDrawOverlays: No permission");
        }
    }

    // method to ask user to grant the Overlay permission
    public void checkOverlayPermission(){
        if (!canDrawOverlays(this)) {
            // send user to the device settings
            Intent myIntent = new Intent(Settings.ACTION_MANAGE_OVERLAY_PERMISSION);
            startActivity(myIntent);
        } else {
            logger.atInfo().log("checkOverlayPermission pass");
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        logger.atWarning().log("onResume show overlay again");
        checkOverlayPermission();
        startService();
    }

    static boolean canDrawOverlays(Context context) {
        if (Build.VERSION.SDK_INT > Build.VERSION_CODES.M && Settings.canDrawOverlays(context)) {
            return true;
        }
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.M) {//USING APP OPS MANAGER
            AppOpsManager manager = (AppOpsManager) context.getSystemService(Context.APP_OPS_SERVICE);
            if (manager != null) {
                try {
                    int result = manager.checkOp(AppOpsManager.OPSTR_SYSTEM_ALERT_WINDOW, Binder.getCallingUid(), context.getPackageName());
                    return result == AppOpsManager.MODE_ALLOWED;
                } catch (Exception ignore) {
                }
            }
        }

        try {//IF This Fails, we definitely can't do it
            WindowManager mgr = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
            if (mgr == null) return false; //getSystemService might return null
            View viewToAdd = new View(context);
            WindowManager.LayoutParams params = new WindowManager.LayoutParams(0, 0, android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.O ?
                    WindowManager.LayoutParams.TYPE_APPLICATION_OVERLAY : WindowManager.LayoutParams.TYPE_SYSTEM_ALERT,
                    WindowManager.LayoutParams.FLAG_NOT_TOUCHABLE | WindowManager.LayoutParams.FLAG_NOT_FOCUSABLE, PixelFormat.TRANSPARENT);
            viewToAdd.setLayoutParams(params);
            mgr.addView(viewToAdd, params);
            mgr.removeView(viewToAdd);
            return true;
        } catch (Exception ignore) {
        }
        return false;
    }
}