package com.example.demoviewpagerapp;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import androidx.viewpager.widget.ViewPager;
import androidx.viewpager.widget.ViewPager.OnPageChangeListener;
import com.google.common.flogger.FluentLogger;

public class MainActivity extends AppCompatActivity {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();
  private ViewPager viewPager;
  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    setContentView(R.layout.activity_main);
    viewPager = findViewById(R.id.view_pager);
    viewPager.setAdapter(new DemoViewPagerAdapter(getSupportFragmentManager(), 0));
    viewPager.addOnPageChangeListener(new OnPageChangeListener() {
      @Override
      public void onPageScrolled(int position, float positionOffset, int positionOffsetPixels) {
        logger.atFinest().log("onPageScrolled position=%s positionOffset=%s, positionOffsetPixels=%s", position, positionOffset, positionOffsetPixels);
      }

      @Override
      public void onPageSelected(int position) {
        logger.atInfo().log("onPageSelected position=%s", position);
      }

      @Override
      public void onPageScrollStateChanged(int state) {
        logger.atInfo().log("onPageScrollStateChanged state=%s", state);
      }
    });
  }
}
