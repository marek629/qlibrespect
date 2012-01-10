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
    MarkerPoint(double time, int freq, GraphicsView *parent = 0);
    MarkerPoint(double time, int freq, const QString &str, GraphicsView *parent = 0);
    MarkerPoint(double time, GraphicsView *parent = 0);
    void setString(const QString &v);
    const QString &string() const { return str; }
    void setTime(double time) {time_ =time;}
    double time() const { return time_; }
    int freq() const { return freq_; }
    void refresh();
    bool isSelected() {return selected;}
    void setSelected(bool select = true) {selected = select;}
    static void setMaxX(int);
    static void setMaxY(int);
private:
    GraphicsView *view;
    QString str; //text stored in mark
    double time_; //x cordinat in seconds
    int freq_; //y cordintas in Hz
    static int maxX; //max length x axes
    static int maxY; //max hight y axes
    bool selected;
};

#endif // MARKERPOINT_H
