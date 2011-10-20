#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "formrespect.h"
#include "librespect.h"
#include <sndfile.h>
#include <QFileDialog>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    plot = ui->widget;

    connect(plot->view(), SIGNAL(selectedItemChanged(QString)),
            ui->plainTextEdit, SLOT(setPlainText(QString)));

    connect(plot->view(), SIGNAL(selectedItemChanged(QString)),
            ui->plainTextEdit, SLOT(setPlainText(QString)));

    connect(ui->plainTextEdit, SIGNAL(textChanged()),
            plot->view(), SLOT(selectedTextChanged()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setPixel(QImage *image, double i, int x, int y)
{
    int m;
    if (i>1.0)
        m = 255;
    else
        m = i * 222 + 20;
    image->setPixel(x,y,QColor(m,m,m).rgb());
}

void MainWindow::on_actionOpen_File_triggered()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("Open File"), QDir::homePath(),
                                                    tr("WAV files")+" (*.wav)");

    SF_INFO* SF_info = new SF_INFO;
    SF_info->format = 0;
    SNDFILE* snd_file = sf_open(filePath.toUtf8(),SFM_READ,SF_info);
    if (snd_file==NULL)
    {
        QMessageBox::critical(this, tr("Open File Error"), tr("Invalid WAVE file"));
        return;
    }
    double sndTime = (double) SF_info->frames / SF_info->channels / SF_info->samplerate;
    int sndFrequency = SF_info->samplerate / 2;
    plot->setFileTime(sndTime);
    plot->setFileFreq(sndFrequency);
    plot->view()->setMaxTime(sndTime);
    plot->view()->setMaxFreq(sndFrequency);
    QString statusText = QString::number(sndTime,'g',6) + " s   " +
                         QString::number(sndFrequency) + " Hz";
    statusBar()->showMessage(statusText);

    quint16 bufferSize = 512;
    quint16 halfBufferSize = bufferSize * 0.5;
    double* arrayPreviousRead = new double [halfBufferSize];
    double* arrayRead = new double [halfBufferSize];
    double* arrayFFT = new double [bufferSize];
    LibReSpect spect;
    quint32 framesBuffer = SF_info->frames / halfBufferSize + 1;
    QImage image(framesBuffer,halfBufferSize,QImage::Format_ARGB32);

    // First samples counting
    sf_read_double(snd_file,arrayFFT,bufferSize);
    for (quint16 i=0, j=halfBufferSize; i<halfBufferSize; i++, j++)
        arrayPreviousRead[i] = arrayFFT[j];
    spect.makeWindow(arrayFFT);
    spect.countFFT(arrayFFT);
    for (quint16 j=0, k=halfBufferSize-1; j<halfBufferSize; j++, k--)
    {
        int m;
        if (arrayFFT[j]>1.0)
        {
            m = 255;
        }
        else
            m = arrayFFT[j] * 254;
        image.setPixel(0,k,QColor(m,m,m).rgb());
    }

    // Next samples counting
    for (quint16 i=1; i<framesBuffer; i++)
    {
        int count = sf_read_double(snd_file,arrayRead,halfBufferSize);
        for (quint16 j=0, k=halfBufferSize; j<halfBufferSize; j++,k++)
        {
            arrayFFT[j] = arrayPreviousRead[j];
            arrayFFT[k] = arrayRead[j];
            arrayPreviousRead[j] = arrayRead[j];
        }
        spect.makeWindow(arrayFFT);
        spect.countFFT(arrayFFT);
        for (quint16 j=0, k=halfBufferSize-1; j<halfBufferSize; j++, k--)
        {
            int m;
            if (arrayFFT[j]>1.0)
            {
                m = 255;
            }
            else
                m = arrayFFT[j] * 222 + 20;
            image.setPixel(i,k,QColor(m,m,m).rgb());
        }
    }

    plot->setupView(image);

    delete [] arrayPreviousRead;
    delete [] arrayRead;
    delete [] arrayFFT;
    delete SF_info;
}
