#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>

#include <qwt.h>
#include <qwt_plot_curve.h>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>		/* open */
#include <unistd.h>		/* exit */
#include <sys/ioctl.h>		/* ioctl */

#include "../module/ni4050.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    class  RangeDisplayInfo
    {
    public:
        RangeDisplayInfo(QString name, NI4050_RANGES range) {rangeName = name; rangeValue = range;}
        QString rangeName;
        NI4050_RANGES rangeValue;
    };

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
    QList<RangeDisplayInfo> rangeDisplayData;
    int fd;
    void fillRangeCombobox();
    void fillDeviceComboBox();

    QTimer timer;
    bool contRunning;

    QwtPlotCurve outCurve;
    QVector <QPointF> valueData;
};

#endif // MAINWINDOW_H
