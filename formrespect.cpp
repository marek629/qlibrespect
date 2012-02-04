#include "formrespect.h"
#include "wavefile.h"
#include <librespect.h>
#include <QScrollBar>
#include <QGraphicsPixmapItem>
#include <QDir>
#include <QMessageBox>
#include <QFileDialog>

#include <QDebug>

#define AXIS_MARGIN 5
#define AXIS_MARK_LENGTH 3
#define AXIS_WIDTH 2
#define SCROLLBAR_SIZE 20

QColor FormRespect::colorMin (Qt::black);
QColor FormRespect::colorMax (Qt::green);
QColor FormRespect::colorOverflow (Qt::red);

static const QString markerFileExt = ".smr";

FormRespect::FormRespect(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FormRespect)
{
    ui->setupUi(this);
    ui->view->setLayoutDirection(Qt::RightToLeft);
    ui->view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    ui->view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    timeAxisSpace = this->height() - ui->view->height() - ui->view->y();
    timeAxisY = this->height() - timeAxisSpace + AXIS_MARGIN;
    timeAxisStartX = ui->view->x() + SCROLLBAR_SIZE;
    timeMarkMinSpacer = 250;

    freqAxisSpace = ui->view->x();
    freqAxisX = ui->view->x() - AXIS_MARGIN;
    freqAxisStopY = ui->view->y() + AXIS_WIDTH/2;
    freqLabelX = freqAxisX - 38 - AXIS_MARK_LENGTH;
    freqMarkMinSpacer = 50;

    painter = new QPainter();
    scene = 0;

    file = 0;
    sndFileTime = 0.;
    sndFileFrequency = 0;
    ui->view->setMaxTime(0);
    ui->view->setMaxFreq(0);

    setupView();

    connect(ui->view->verticalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(update()));
    connect(ui->view->horizontalScrollBar(),SIGNAL(valueChanged(int)),this,SLOT(update()));
    connect(ui->view,SIGNAL(pixmapChanged()),this,SLOT(update()));
}

FormRespect::~FormRespect()
{
    delete ui;
    delete scene;
    delete painter;
    delete file;
}

void FormRespect::setupView(const QImage &image)
{
    if (scene != 0)
    {
        delete scene;
        scene = 0;
    }
    scene = new QGraphicsScene();
    ui->view->setScene(scene);
    ui->view->setPixmap(QPixmap::fromImage(image));
}

void FormRespect::setupView(const QString &filePath)
{
    if (file != 0) {
        delete file;
        file = 0;
    }
    file = new WaveFile(filePath,this);
    setupView(file);
}

void FormRespect::setupView(WaveFile *file)
{
    bool open = file->isOpen();
    if (!open && !file->open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, tr("Open File Error"),
                              tr("Open file failed. Check your permissions."));
        return;
    }
    double sndTime = file->realTime();
    int sndFrequency = file->maxFrequency();
    setFileTime(sndTime);
    setFileFreq(sndFrequency);
    view()->setMaxTime(sndTime);
    view()->setMaxFreq(sndFrequency);
    QString statusText = QString::number(sndTime,'g',6) + " s   " +
                         QString::number(sndFrequency) + " Hz";
    emit setStatusMessage(statusText);

    quint16 bufferSize = 512;
    quint16 halfBufferSize = bufferSize * 0.5;
    double* arrayPreviousRead = new double [halfBufferSize];
    double* arrayRead = new double [halfBufferSize];
    double* arrayFFT = new double [bufferSize];
    LibReSpect spect;
    quint32 framesBuffer = file->numSamples() / halfBufferSize;
    QImage image(framesBuffer,halfBufferSize,QImage::Format_ARGB32);

    // First samples counting
    qint64 readedBytes = file->readData(arrayFFT,bufferSize);
    for (quint16 i=0, j=halfBufferSize; i<halfBufferSize; i++, j++)
        arrayPreviousRead[i] = arrayFFT[j];
    spect.makeWindow(arrayFFT);
    spect.countFFT(arrayFFT);
    for (quint16 j=0, k=halfBufferSize-1; j<halfBufferSize; j++, k--)
        setPixel(&image,arrayFFT[j],0,k);

    // Next samples counting
    for (quint16 i=1; i<framesBuffer; i++)
    {
        readedBytes += file->readData(arrayRead,halfBufferSize);
        for (quint16 j=0, k=halfBufferSize; j<halfBufferSize; j++,k++)
        {
            arrayFFT[j] = arrayPreviousRead[j];
            arrayFFT[k] = arrayRead[j];
            arrayPreviousRead[j] = arrayRead[j];
        }
        spect.makeWindow(arrayFFT);
        spect.countFFT(arrayFFT);
        for (quint16 j=0, k=halfBufferSize-1; j<halfBufferSize; j++, k--)
            setPixel(&image,arrayFFT[j],i,k);
    }
    qDebug() << "Readed data bytes:" << readedBytes;

    setupView(image);
    if (!open)
        file->close();

    delete [] arrayPreviousRead;
    delete [] arrayRead;
    delete [] arrayFFT;
}

void FormRespect::resizeEvent(QResizeEvent *)
{
    int w = this->width() - freqAxisSpace;
    int h = this->height() - timeAxisSpace;

    timeAxisY = h + AXIS_MARGIN;

    ui->view->resize(w,h);
}

void FormRespect::paintEvent(QPaintEvent *)
{
    // begining
    painter->begin(this);
    QPalette palette;
    QPen pen(palette.windowText(),AXIS_WIDTH);
    painter->setPen(pen);

    // drawing axes
    painter->drawLine(freqAxisX,ui->view->y(),freqAxisX,timeAxisY);
    painter->drawLine(freqAxisX,timeAxisY,freqAxisSpace+ui->view->width(),timeAxisY);


    // drawing markers by frequency axis
    freqAxisStartY = ui->view->y() + ui->view->height() - ui->view->horizontalScrollBar()->height();
    double freqMarksCount = (double) (freqAxisStartY - freqAxisStopY) / freqMarkMinSpacer;
    uchar iFreqMarksCount = (uchar)freqMarksCount;
    uchar freqModulo = ( freqMarksCount - iFreqMarksCount ) * freqMarkMinSpacer;
    double freqMarkSpacer = freqMarkMinSpacer + (double) freqModulo / iFreqMarksCount;

    quint16 freqMinValue = sndFileFrequency * (double) ( ui->view->scene()->height() - 1 -
                                      ui->view->mapToScene(ui->view->viewport()->rect().bottomLeft()).y())
                            / ui->view->scene()->height();

    quint16 freqMaxValue = sndFileFrequency * (double) ( ui->view->scene()->height() -
                                                         ui->view->mapToScene(0,0).y() )
                            / ui->view->scene()->height();

    double freqValueStep = (double) (freqMaxValue - freqMinValue) / iFreqMarksCount;
    double freqValue = freqMaxValue;
    quint16 freqMarkEndX = freqAxisX - AXIS_MARK_LENGTH;

    painter->drawLine(freqAxisX,freqAxisStopY,freqMarkEndX,freqAxisStopY);
    painter->drawText(freqLabelX,10,QString::number(freqMaxValue*0.001,'f',2));
    for (uchar i=1; i<=iFreqMarksCount; i++)
    {
        freqValue -= freqValueStep;
        quint16 y = freqAxisStopY + i*freqMarkSpacer;
        painter->drawLine(freqAxisX,y,freqMarkEndX,y);
        painter->drawText(freqLabelX,y+4,QString::number(freqValue*0.001,'f',2));
    }


    // drawing markers by time axis
    timeAxisStopX = timeAxisStartX + ui->view->width() - AXIS_WIDTH/2 - SCROLLBAR_SIZE;
    double timeMarksCount = (double) (timeAxisStopX - timeAxisStartX) / timeMarkMinSpacer;
    uchar iTimeMarksCount = (uchar) timeMarksCount;
    uchar timeModulo = (timeMarksCount - iTimeMarksCount) * timeMarkMinSpacer;
    double timeMarkSpacer = timeMarkMinSpacer + (double) timeModulo / iTimeMarksCount;

    double timeMinValue = sndFileTime * ui->view->mapToScene(ui->view->viewport()->rect().bottomLeft()).x()
                            / ui->view->scene()->width();

    double timeMaxValue = sndFileTime * (ui->view->mapToScene(ui->view->viewport()->rect().bottomRight()).x()+1)
                            / ui->view->scene()->width();

    double timeValueStep = (timeMaxValue - timeMinValue) / iTimeMarksCount;
    double timeValue = timeMinValue;
    quint16 timeMarkEndY = timeAxisY + AXIS_MARK_LENGTH;

    for (uchar i=0; i<iTimeMarksCount; i++)
    {
        quint16 x = timeAxisStartX + i*timeMarkSpacer;
        painter->drawLine(x,timeAxisY,x,timeMarkEndY);
        painter->drawText(x-20,timeMarkEndY+14,QString::number(timeValue,'f',3));
        timeValue += timeValueStep;
    }
    painter->drawLine(timeAxisStopX,timeAxisY,timeAxisStopX,timeMarkEndY);
    painter->drawText(timeAxisStopX-40,timeMarkEndY+14,QString::number(timeMaxValue,'f',3));

    // ending
    painter->end();
}

void FormRespect::setFileTime(double v)
{
    sndFileTime = v;
}

void FormRespect::setFileFreq(int v)
{
    sndFileFrequency = v;
}

void FormRespect::save(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly))
    {
        QMessageBox::critical(this, tr("Save failed"), tr("File open to write failed."));
        return;
    }
//    QDataStream out(&file);
//    out.setVersion(QDataStream::Qt_4_6);
//    out << view()->markerMap().size();
//    foreach (MarkerPoint marker, view()->markerMap())
//    {
//        out << marker.time() << marker.string();
//    }
}

void FormRespect::saveAs()
{
    save( QFileDialog::getSaveFileName(this, tr("Save Markers"), QDir::homePath(),
                                         tr("Marker files")+" (*"+markerFileExt+")" ) );
}

void FormRespect::load(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(this, tr("Load failed"), tr("File open to read failed."));
        return;
    }
    QDataStream in(&file);
    in.setVersion(QDataStream::Qt_4_6);
    int size;
    in >> size;
    double time;
    int freq;
//    QString string;
//    for (int i=0; i<size; i++)
//    {
//        in >> time >> freq >> string;
//        view()->addItem( MarkerPoint(time,freq,string,view()) );
//    }
}

void FormRespect::loadFrom()
{
    load( QFileDialog::getOpenFileName(this, tr("Load Markers"), QDir::homePath(),
                                           tr("Marker files")+" (*"+markerFileExt+")") );
}

void FormRespect::setPixel(QImage *image, double i, int x, int y)
{
    QColor color;
    if (i>1.0)
        color = colorOverflow;
    else
    {
        int r1 = colorMin.red();
        int g1 = colorMin.green();
        int b1 = colorMin.blue();
        int r2 = colorMax.red();
        int g2 = colorMax.green();
        int b2 = colorMax.blue();
        color.setRed((r2-r1)*i);
        color.setGreen((g2-g1)*i);
        color.setBlue((b2-b1)*i);
    }
    image->setPixel(x,y,color.rgb());
}

void FormRespect::setColorMin(const QString &name)
{
    colorMin = QColor(name);
}

void FormRespect::setColorMax(const QString &name)
{
    colorMax = QColor(name);
}

void FormRespect::setColorOverflow(const QString &name)
{
    colorOverflow = QColor(name);
}
