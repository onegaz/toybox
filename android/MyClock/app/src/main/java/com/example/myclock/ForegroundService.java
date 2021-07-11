package com.example.myclock;

import android.app.Notification;
import android.app.NotificationChannel;
import android.app.NotificationManager;
import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.os.Build;
import android.os.IBinder;
import com.google.common.flogger.FluentLogger;

import androidx.annotation.RequiresApi;
import androidx.core.app.NotificationCompat;

public class ForegroundService extends Service {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();
  private SoftwareInfoWindow softwareInfoWindow;
  public ForegroundService() {
  }

  @Override
  public IBinder onBind(Intent intent) {
    throw new UnsupportedOperationException("Not yet implemented");
  }

  @Override
  public void onCreate() {
    super.onCreate();
    // create the custom or default notification
    // based on the android version
    if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O)
      startForegroundWithCustomNotification();
    else
      startForeground(1, new Notification());
  }

  @Override
  public int onStartCommand(Intent intent, int flags, int startId) {
    if (softwareInfoWindow == null) {
      softwareInfoWindow =new SoftwareInfoWindow(this);
    }
    softwareInfoWindow.open();
    return super.onStartCommand(intent, flags, startId);
  }

  // for android version >=O we need to create custom notification stating that a foreground service is running
  @RequiresApi(Build.VERSION_CODES.O)
  private void startForegroundWithCustomNotification()
  {
    String NOTIFICATION_CHANNEL_ID = "example.permanence";
    String channelName = "Background Service";
    NotificationChannel chan = new NotificationChannel(NOTIFICATION_CHANNEL_ID, channelName, NotificationManager.IMPORTANCE_MIN);

    NotificationManager manager = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
    assert manager != null;
    manager.createNotificationChannel(chan);

    NotificationCompat.Builder notificationBuilder = new NotificationCompat.Builder(this, NOTIFICATION_CHANNEL_ID);
    Notification notification = notificationBuilder.setOngoing(true)
        .setContentTitle("SoftwareInfoWindow Service running")
        .setContentText("Displaying SoftwareInfoWindow over other apps")

        // this is important, otherwise the notification will show the way
        // you want i.e. it will show some default notification
        .setSmallIcon(R.drawable.ic_launcher_foreground)

        .setPriority(NotificationManager.IMPORTANCE_MIN)
        .setCategory(Notification.CATEGORY_SERVICE)
        .build();
    startForeground(2, notification);
  }
}
