#ifndef FORMRESPECT_H
#define FORMRESPECT_H

#include <QWidget>

#include "ui_formrespect.h"

class QGraphicsScene;
class GraphicsView;

class FormRespect : public QWidget
{
    Q_OBJECT

public:
    explicit FormRespect(QWidget *parent = 0);
    ~FormRespect();
    GraphicsView *view() const { return ui->view; }
    void setupView(const QImage &image = QImage());
    void setFileTime(double);
    double fileTime() const { return sndFileTime; }
    void setFileFreq(int);
    int fileFreq() const { return sndFileFrequency; }

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

public slots:
    void save(const QString &);
    void saveAs();
    void load(const QString &);
    void loadFrom();
};

#endif // FORMRESPECT_H
