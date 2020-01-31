#ifndef RNTI_TABLE_H
#define RNTI_TABLE_H

#pragma once

#include "qcustomplot/qcustomplot.h"
#include "settings.h"
#include "adapters_qt/SpectrumAdapter.h"
#include "plots.h"

#include <iostream>

struct RNTITableDataStorage{
public:
  RNTITableDataStorage(){};
  ~RNTITableDataStorage(){};

  void insert(uint32_t rnti, const uint32_t tbs, const uint32_t mcs, const uint32_t prb){
    if(!db.contains(rnti)){
      // RNTI has not been inserted yet. Making first entry and initialize db
      QMap < QString, uint32_t > per_rnti_vals;
      per_rnti_vals.insert("tbs", tbs);
      per_rnti_vals.insert("mcs", mcs);
      per_rnti_vals.insert("prb", prb);
      per_rnti_vals.insert("activity", 1);
      db.insert(rnti, per_rnti_vals);
    }else{
      // If there is already an entry for this rnti, just access the db and add the constructed data
      QMap < QString, uint32_t > per_rnti_vals;

      per_rnti_vals.insert("tbs", tbs + db.value(rnti).value("tbs"));
      per_rnti_vals.insert("mcs", mcs + db.value(rnti).value("mcs"));
      per_rnti_vals.insert("prb", prb + db.value(rnti).value("prb"));
      per_rnti_vals.insert("activity", 1 + db.value(rnti).value("activity"));
      db.insert(rnti, per_rnti_vals);
    }
  }

  QMap<uint32_t, QMap < QString, double > > calcAndPurge(){
    QMap<uint32_t, QMap < QString, double > > result;

    // Iterate over all RNTIs
    QMap<uint32_t, QMap < QString, uint32_t > >::const_iterator rnti_entry = db.constBegin();
    while (rnti_entry != db.constEnd()) {
      uint32_t currentRNTI = rnti_entry.key();
      // Calculate values for the current RNTI
      QMap < QString, double > values;
      uint32_t activity_count = db.value(currentRNTI).value("activity");
      values.insert("activity", activity_count);
      values.insert("tbs", (double)db.value(currentRNTI).value("tbs")/activity_count);
      values.insert("mcs", (double)db.value(currentRNTI).value("mcs")/activity_count);
      values.insert("prb", (double)db.value(currentRNTI).value("prb")/activity_count);
      result.insert(currentRNTI, values);
      ++rnti_entry;
    }
    // Clear database for next sampling period
    db.clear();
    return result;
  }
private:
  QMap<uint32_t, QMap < QString, uint32_t > > db;
};



class RNTITable : public QObject{
  Q_OBJECT
public:
  // Handle pointers on Main windows settings, spectrumAdapter and mdiArea
  explicit RNTITable(Settings* p_glob_settings, SpectrumAdapter *spectrumAdapter, QMdiArea *mdiArea);
  ~RNTITable();

  void activate();
  void deactivate();

private:
  // Main window context
  Settings* glob_settings = nullptr;
  SpectrumAdapter* spectrumAdapter = nullptr;
  QMdiArea* mdiArea = nullptr;

  // View context
  QVBoxLayout *m_layout = new QVBoxLayout;
  QWidget *m_window = nullptr;
  QMdiSubWindow *m_subwindow = nullptr;

  // Toolbox context
  void setup_toolbox();
  QHBoxLayout *m_tb_layout = new QHBoxLayout;
  QWidget *m_toolbox = nullptr;
  QSlider *update_interval_slider = nullptr;
  QLabel *interval_label;
  QLabel *interval_label_text;

  // Table context
  void resize();
  QTableWidget *table = nullptr;
  QHeaderView *table_horizontal_header = nullptr;
  QHeaderView *table_vertical_header = nullptr;
  
  // Sampling context
  QTimer fps_timer;
  RNTITableDataStorage* db;
  std::mutex db_mutex;
  
private slots:
  void handle_data(const ScanLineLegacy*);
  void refreshTable();
  void updateCurrentColumn(int logicalIndex);
  void updateCurrentColumnByCell();
  void restartTimer(int interval);
};

#endif // RNTI_TABLE_H
