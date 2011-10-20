#include "formrespect.h"
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
    timeAxisStartX = ui->view->x();
    timeMarkMinSpacer = 250;

    freqAxisSpace = ui->view->x();
    freqAxisX = ui->view->x() - AXIS_MARGIN;
    freqAxisStopY = ui->view->y() + AXIS_WIDTH/2;
    freqLabelX = freqAxisX - 38 - AXIS_MARK_LENGTH;
    freqMarkMinSpacer = 50;

    painter = new QPainter();
    scene = 0;

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
    timeAxisStopX = timeAxisStartX + ui->view->width() - AXIS_WIDTH/2;
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
    QDataStream out(&file);
    out.setVersion(QDataStream::Qt_4_6);
    out << view()->markerMap().size();
    foreach (MarkerPoint marker, view()->markerMap())
    {
        out << marker.time() << marker.freq() << marker.string();
    }
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
    QString string;
    for (int i=0; i<size; i++)
    {
        in >> time >> freq >> string;
        view()->addItem( MarkerPoint(time,freq,string,view()) );
    }
}

void FormRespect::loadFrom()
{
    load( QFileDialog::getOpenFileName(this, tr("Load Markers"), QDir::homePath(),
                                           tr("Marker files")+" (*"+markerFileExt+")") );
}
