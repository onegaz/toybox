package com.example.demoviewpagerapp;

import androidx.annotation.NonNull;
import androidx.fragment.app.Fragment;
import androidx.fragment.app.FragmentManager;
import androidx.fragment.app.FragmentPagerAdapter;
import java.util.ArrayList;
import java.util.List;

public class DemoViewPagerAdapter extends FragmentPagerAdapter {
  private final List<Fragment> fragmentList = new ArrayList<>();

  public DemoViewPagerAdapter(@NonNull FragmentManager fm,
      int behavior) {
    super(fm, behavior);
    fragmentList.add(FirstPage.newInstance());
    fragmentList.add(SecondPage.newInstance());
  }

  @NonNull
  @Override
  public Fragment getItem(int position) {
    return fragmentList.get(position);
  }

  @Override
  public int getCount() {
    return fragmentList.size();
  }
}
