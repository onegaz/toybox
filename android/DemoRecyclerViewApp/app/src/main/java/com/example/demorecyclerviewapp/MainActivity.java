package com.example.demorecyclerviewapp;

import androidx.appcompat.app.AppCompatActivity;
import android.os.Bundle;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;
import com.google.common.flogger.FluentLogger;
import java.util.ArrayList;
import java.util.List;

public class MainActivity extends AppCompatActivity {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private List<DataModel> dataModelList;
  private RecyclerView demoView;

  @Override
  protected void onCreate(Bundle savedInstanceState) {
    super.onCreate(savedInstanceState);
    logger.atInfo().log("onCreate");
    setContentView(R.layout.activity_main);
    createDummyData();
    demoView = findViewById(R.id.recycler_view);
    LinearLayoutManager layoutManager = new LinearLayoutManager(this,
            LinearLayoutManager.VERTICAL, false);
    demoView.setLayoutManager(layoutManager);
    RecyclerViewAdapter adapter = new RecyclerViewAdapter(dataModelList);
    demoView.setAdapter(adapter);
  }

  @Override
  protected void onPostResume() {
    super.onPostResume();
    logger.atInfo().log("onPostResume");
  }

  @Override
  protected void onDestroy() {
    super.onDestroy();
    logger.atInfo().log("onDestroy");
  }

  @Override
  protected void onPause() {
    super.onPause();
    logger.atInfo().log("onPause");
  }

  @Override
  protected void onResume() {
    super.onResume();
    logger.atInfo().log("onResume");
  }

  private void createDummyData() {
    dataModelList = new ArrayList<>();
    dataModelList.add(new DataModel("Keyboard", "many keys"));
    dataModelList.add(new DataModel("MS mice", "3 buttons"));
    dataModelList.add(new DataModel("Mac mice", "1 button"));
    for(int i=0; i<40; i++) {
      dataModelList.add(new DataModel("dummy title " + i, "dummy description " + i));
    }
  }
}
