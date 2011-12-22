#ifndef FORMRESPECT_H
#define FORMRESPECT_H

#include <QWidget>

#include "ui_formrespect.h"

class QGraphicsScene;
class GraphicsView;
class WaveFile;

class FormRespect : public QWidget
{
    Q_OBJECT

public:
    explicit FormRespect(QWidget *parent = 0);
    ~FormRespect();
    GraphicsView *view() const { return ui->view; }
    void setupView(const QImage &image = QImage());
    void setupView(const QString &filePath);
    void setupView(WaveFile *file);
    void setFileTime(double);
    double fileTime() const { return sndFileTime; }
    void setFileFreq(int);
    int fileFreq() const { return sndFileFrequency; }
    static void setColorMin(const QString &name);
    static void setColorMax(const QString &name);
    static void setColorOverflow(const QString &name);

protected:
    virtual void resizeEvent(QResizeEvent *);
    virtual void paintEvent(QPaintEvent *);

private:
    Ui::FormRespect *ui;
    quint16 timeAxisY;
    quint16 timeAxisStartX;
    quint16 timeAxisStopX;
    quint16 timeAxisSpace;
    quint16 timeMarkMinSpacer;
    qint16 freqLabelX;
    quint16 freqAxisX;
    quint16 freqAxisStartY;
    quint16 freqAxisStopY;
    quint16 freqAxisSpace;
    uchar freqMarkMinSpacer;
    double sndFileTime;
    quint16 sndFileFrequency;
    QGraphicsScene* scene;
    QPainter* painter;
    static QColor colorMin;
    static QColor colorMax;
    static QColor colorOverflow;

    void setPixel(QImage *image, double i, int x, int y);


public slots:
    void save(const QString &);
    void saveAs();
    void load(const QString &);
    void loadFrom();

signals:
    void setStatusMessage(const QString &);
};

#endif // FORMRESPECT_H
