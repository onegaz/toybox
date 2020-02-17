package com.example.myfragmentapplication;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import com.example.myfragmentapplication.ui.main.MainFragment;
import com.google.common.flogger.FluentLogger;

public class MainActivity extends AppCompatActivity {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    logger.atInfo().log("onCreate");
    super.onCreate(savedInstanceState);
    setContentView(R.layout.main_activity);
    if (savedInstanceState == null) {
      getSupportFragmentManager().beginTransaction()
          .replace(R.id.container, MainFragment.newInstance())
          .commitNow();
    }
  }

  @Override
  protected void onStart() {
    logger.atInfo().log("onStart");
    super.onStart();
  }

  @Override
  protected void onStop() {
    logger.atInfo().log("onStop");
    super.onStop();
  }

  @Override
  protected void onPostResume() {
    logger.atInfo().log("onPostResume");
    super.onPostResume();
  }

  @Override
  protected void onDestroy() {
    logger.atInfo().log("onDestroy");
    super.onDestroy();
  }
}
