#ifndef PLOT_H
#define PLOT_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"

#define CUSTOM_LINE_COLORS   14
#define GCP_CUSTOM_LINE_COLORS 4

namespace Ui {
class Plot;
}

class Plot : public QMainWindow
{
    Q_OBJECT

public:
    explicit Plot(QWidget *parent = nullptr);
    ~Plot();

private slots:

    void slot_plot_mouseWheel(QWheelEvent*);
    void slot_plot_mouseMove(QMouseEvent*);
    void slot_plot_selectionChangedByUser();
    void slot_plot_legendDoubleClick(QCPLegend*, QCPAbstractLegendItem*, QMouseEvent*);
    void slot_timerUpdatePlotr_timeout();

    void on_spinBoxXCurPos_valueChanged(int arg1);
    void on_spinBoxXPoints_valueChanged(int arg1);
    void on_spinBoxYMin_valueChanged(int arg1);
    void on_spinBoxYMax_valueChanged(int arg1);
    void on_spinBoxXTicks_valueChanged(int arg1);
    void on_spinBoxYTicks_valueChanged(int arg1);
    void on_checkBoxXTrackAixs_stateChanged(int arg1);
    void on_checkBoxYAutoScale_stateChanged(int arg1);
    void on_pushButtonYAutoScale_clicked();
    void on_listWidgetChannels_itemDoubleClicked(QListWidgetItem *item);
    void on_pushButtonShowAllCurve_clicked();
    void on_pushButtonClearAllCurve_clicked();

    void on_pushButtonStartPlot_clicked();

private:
    Ui::Plot *ui;

    /* Line colors */
    QColor line_colors[CUSTOM_LINE_COLORS];
    QColor gui_colors[GCP_CUSTOM_LINE_COLORS];

    /* Main info */                                                                      // Status connection variable
    bool flagPlotting;                                                                        // Status plotting variable
    qint64 dataPointNumber;                                                                  // Keep track of data points
    int channelNumber;


    QTimer timerUpdatePlot;

    void setupPlot();
public:
    void onNewDataArrived(QByteArray baRecvData);                                           // Slot for new data from serial port

};

#endif // PLOT_H
