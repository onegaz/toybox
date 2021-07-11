package com.example.demorecyclerviewapp;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;
import com.google.common.flogger.FluentLogger;
import java.util.List;

public class RecyclerViewAdapter extends RecyclerView.Adapter<RecyclerViewHolder> {
  private static final FluentLogger logger = FluentLogger.forEnclosingClass();

  private List<DataModel> dataModelList;
  private int onCreateViewHolderCount;

  public RecyclerViewAdapter(List<DataModel> dataModelList) {
    this.dataModelList = dataModelList;
  }

  @NonNull
  @Override
  public RecyclerViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
    onCreateViewHolderCount++;
    logger.atInfo().log("onCreateViewHolder viewType=%s onCreateViewHolderCount=%s", viewType, onCreateViewHolderCount);
    View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.recycler_view_list_item, parent, false);
    return new RecyclerViewHolder(view);
  }

  @Override
  public int getItemCount() {
    return dataModelList.size();
  }

  @Override
  public void onBindViewHolder(@NonNull RecyclerViewHolder holder, int position) {
    logger.atInfo().log("onBindViewHolder position=%s", position);
    holder.setDescription(dataModelList.get(position).getDescription());
    holder.setTitle(dataModelList.get(position).getTitle());
  }
}
