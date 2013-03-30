#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <qwt/qwt.h>
#include <qwt/qwt_plot_curve.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>		/* open */
#include <unistd.h>		/* exit */
#include <sys/ioctl.h>		/* ioctl */

#include "../module/ni4050.h"

typedef struct MeasurementMode_t {
    QString name;
    QString suffix;
    QString title;
    NI4050_RANGES range;
} MeasurementMode;

const MeasurementMode measurementModes[] = {
    {"250VDC", "V", "Voltage", NI4050_RANGE_250VDC},
    {"25VDC", "V", "Voltage", NI4050_RANGE_25VDC},
    {"2VDC", "V", "Voltage",NI4050_RANGE_2VDC},
    {"200mVDC", "mV", "Voltage",NI4050_RANGE_200mVDC},
    {"20mVDC", "mV", "Voltage",NI4050_RANGE_20mVDC},
    {"250VAC", "V", "Voltage",NI4050_RANGE_250VAC},
    {"25VAC", "V", "Voltage",NI4050_RANGE_25VAC},
    {"2VAC", "V", "Voltage",NI4050_RANGE_2VAC},
    {"200mVAC", "mV", "Voltage",NI4050_RANGE_200mVAC},
    {"20mVAC", "mV", "Voltage",NI4050_RANGE_20mVAC},
    {"EXTOHM",  "Ohm//FIXME", "Resistance",NI4050_RANGE_EXTOHM},
    {"2MOHM", "MOhm", "Resistance",NI4050_RANGE_2MOHM},
    {"200kOHM", "KOhm", "Resistance",NI4050_RANGE_200kOHM},
    {"20kOHM", "KOhm", "Resistance",NI4050_RANGE_20kOHM},
    {"2kOHM", "KOhm", "Resistance",NI4050_RANGE_2kOHM},
    {"200OHM", "Ohm", "Resistance",NI4050_RANGE_200OHM},
    {"DIODE", "V", "Foward voltage",NI4050_RANGE_DIODE},
    {"INVALID", "", "", NI4050_RANGE_INVALID}
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButtonOpen_clicked();

    void on_pushButtonReadValue_clicked();

    void on_pushButtonStartMeasurement_clicked();

    void on_checkBoxReadContinously_toggled(bool checked);
    void on_comboBoxMeasurementMode_activated(const QString &arg1);

    void timeOut();

    void on_pushButtonClearPlot_clicked();

private:
    Ui::MainWindow *ui;
    int fd;
    void fillRangeCombobox();
    void fillDeviceComboBox();

    QTimer timer;
    bool contRunning;

    QwtPlotCurve outCurve;
    QVector <QPointF> valueData;
};

#endif // MAINWINDOW_H
