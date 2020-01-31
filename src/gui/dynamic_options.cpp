/*
 * Copyright (c) 2019 Robert Falkenberg.
 *
 * This file is part of FALCON
 * (see https://github.com/falkenber9/falcon).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * A copy of the GNU Affero General Public License can be found in
 * the LICENSE file in the top-level directory of this distribution
 * and at http://www.gnu.org/licenses/.
 */
#include "mainwindow.h"
#include "ui_mainwindow.h"


void MainWindow::handle_dl_alloc(bool nexist){
  if(nexist){
    //  If dl allocation object does not exist (by flag), create a new one and activate it instantly if eye is active
    //  Additionally ensure that if there is an old object, it gets deleted first before creating a new one.
    if(dl_alloc != nullptr){delete dl_alloc; dl_alloc = nullptr;}
    dl_alloc = new Waterfall_DL(&glob_settings, &spectrumAdapter, ui->mdiArea);
    if(active_eye){ dl_alloc->activate(); }
  }else{
    if (dl_alloc != nullptr){
      delete dl_alloc;
      dl_alloc = nullptr;
    }
  }
  ui->mdiArea->tileSubWindows();
}

void MainWindow::handle_ul_alloc(bool nexist){
  if(nexist){
    //  If ul allocation object does not exist (by flag), create a new one and activate it instantly if eye is active
    //  Additionally ensure that if there is an old object, it gets deleted first before creating a new one.
    if(ul_alloc != nullptr){delete ul_alloc; ul_alloc = nullptr;}
    ul_alloc = new Waterfall_UL(&glob_settings, &spectrumAdapter, ui->mdiArea);
    if(active_eye){ ul_alloc->activate(); }
  }else{
    if (ul_alloc != nullptr){
      delete ul_alloc;
      ul_alloc = nullptr;
    }
  }
  ui->mdiArea->tileSubWindows();
}

void MainWindow::handle_diff_alloc(bool nexist){
  if(nexist){
    //  If diff object does not exist (by flag), create a new one and activate it instantly if eye is active
    //  Additionally ensure that if there is an old object, it gets deleted first before creating a new one.
    if(diff_alloc != nullptr){delete diff_alloc; diff_alloc = nullptr;}
    diff_alloc = new Waterfall_DIFF(&glob_settings, &spectrumAdapter, ui->mdiArea);
    if(active_eye){ diff_alloc->activate(); }
  }else{
    if (diff_alloc != nullptr){
      delete diff_alloc;
      diff_alloc = nullptr;
    }
  }
  ui->mdiArea->tileSubWindows();
}

void MainWindow::handle_dl_spec(bool nexist){
  if(nexist){
    //  If spectrum object does not exist (by flag), create a new one and activate it instantly if eye is active
    //  Additionally ensure that if there is an old object, it gets deleted first before creating a new one.
    if(dl_spec != nullptr){delete dl_spec; dl_spec = nullptr;}
    dl_spec = new Waterfall_SPEC(&glob_settings, &spectrumAdapter, ui->mdiArea);
    connect(cp->get_color_range_slider(),SIGNAL(secondValueChanged(int)),SLOT(range_slider_value_changed(int)));
    connect(cp->get_color_range_slider(),SIGNAL(firstValueChanged(int)),SLOT(range_slider_value_changed(int)));
    if(active_eye){
      dl_spec->activate();
      this->range_slider_value_changed(0);  // initially apply slider values
    }
  }else{
    if (dl_spec != nullptr){
      disconnect(dl_spec);
      delete dl_spec;
      dl_spec = nullptr;
    }
  }
  ui->mdiArea->tileSubWindows();
}

void MainWindow::handle_rnti_table(bool nexist){
  if(nexist){
    //  If rnti table object does not exist (by flag), create a new one and activate it instantly if eye is active
    //  Additionally ensure that if there is an old object, it gets deleted first before creating a new one.
    if(rnti_table != nullptr){delete rnti_table; rnti_table = nullptr;}
    rnti_table = new RNTITable(&glob_settings, &spectrumAdapter, ui->mdiArea);
    if(active_eye){ rnti_table->activate(); }
  }else{
    if (rnti_table != nullptr){
      delete rnti_table;
      rnti_table = nullptr;
    }
  }
  ui->mdiArea->tileSubWindows();
}

void MainWindow::handle_perf_plot(bool nexist){
  if(nexist){
    //  If perf plot object does not exist (by flag), create a new one and activate it instantly if eye is active
    //  Additionally ensure that if there is an old object, it gets deleted first before creating a new one.
    if(perf_plot != nullptr){delete perf_plot; perf_plot = nullptr;}
    perf_plot = new PerformancePlot(&glob_settings, &spectrumAdapter, ui->mdiArea);
    connect(cp, SIGNAL(color_change()),perf_plot, SLOT(update_plot_color()));
    if(active_eye){ perf_plot->activate(); }
  }else{
    if (perf_plot != nullptr){
      disconnect(perf_plot);
      delete perf_plot;
      perf_plot = nullptr;
    }
  }
  ui->mdiArea->tileSubWindows();
}

void MainWindow::range_slider_value_changed(int value){
  if(dl_spec != nullptr){
    Waterfall_SPEC* spec = dynamic_cast<Waterfall_SPEC*>(dl_spec);
    spec->range_slider_value_changed(cp->get_color_range_slider()->firstValue() , cp->get_color_range_slider()->secondValue());
  }
}
