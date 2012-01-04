#ifndef FORMRESPECT_H
#define FORMRESPECT_H

#include <QWidget>

#include "ui_formrespect.h"

class QGraphicsScene;
class GraphicsView;
class WaveFile;

// Plot with axes widget class
class FormRespect : public QWidget
{
    Q_OBJECT

public:
    explicit FormRespect(QWidget *parent = 0);
    ~FormRespect();
    // Returns pointer to included plot wigdet
    GraphicsView *view() const { return ui->view; }
    // Set up plot widget
    void setupView(const QImage &image = QImage());
    void setupView(const QString &filePath);
    void setupView(WaveFile *file);
    // Set time length of wave file in seconds
    void setFileTime(double);
    // Returns time value in seconds of wave file
    double fileTime() const { return sndFileTime; }
    // Set maximum frequency value in Hz witch it can be in wave file
    void setFileFreq(int);
    // Returns maximum frequency value in Hz witch it can be in wave file
    int fileFreq() const { return sndFileFrequency; }
    // Set color values in following format: "#RRGGBB"
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

    // Set pixel color
    // Set pixel color in image coordinates x, y to i value
    // i value should vary in the range 0.0 to 1.0, if i > 1.0 sets colorOverflow
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
