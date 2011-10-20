#ifndef MARKERPOINT_H
#define MARKERPOINT_H

#include <QPoint>
#include <QString>

class GraphicsView;
class QGraphicsItem;

class MarkerPoint : public QPoint
{
public:
    MarkerPoint(GraphicsView *parent = 0);
    MarkerPoint(int x, int y, GraphicsView *parent = 0);
    MarkerPoint(double time, int freq, GraphicsView *parent = 0);
    MarkerPoint(double time, int freq, const QString &str, GraphicsView *parent = 0);
    void setString(const QString&);
    const QString &string() const { return str; }
    double time() const { return time_; }
    int freq() const { return freq_; }
    void refresh();
    static void setMaxX(int);
    static void setMaxY(int);
private:
    GraphicsView *view;
//    QGraphicsEllipseItem *itemPtr;
    QString str;
    double time_;
    int freq_;
    static int maxX;
    static int maxY;
};

#endif // MARKERPOINT_H
