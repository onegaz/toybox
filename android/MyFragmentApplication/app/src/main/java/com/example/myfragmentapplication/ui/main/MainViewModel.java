package com.example.myfragmentapplication.ui.main;

import androidx.lifecycle.LiveData;
import androidx.lifecycle.MutableLiveData;
import androidx.lifecycle.ViewModel;
import com.google.common.flogger.FluentLogger;
import java.util.Date;

public class MainViewModel extends ViewModel {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();
  private final MutableLiveData<DemoElement> demoElementMutableLiveData = new MutableLiveData<>();
  public LiveData<DemoElement> getDemoElement() {
    return demoElementMutableLiveData;
  }

  public MainViewModel() {
    logger.atInfo().log("MainViewModel set initial value");
    demoElementMutableLiveData.setValue(new DemoElement("Initial DemoElement"));
  }

  void doAction() {
    demoElementMutableLiveData.postValue(new DemoElement("Initial DemoElement updated at " + new Date().toString()));
  }
}
