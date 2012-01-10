#include "markerpoint.h"
#include "graphicsview.h"

int MarkerPoint::maxX = 0;
int MarkerPoint::maxY = 0;

MarkerPoint::MarkerPoint(GraphicsView *parent) : QPoint()
{
    view = parent;
}

MarkerPoint::MarkerPoint(double time, int freq, GraphicsView *parent) : QPoint()
{
    view = parent;
    time_ = time;
    freq_ = freq;
    refresh();
}

MarkerPoint::MarkerPoint(double time, int freq, const QString &str, GraphicsView *parent)
{
    view = parent;
    time_ = time;
    freq_ = freq;
    refresh();
    setString(str);
}

void MarkerPoint::refresh()
{
    setX( maxX * time_ / view->maxTime() );
    setY( maxY - maxY * freq_ / view->maxFreq() );
}

void MarkerPoint::setString(const QString &v)
{
    str = v;
}

void MarkerPoint::setMaxX(int v)
{
    maxX = v;
}

void MarkerPoint::setMaxY(int v)
{
    maxY = v;
}
