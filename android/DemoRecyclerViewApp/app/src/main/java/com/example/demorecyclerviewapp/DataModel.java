package com.example.demorecyclerviewapp;

public class DataModel {
  private String title;
  private String description;

  public DataModel(String title, String description) {
    this.title = title;
    this.description = description;
  }

  public String getTitle() {
    return title;
  }

  public String getDescription() {
    return description;
  }
}
