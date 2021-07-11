package com.example.myfragmentapplication.ui.main;

import android.os.Build.VERSION;
import android.view.View.OnClickListener;
import android.widget.TextView;
import androidx.appcompat.widget.AppCompatButton;
import androidx.lifecycle.Observer;
import androidx.lifecycle.ViewModelProvider;
//import androidx.lifecycle.ViewModelProviders;
import android.os.Bundle;
import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import com.example.myfragmentapplication.R;
import com.google.common.flogger.FluentLogger;

public class MainFragment extends Fragment {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();
  private MainViewModel mainViewModel;
  private TextView textView;
  private TextView buildInfo;
  private AppCompatButton doAction;

  public static MainFragment newInstance() {
    return new MainFragment();
  }

  @Nullable
  @Override
  public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
      @Nullable Bundle savedInstanceState) {
    logger.atInfo().log("onCreateView");
    return inflater.inflate(R.layout.main_fragment, container, false);
  }

  @Override
  public void onStart() {
    logger.atInfo().log("onStart findViewById");
    super.onStart();
    textView = (TextView)getView().findViewById(R.id.message);
    buildInfo = (TextView)getView().findViewById(R.id.build_info);
    doAction = (AppCompatButton)getView().findViewById(R.id.do_action);
    //textView.setText(mViewModel.getDemoElement().getValue().getMessage());

    buildInfo.setText(getBuildString());
    doAction.setOnClickListener(new OnClickListener() {
      @Override
      public void onClick(View v) {
        logger.atInfo().log("onClick call mViewModel.doAction");
        mainViewModel.doAction();
      }
    });
  }

  @Override
  public void onActivityCreated(@Nullable Bundle savedInstanceState) {
    logger.atInfo().log("onActivityCreated");
    super.onActivityCreated(savedInstanceState);
    // ViewModelProviders deprecated
    // mViewModel = ViewModelProviders.of(this).get(MainViewModel.class);
    mainViewModel = new ViewModelProvider(this).get(MainViewModel.class);

    mainViewModel.getDemoElement().observe(getViewLifecycleOwner(), new Observer<DemoElement>() {
      @Override
      public void onChanged(DemoElement demoElement) {
        textView.setText(mainViewModel.getDemoElement().getValue().getMessage());
        logger.atInfo().log("DemoElement.onChanged to " + textView.getText());
        if (doAction.getVisibility()==View.GONE) {
          logger.atInfo().log("DemoElement.onChanged make button VISIBLE");
          doAction.setVisibility(View.VISIBLE);
        }
      }
    });
  }

  private static String getBuildString() {
    StringBuilder sb = new StringBuilder();
    sb.append("BASE_OS=").append(VERSION.BASE_OS);
    sb.append(", CODENAME=").append(VERSION.CODENAME);
    sb.append(", RELEASE=").append(VERSION.RELEASE);
    sb.append(", SDK_INT=").append(VERSION.SDK_INT);
    return sb.toString();
  }
}
