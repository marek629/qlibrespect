#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMap>
#include "markerpoint.h"

class QPixmap;

// Plot (spectrogram) widget class
class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(QWidget* parent = 0);
    // Set plot pixmap
    void setPixmap(QPixmap pixmap);
    // Returns pointer to plot pixmap item
    QGraphicsPixmapItem* pixmapItem() const { return graphicsPixmapItem; }
    // Set time length of wave file in seconds
    void setMaxTime(double);
    // Set maximum frequency value in Hz witch it can be in wave file
    void setMaxFreq(int);
    // Returns time value in seconds of wave file
    double maxTime() const { return fileTime; }
    // Returns maximum frequency value in Hz witch it can be in wave file
    int maxFreq() const { return fileFreq; }
    // Removes marker item from this object and returns removed marker point object
    void addMarker(MarkerPoint &marker);
    void addMarkers(QVector<MarkerPoint> &markers);
    void delMarker(MarkerPoint &marker);

protected:
    virtual void wheelEvent(QWheelEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

signals:
    void pixmapChanged();
    void selectedItemChanged(const QString &);

public slots:

private:
    double freqZoomFactor;
    double timeZoomFactor;
    double minFreqZoomFactor;
    double minTimeZoomFactor;
    double fileTime;
    int fileFreq;
    bool mouseRightPressed;
    bool mouseLeftPressed;
    QPoint mousePosition;
    bool keyCtrlPressed;
    QPixmap pixmap;
    QGraphicsPixmapItem* graphicsPixmapItem;

    void scalePixmap();

};

#endif // GRAPHICSVIEW_H
