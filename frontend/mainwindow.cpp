#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QPen>
#include <QDir>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    fd(-1)
{
    ui->setupUi(this);

    fillRangeCombobox();
    fillDeviceComboBox();

    contRunning = false;

    outCurve.setTitle("Output data");
    outCurve.setSamples(valueData);
    outCurve.attach(ui->qwtPlot);
    outCurve.setPen(QPen(Qt::green));

    ui->qwtPlot->setAxisAutoScale(0);
    ui->qwtPlot->setAxisAutoScale(1);

    ui->qwtPlot->setAxisTitle(1, tr("Time"));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButtonOpen_clicked()
{
    if (fd == -1) {
        if ((fd = ::open(ui->comboBoxDevice->currentText().toAscii() ,O_RDWR)) != -1) {
            ui->pushButtonOpen->setText(tr("Close"));
            ui->pushButtonStartMeasurement->setEnabled(true);
            return;
        } else {
            qWarning() << "failed to open" << fd;
            ::close(fd);
            fd = -1;
        }
    } else {
        ::close(fd);
        fd = -1;
        ui->pushButtonOpen->setText(tr("Open"));
    }
    ui->pushButtonStartMeasurement->setEnabled(false);
}

void MainWindow::on_pushButtonReadValue_clicked()
{
    ui->doubleSpinBoxValue->setValue(0);
    double value = 0;

    if (fd != -1) {
        if (ui->checkBoxReadContinously->isChecked()) {
            contRunning = !contRunning;
            if (contRunning) {
                if (ioctl(fd, NIDMM_IOCREADDATA, &value) != -1) {
                    ui->doubleSpinBoxValue->setValue(value);
                    QTimer::singleShot(ui->doubleSpinBoxInterval->value()*1000, this, SLOT(timeOut()));
                    ui->pushButtonReadValue->setText(tr("Stop reading"));
                }
            } else {
                ui->pushButtonReadValue->setText(tr("Start reading"));
            }
        } else {
            if (ioctl(fd, NIDMM_IOCREADDATA, &value) != -1) {
                ui->doubleSpinBoxValue->setValue(value);
            }
        }
    }
}

void MainWindow::on_pushButtonStartMeasurement_clicked()
{
    NI4050_RANGES range = (NI4050_RANGES)ui->comboBoxMeasurementMode->itemData(ui->comboBoxMeasurementMode->currentIndex()).toInt();

    if (fd != -1) {
        if (ioctl(fd, NIDMM_IOCSTARTMEASUREMENT, &range) != -1) {
            ui->pushButtonReadValue->setEnabled(true);
        }
    }

    ui->qwtPlot->setAxisTitle(0, measurementModes[ui->comboBoxMeasurementMode->currentIndex()].title);
}

void MainWindow::fillRangeCombobox()
{
    int i = 0;
    ui->comboBoxMeasurementMode->clear();
    while (measurementModes[i].range != NI4050_RANGE_INVALID) {
        ui->comboBoxMeasurementMode->addItem(measurementModes[i].name, measurementModes[i].range);
        i++;
    }
}

void MainWindow::fillDeviceComboBox()
{
    QDir devDir("/dev/");
    QStringList devList = devDir.entryList(QStringList("nidmm*"), QDir::System | QDir::NoDotAndDotDot, QDir::Name);
    foreach (QString str, devList) {
        ui->comboBoxDevice->addItem("/dev/"+str);
    }
}

void MainWindow::on_checkBoxReadContinously_toggled(bool checked)
{
    ui->doubleSpinBoxInterval->setEnabled(checked);
    if (checked) {
        ui->pushButtonReadValue->setText("Start reading");
    } else {
        ui->pushButtonReadValue->setText("Read value");
    }
}

void MainWindow::on_comboBoxMeasurementMode_activated(const QString &arg1)
{

}

void MainWindow::timeOut()
{
    double value = 0;
    if (ui->checkBoxReadContinously->isChecked() && contRunning) {
        if (fd != -1) {
            if (ioctl(fd, NIDMM_IOCREADDATA, &value) != -1) {
                ui->doubleSpinBoxValue->setValue(value);
                QTimer::singleShot(ui->doubleSpinBoxInterval->value()*1000, this, SLOT(timeOut()));
                if (ui->checkBoxPlotNeeded->isChecked()) {
                    QPointF pt;
                    double x = 0;
                    if (valueData.size())
                        x = valueData.last().x();
                    pt.setX(x + ui->doubleSpinBoxInterval->value()*1000);
                    pt.setY(value);
                    valueData.append(pt);
                    outCurve.setSamples(valueData);

                    ui->qwtPlot->replot();
                }
            }
        }
    }
}

void MainWindow::on_pushButtonClearPlot_clicked()
{
    valueData.clear();
    outCurve.setSamples(valueData);
    ui->qwtPlot->replot();
}
