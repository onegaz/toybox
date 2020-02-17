package com.example.demorecyclerviewapp;

import android.view.View;
import android.widget.TextView;
import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

public class RecyclerViewHolder extends RecyclerView.ViewHolder {
  private TextView title;
  private TextView description;

  public RecyclerViewHolder(@NonNull View itemView) {
    super(itemView);
    title = (TextView)itemView.findViewById(R.id.item_title);
    description = (TextView)itemView.findViewById(R.id.description);
  }

  public void setTitle(String title) {
    this.title.setText(title);
  }

  public void setDescription(String text) {
    description.setText(text);
  }
}
