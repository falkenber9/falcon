#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <string>
#include <QDebug>
#include "file_input_output.h"
#include "falcon/definitions.h"
#include "falcon/meas/TrafficGenerator.h"
#include "falcon/meas/AuxModem.h"
#include "plots.h"

#include "qcustomplot/qcustomplot.h"

#include "settings.h"

struct ParamsContainer {
    char iqSamplesFilename[1024] = {0};
};

static ParamsContainer paramsContainer;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    //Create layers
    //spectrumAdapter already exists on stack
    decoderThread = new DecoderThread();  // ScanThread
    decoderThread->init();
    //Connect layers
    decoderThread->subscribe(&spectrumAdapter);

    ui->setupUi(this);
    ui->mdiArea->tileSubWindows();

    //Settings are initialised on startup in constructor of settings class.
    //Init Checkboxes:
    ui->actionDifference->          setChecked(glob_settings.glob_args.gui_args.show_diff);
    ui->actionDownlink->            setChecked(glob_settings.glob_args.gui_args.show_downlink);
    ui->actionSpectrum->            setChecked(glob_settings.glob_args.gui_args.show_spectrum);
    ui->actionUplink->              setChecked(glob_settings.glob_args.gui_args.show_uplink);

    ui->actionSave_Settings->       setChecked(glob_settings.glob_args.gui_args.save_settings);
    ui->actionUse_File_as_Source->  setChecked(glob_settings.glob_args.gui_args.use_file_as_source);
    ui->checkBox_FileAsSource ->    setChecked(glob_settings.glob_args.gui_args.use_file_as_source);

    chart_a_buffer = nullptr;
}

MainWindow::~MainWindow() {
    delete ui;
    delete[] chart_a_buffer;
    if(decoderThread != nullptr) {
        decoderThread->stop();
        decoderThread->unsubscribe(&spectrumAdapter);
        delete decoderThread;
        decoderThread = nullptr;
    }
}

void MainWindow::draw(float *inc, float *dec) {
    if(spectrum_view_on) {
        spectrum_view_ul->addLine(inc);
        spectrum_view_dl->addLine(dec);
        spectrum_view_ul->update();
        spectrum_view_dl->update();


        //Autoscaling for Spectrum ul

        if(a_window->size().height() != windowsize_tmp_a.height() ||
                a_window->size().width() != windowsize_tmp_a.width() )
        {
            spectrum_view_ul->setFixedSize(a_window->size().width(),a_window->size().height());

        }

        windowsize_tmp_a = a_window->size();

        //Autoscaling for Spectrum dl

        if(b_window->size().height() != windowsize_tmp_b.height() ||
                b_window->size().width() != windowsize_tmp_b.width() )
        {
            spectrum_view_dl->setFixedSize(a_window->size().width(),a_window->size().height());

        }

        windowsize_tmp_b = b_window->size();
    }
}

void MainWindow::draw_ul(const ScanLineLegacy *data) {
    if(glob_settings.glob_args.gui_args.show_uplink) {
        spectrum_view_ul->addLine(data->linebuf);
        spectrum_view_ul->update();

        //Autoscaling for Spectrum

        if(a_window->size().height() != windowsize_tmp_a.height() ||
                a_window->size().width() != windowsize_tmp_a.width() )
        {
            //  spectrum_view_ul->setFixedSize(a_window->size().width(),a_window->size().height() - 80);
            spectrum_view_ul->setFixedSize(a_window->size().width(),a_window->size().height());
            // chart_a_view->setGeometry(0,a_window->size().height() - 80 ,a_window->size().width() ,80);
        }
        windowsize_tmp_a = a_window->size();

    }

    delete data;  // Feld Löschen
}

void MainWindow::draw_dl(const ScanLineLegacy *data) {
    if(glob_settings.glob_args.gui_args.show_downlink) {
        spectrum_view_dl->addLine(data->linebuf);
        spectrum_view_dl->update();

        //Autoscaling for Spectrum

        if(b_window->size().height() != windowsize_tmp_b.height() ||
                b_window->size().width() != windowsize_tmp_b.width() )
        {
            spectrum_view_dl->setFixedSize(b_window->size().width(),b_window->size().height());
        }
        windowsize_tmp_b = b_window->size();
    }

    if(show_plot1){

        //Count active Blocks
        int counter = 0;

        for(int i = 0; i < SPECTROGRAM_LINE_WIDTH; i++){
            if(data->linebuf[i] > 0) counter++;
        }

        //add_data_to_plot(counter, plot_a_downlink ,chart_a_buffer);

        addData(RNTI_HIST, testplot, data);

        //autoscaling for PerformancePlot

        if(plot_a_window->size().height() != windowsize_tmp_plot_a.height() ||
                plot_a_window->size().width() != windowsize_tmp_plot_a.width() )
        {
            // chart_a_view->setFixedSize(plot_a_window->size().width(),plot_a_window->size().height());
        }
        windowsize_tmp_plot_a = plot_a_window->size();
    }

    delete data;
}

void MainWindow::draw_spectrum(const ScanLineLegacy *data){

    if(glob_settings.glob_args.gui_args.show_spectrum) {
        spectrum_view->addLine(data->linebuf);
        spectrum_view->update();

        //Autoscaling for Spectrum

        if(c_window->size().height() != windowsize_tmp_c.height() ||
                c_window->size().width() != windowsize_tmp_c.width() ){

            spectrum_view->setFixedSize(c_window->size().width(),c_window->size().height());
        }
        windowsize_tmp_c = c_window->size();
    }

    delete data;
}

void MainWindow::draw_spectrum_diff(const ScanLineLegacy *data){

    if(glob_settings.glob_args.gui_args.show_diff) {
        spectrum_view_diff->addLine(data->linebuf);
        spectrum_view_diff->update();

        //Autoscaling for Spectrum

        if(d_window->size().height() != windowsize_tmp_d.height() ||
                d_window->size().width() != windowsize_tmp_d.width() ){

            spectrum_view_diff->setFixedSize(d_window->size().width(),d_window->size().height());
        }
        windowsize_tmp_d = d_window->size();
    }

    delete data;

}

void MainWindow::on_actionNew_triggered() {
    if(spectrum_view_on) {
        qDebug() << "Window exists";
    }
    else {
        // Setup prog args (defaults)
        prog_args.input_file_name = nullptr;     // Values from Args_default;
        prog_args.file_cell_id    = 0;
        prog_args.file_nof_ports  = 1;
        prog_args.file_nof_prb    = 25;

        // Setup prog args (from GUI)
        prog_args.file_nof_ports  = static_cast<uint32_t>(ui->spinBox_Ports->value());
        prog_args.file_cell_id    = ui->spinBox_CellId->value();
        prog_args.file_nof_prb    = ui->spinBox_Prb->value();

        // Setup prog args (from file, if requested)
        if(ui->checkBox_FileAsSource->isChecked()){
            QString filename = ui->lineEdit_FileName->text();
            if(!get_infos_from_file(filename, prog_args)) {
                qDebug() << "Could not load parameters from file source" << endl;
                return;
            }
        }

        //Init Adapters:

        spectrumAdapter.emit_uplink     = false;
        spectrumAdapter.emit_downlink   = false;
        spectrumAdapter.emit_spectrum   = false;
        spectrumAdapter.emit_difference = false;
        spectrumAdapter.emit_plot1      = false;

        //Start Windows:

        //Uplink:
        if(glob_settings.glob_args.gui_args.show_uplink){
            a_window = new QWidget();
            a_window->setObjectName("Uplink");
            a_window->setWindowTitle("Uplink");

            //Insert Spectrum:
            spectrum_view_ul = new Spectrum(a_window);
            spectrum_view_ul->setObjectName("Spectrum View UL");
            spectrum_view_ul->setGeometry(0,0,100,100);

            //Insert performance Plot:
            /*  if(show_plot1){
                chart_a_view = new QChartView(a_window);
                chart_a_view->setObjectName("Chart A");
                chart_a_view->setGeometry(0,0,100,100);

                QLineSeries *series = new QLineSeries();

                series->append(0, 6);
                series->append(2, 4);
                series->append(3, 8);
                series->append(7, 4);
                series->append(10, 5);
                *series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);

                plot_a_chart = new QChart();
                plot_a_chart->legend()->hide();
                plot_a_chart->addSeries(series);
                plot_a_chart->createDefaultAxes();
                plot_a_chart->setTitle("Simple line chart example");

                chart_a_view->setChart(plot_a_chart);


            }*/




            ui->mdiArea->addSubWindow(a_window);
            a_window->show();

            spectrumAdapter.emit_uplink = true;
            connect (&spectrumAdapter, SIGNAL(update_ul(const ScanLineLegacy*)),SLOT(draw_ul(const ScanLineLegacy*)));

        }
        // else disconnect(&spectrumAdapter, SIGNAL(update_ul(ScanLine*)),SLOT(draw_ul(ScanLine*)));

        //Downlink:
        if(glob_settings.glob_args.gui_args.show_downlink){
            b_window = new QWidget();
            b_window->setObjectName("Downlink");
            b_window->setWindowTitle("Downlink");
            spectrum_view_dl = new Spectrum(b_window);

            spectrum_view_dl->setObjectName("Spectrum View DL");
            spectrum_view_dl->setGeometry(0,0,100,100);

            ui->mdiArea->addSubWindow(b_window);
            b_window->show();

            spectrumAdapter.emit_downlink = true;
            connect (&spectrumAdapter, SIGNAL(update_dl(const ScanLineLegacy*)),SLOT(draw_dl(const ScanLineLegacy*)));
        }
        //else disconnect (&spectrumAdapter, SIGNAL(update_dl(ScanLine*)),SLOT(draw_dl(ScanLine*)),&b_window);

        // Pure Spectrum:
        if(glob_settings.glob_args.gui_args.show_spectrum){

            c_window = new QWidget();
            c_window->setObjectName("Spectrum");
            c_window->setWindowTitle("Spectrum");
            spectrum_view = new Spectrum(c_window);

            spectrum_view->setObjectName("Spectrum View");
            spectrum_view->setGeometry(0,0,100,100);

            ui->mdiArea->addSubWindow(c_window);
            c_window->show();

            spectrumAdapter.emit_spectrum = true;
            connect (&spectrumAdapter, SIGNAL(update_spectrum(const ScanLineLegacy*)),SLOT(draw_spectrum(const ScanLineLegacy*)));
        }
        //else disconnect (&spectrumAdapter, SIGNAL(update_spectrum(ScanLine*)),SLOT(draw_spectrum(ScanLine*)));

        // Spectrum - Downlink
        if(glob_settings.glob_args.gui_args.show_diff){

            d_window = new QWidget();
            d_window->setObjectName("Spectrum Diff");
            d_window->setWindowTitle("Spectrum Difference");
            spectrum_view_diff = new Spectrum(d_window);

            spectrum_view_diff->setObjectName("Spectrum View Diff");
            spectrum_view_diff->setGeometry(0,0,100,100);

            ui->mdiArea->addSubWindow(d_window);
            d_window->show();

            spectrumAdapter.emit_difference = true;
            connect (d_window, SIGNAL(destroyed()),SLOT(spectrum_window_destroyed()));
            connect (&spectrumAdapter, SIGNAL(update_spectrum_diff(const ScanLineLegacy*)),SLOT(draw_spectrum_diff(const ScanLineLegacy*)));

        }
        //else disconnect (&spectrumAdapter, SIGNAL(update_spectrum_diff(ScanLine*)),SLOT(draw_spectrum_diff(ScanLine*)));

        // Performance Plot extern:
        if(show_plot1){

            //Generate Window PLOT_A_WINDOW:
            plot_a_window = new QWidget();
            plot_a_window->setObjectName("plot a");
            plot_a_window->setWindowTitle("PLOT A");

            testplot = new QCustomPlot(plot_a_window);
            testplot->setObjectName(QStringLiteral("customPlot"));
            testplot->setGeometry(0,0,400,200);
            setupPlot(RNTI_HIST, testplot);

            //Add Subwindow to MDI Area
            ui->mdiArea->addSubWindow(plot_a_window);
            plot_a_window->show();

        }


        // Organise Windows:

        ui->mdiArea->tileSubWindows();

        spectrum_view_on = true;
        //spectrum_view_ul->setFixedSize(a_window->size().width(),a_window->size().height());
        //spectrum_view_dl->setFixedSize(b_window->size().width(),b_window->size().height());

        //connect (a_window, SIGNAL(destroyed()),SLOT(spectrum_window_destroyed()));
        //connect (b_window, SIGNAL(destroyed()),SLOT(spectrum_window_destroyed()));

        decoderThread->start();

        qDebug() << "Spectrum View on";
    }
}

void MainWindow::spectrum_window_destroyed() {
    on_actionStop_triggered();
}

void MainWindow::got_update(){






}

void MainWindow::on_actionStop_triggered()
{
    decoderThread->stop();
    ui->mdiArea->closeAllSubWindows();
    spectrum_view_on = false;
    spectrumAdapter.disconnect();  //Disconnect all Signals
}

void MainWindow::add_data_to_plot(int data, QLineSeries *series, int *buffer){

    series->clear();
    buffer[chart_a_position] = data;

    for(int i = chart_a_size - 1; i >= 0 ; i--){

        if(i + chart_a_position <= chart_a_size){
            series->append(i,buffer[i + chart_a_position]);
        }
        else{
            series->append(i,buffer[i + chart_a_position - chart_a_size]);
        }
    }
    if(chart_a_position > 0)chart_a_position--;
    else chart_a_position = chart_a_size;

    //qDebug ()<< "Series: " <<series->count();

}

/*void MainWindow::subwindow_resized(QWidget *window){




}*/

void MainWindow::draw_rnti_hist(const ScanLineLegacy *data){

    /*for(int i = 0; i < 65000; i++){
    if(data->rnti_hist[i] > 10) qDebug() << " RNTI: "<< data->rnti_hist[i];
  }*/
    delete data;


}

void MainWindow::on_Select_file_button_clicked()
{
    qDebug () << "Clicked Select File";
    FileDialog input_file;
    ui->lineEdit_FileName->setText(input_file.openFile());
}

void MainWindow::on_lineEdit_FileName_textChanged(const QString &arg1)
{

    QString buffer_string;

    buffer_string = ui->lineEdit_FileName->text();

    if(buffer_string.contains("file://")){

        buffer_string.remove("file://");
        ui->lineEdit_FileName->setText(buffer_string);
    }

    //qDebug() <<"Buffer String: "<< buffer_string;

    get_infos_from_file(buffer_string, prog_args);

}

bool MainWindow::get_infos_from_file(QString filename, volatile prog_args_t& args) {

    qDebug() << "Filename: " << filename;

    bool no_proberesult = false;
    bool no_networkinfo = false;

    QString basename = filename;
    if(basename.contains("-iq.bin") > 0) {
        basename.remove("-iq.bin");
    }
    else if(basename.contains("-traffic.csv") > 0) {
        basename.remove("-traffic.csv");
    }
    else if(basename.contains("-cell.csv") > 0) {
        basename.remove("-cell.csv");
    }

    QString probeResultFilename = basename + "-traffic.csv";
    QString cellInfoFilename = basename + "-cell.csv";
    QString iqSamplesFilename = basename + "-iq.bin";

    qDebug() << "probeResultFilename: " << probeResultFilename;
    qDebug() << "cellInfoFilename: " << cellInfoFilename;
    qDebug() << "iqSamplesFilename: " << iqSamplesFilename;

    ProbeResult probeResult;
    QFile probeResultFile(probeResultFilename);
    if(probeResultFile.open(QIODevice::ReadOnly)) {
        QTextStream linestream(&probeResultFile);
        //while(!linestream.atEnd()) {
        if(!linestream.atEnd()) { // no loop, only first line
            QString line = linestream.readLine();
            probeResult.fromCSV(line.toStdString(), ',');
            qDebug() << QString::fromStdString(probeResult.toCSV(','));
        }
        probeResultFile.close();
    }
    else {
        qDebug () << "Could not open probeResultFile: " << probeResultFilename << endl;
        no_proberesult = true;
    }

    NetworkInfo networkInfo;
    QFile cellInfoFile(cellInfoFilename);
    if(cellInfoFile.open(QIODevice::ReadOnly)) {
        QTextStream linestream(&cellInfoFile);
        //while(!linestream.atEnd()) {
        if(!linestream.atEnd()) { // no loop, only first line
            QString line = linestream.readLine();
            networkInfo.fromCSV(line.toStdString(), ',');
            qDebug() << QString::fromStdString(networkInfo.toCSV(','));
        }
        cellInfoFile.close();
    }
    else {
        qDebug () << "Could not open cellInfoFile: " << cellInfoFilename << endl;
        no_networkinfo = true;
    }

    strcpy(paramsContainer.iqSamplesFilename, iqSamplesFilename.toLatin1().data());
    args.input_file_name = paramsContainer.iqSamplesFilename;
    if(!no_networkinfo){
        args.file_cell_id = static_cast<uint32_t>(networkInfo.lteinfo->pci);
        ui->spinBox_CellId->setValue(networkInfo.lteinfo->pci);
    }
    if(!no_proberesult){
        args.file_nof_prb = networkInfo.nof_prb;
        ui->spinBox_Prb->setValue(networkInfo.nof_prb);
    }

    return true;
}

void MainWindow::mousePressEvent(QMouseEvent *event){

    if(spectrum_view_on){
        if(glob_settings.glob_args.gui_args.show_downlink){
            spectrum_view_dl->paused = !spectrum_view_dl->paused;
            spectrum_view_dl->view_port = SPECTROGRAM_LINE_COUNT - SPECTROGRAM_LINE_SHOWN - 1;
        }
        if(glob_settings.glob_args.gui_args.show_uplink){
            spectrum_view_ul->paused = !spectrum_view_ul->paused;
            spectrum_view_ul->view_port = SPECTROGRAM_LINE_COUNT - SPECTROGRAM_LINE_SHOWN - 1;
        }
        if(glob_settings.glob_args.gui_args.show_diff){
            spectrum_view_diff->paused = !spectrum_view_diff->paused;
            spectrum_view_diff->view_port = SPECTROGRAM_LINE_COUNT - SPECTROGRAM_LINE_SHOWN - 1;
        }
        if(glob_settings.glob_args.gui_args.show_spectrum){
            spectrum_view->paused = !spectrum_view->paused;
            spectrum_view->view_port = SPECTROGRAM_LINE_COUNT - SPECTROGRAM_LINE_SHOWN - 1;
        }
    }
}

void MainWindow::wheelEvent(QWheelEvent *event){
    if(spectrum_view_on){
        if(glob_settings.glob_args.gui_args.show_downlink){
            if(spectrum_view_dl->paused){
                if(event->delta() > 0) spectrum_view_dl->scroll_up();
                else spectrum_view_dl->scroll_down();
            }
        }
        if(glob_settings.glob_args.gui_args.show_uplink){
            if(spectrum_view_ul->paused){
                if(event->delta() > 0) spectrum_view_ul->scroll_up();
                else spectrum_view_ul->scroll_down();
            }
        }
        if(glob_settings.glob_args.gui_args.show_diff){
            if(spectrum_view_diff->paused){
                if(event->delta() > 0) spectrum_view_diff->scroll_up();
                else spectrum_view_diff->scroll_down();
            }
        }
        if(glob_settings.glob_args.gui_args.show_spectrum){
            if(spectrum_view->paused){
                if(event->delta() > 0) spectrum_view->scroll_up();
                else spectrum_view->scroll_down();
            }
        }
    }
}

