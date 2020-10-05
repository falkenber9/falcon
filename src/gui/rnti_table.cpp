#include "rnti_table.h"
#include "falcon/prof/Lifetime.h"


#define TABLE_UPDATE_INTERVAL_GRANULARITY 100 // granularity, the timer can be adjusted per slider

RNTITable::RNTITable(Settings* p_glob_settings, SpectrumAdapter *p_spectrumAdapter, QMdiArea *p_mdiArea) :
  glob_settings(p_glob_settings),
  spectrumAdapter(p_spectrumAdapter),
  mdiArea(p_mdiArea){
  db = new RNTITableDataStorage();
  m_window = new QWidget();
  m_window->setObjectName("RNTI Table");
  m_window->setWindowTitle("UE Activity");

  table = new QTableWidget(0, 5, m_window);

  m_layout->addWidget(table);
  m_window->setLayout(m_layout);
  m_window->setSizePolicy(QSizePolicy ::Ignored , QSizePolicy ::Ignored );

  m_subwindow = mdiArea->addSubWindow(m_window, Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  m_subwindow->showMaximized();
  
  QStringList labels = { "RNTI", "Activity", "TBS (avg)", "MCS (avg)", "PRB (avg)"};
  table->setHorizontalHeaderLabels(labels);

  m_toolbox = new QWidget();
  setup_toolbox();
  m_layout->insertWidget(0, m_toolbox);
  table_horizontal_header = qobject_cast<QTableView *>(table)->horizontalHeader();
  table_vertical_header = qobject_cast<QTableView *>(table)->verticalHeader();
  m_window->show();
  table->setEditTriggers(QAbstractItemView::NoEditTriggers);
  resize();
}

RNTITable::~RNTITable() {
  if(table!=nullptr) delete table;
  if(db!=nullptr) delete db;
  if(update_interval_slider != nullptr) delete update_interval_slider;
  
  if(mdiArea != nullptr){ // mdiArea != nullptr --> was activated at least once
    //Close Subwindow:
    mdiArea->setActiveSubWindow(m_subwindow);
    mdiArea->removeSubWindow(m_subwindow);
  }
}

void RNTITable::resize(){
  int availableWidth =  m_window->size().width();
  int verticalHeadersize;
  if (table->rowCount() == 0){
    verticalHeadersize = 0;
  }else{
    verticalHeadersize = table_vertical_header->sectionSize(0) + 9; // +9 for vertical scrollbar, which is only visible for rowCount > 0
  }
  for(int c = 0; c < table->columnCount(); c++){
    table->setColumnWidth(c,(availableWidth - verticalHeadersize)/table->columnCount() - table->columnCount());
  }
}

void RNTITable::activate(){
  connect (&fps_timer, SIGNAL(timeout()), this, SLOT(refreshTable()));
  fps_timer.start(update_interval_slider->value()*TABLE_UPDATE_INTERVAL_GRANULARITY);
  connect(spectrumAdapter, SIGNAL(update_table(const ScanLineLegacy*)), SLOT(handle_data(const ScanLineLegacy*)));
  connect(table, SIGNAL(cellDoubleClicked(int,int)), SLOT(updateCurrentColumnByCell()));
  connect(table_horizontal_header, SIGNAL(sectionClicked(int)),  SLOT(updateCurrentColumn(int)));
  spectrumAdapter->emit_table_update = true;
}

void RNTITable::deactivate(){
  disconnect(spectrumAdapter, SIGNAL(update_table(const ScanLineLegacy*)), this,SLOT(handle_data(const ScanLineLegacy*)));
  fps_timer.stop();
  fps_timer.disconnect();
}

void RNTITable::refreshTable(){
  db_mutex.lock();
  QMap<uint32_t, QMap < QString, double > > res = db->calcAndPurge();
  db_mutex.unlock();
  
  table->clearContents();
  while(table->rowCount() != 0){table->removeRow(0);}
  QMap<uint32_t, QMap < QString, double > >::const_iterator i = res.constBegin();
  
  while (i != res.constEnd()) {
    table->insertRow(0);
    // RNTI entry
    table->setItem(0,0,new QTableWidgetItem());
    table->item(0,0)->setData(Qt::EditRole, i.key());
    // Activity entry
    table->setItem(0,1,new QTableWidgetItem());
    table->item(0,1)->setData(Qt::EditRole, res.value(i.key() ).value("activity"));
    // TBS entry
    table->setItem(0,2,new QTableWidgetItem());
    table->item(0,2)->setData(Qt::EditRole, res.value(i.key() ).value("tbs"));
    // MCS entry
    table->setItem(0,3,new QTableWidgetItem());
    table->item(0,3)->setData(Qt::EditRole, res.value(i.key() ).value("mcs"));
    // PRB entry
    table->setItem(0,4,new QTableWidgetItem());
    table->item(0,4)->setData(Qt::EditRole, res.value(i.key() ).value("prb"));

    ++i;
  }
  table->sortByColumn(glob_settings->glob_args.gui_args.sort_by_column, Qt::SortOrder(glob_settings->glob_args.gui_args.sort_order));
  resize();
}

void RNTITable::handle_data(const ScanLineLegacy *data){
  uint32_t sfn = data->sfn;
  uint32_t sf_idx = data->sf_idx;
  uint32_t mcs_idx = data->mcs_idx;
  int mcs_tbs = data->mcs_tbs;
  uint32_t l_prb = data->l_prb;
  uint32_t rnti = data->rnti;

  db_mutex.lock();
  db->insert(rnti, mcs_tbs, mcs_idx, l_prb);
  db_mutex.unlock();

  delete data;
}

void RNTITable::updateCurrentColumn(int logicalIndex){
  if (0 <= logicalIndex < table->columnCount()){  // Validate column, just in case something went wrong
    if (logicalIndex == glob_settings->glob_args.gui_args.sort_by_column){
      glob_settings->glob_args.gui_args.sort_order = (glob_settings->glob_args.gui_args.sort_order + 1)%2;  // Orders are an enum with just 0 (Ascending) and 1 (Descending) as entry. Just use +1%2 to change between both options
    }
    glob_settings->glob_args.gui_args.sort_by_column = logicalIndex;
  }
  if(glob_settings->glob_args.gui_args.save_settings)glob_settings->store_settings();  //If save_settings = true, save to file.
  table->sortByColumn(glob_settings->glob_args.gui_args.sort_by_column, Qt::SortOrder(glob_settings->glob_args.gui_args.sort_order));
}

void RNTITable::updateCurrentColumnByCell(){
  updateCurrentColumn(table->currentColumn());
}

void RNTITable::restartTimer(int interval){
  interval_label->setNum(update_interval_slider->value()*TABLE_UPDATE_INTERVAL_GRANULARITY);
  fps_timer.stop();
  fps_timer.start(interval*TABLE_UPDATE_INTERVAL_GRANULARITY);
}

void RNTITable::setup_toolbox(){
  update_interval_slider = new QSlider(m_toolbox);
  update_interval_slider->setGeometry(180, 600, 160, 20);
  update_interval_slider->setMaximumHeight(30);
  update_interval_slider->setMinimum(1);
  update_interval_slider->setMaximum(50);
  update_interval_slider->setValue(5);
  update_interval_slider->setOrientation(Qt::Horizontal);
  update_interval_slider->setSingleStep(1);

  interval_label_text = new QLabel(m_toolbox);
  interval_label_text->setGeometry(180, 600, 160, 20);
  interval_label_text->setText("Table update inverval (ms): ");
  interval_label_text->setFixedWidth(170);

  interval_label = new QLabel(m_toolbox);
  interval_label->setGeometry(180, 600, 160, 20);
  interval_label->setNum(update_interval_slider->value()*TABLE_UPDATE_INTERVAL_GRANULARITY);
  interval_label->setFixedWidth(50);

  m_tb_layout->addWidget(update_interval_slider);
  m_tb_layout->addWidget(interval_label_text);
  m_tb_layout->addWidget(interval_label);

  m_toolbox->setLayout(m_tb_layout);
  connect(update_interval_slider, SIGNAL(valueChanged(int)), SLOT(restartTimer(int)));

}
