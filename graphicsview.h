#ifndef GRAPHICSVIEW_H
#define GRAPHICSVIEW_H

#include <QGraphicsView>
#include <QMap>
#include "markerpoint.h"

class QPixmap;

class GraphicsView : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsView(QWidget* parent = 0);
    void setPixmap(QPixmap pixmap);
    QGraphicsPixmapItem* pixmapItem() const { return graphicsPixmapItem; }
    const QMap<QGraphicsItem*,MarkerPoint> &markerMap() const { return markerPointMap; }
    void setMaxTime(double);
    void setMaxFreq(int);
    double maxTime() const { return fileTime; }
    int maxFreq() const { return fileFreq; }
    MarkerPoint removeItem(QGraphicsItem *itemPtr);
    QGraphicsItem *addItem(const MarkerPoint &marker, uchar radius = markerPointRadius);

protected:
    virtual void wheelEvent(QWheelEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseDoubleClickEvent(QMouseEvent *event);
    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);
    virtual void resizeEvent(QResizeEvent *event);

signals:
    void pixmapChanged();
    void selectedItemChanged(const QString &);

public slots:
    void selectedTextChanged();

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
    QMap<QGraphicsItem*,MarkerPoint> markerPointMap;
    QGraphicsItem* selectedMarker;
    static const uchar markerPointRadius = 5;
    static const uchar markerPointSelectedRadius = 2.4 * markerPointRadius;

    void scalePixmap();

};

#endif // GRAPHICSVIEW_H
