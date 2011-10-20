#ifndef QLIBRESPECT_H
#define QLIBRESPECT_H

#include "userinterface.h"

class QGraphicsScene;
class QGraphicsView;
class QScrollBar;

class qLibReSpect : public UserInterface
{
    Q_OBJECT

public:
    qLibReSpect(QWidget *parent = 0);
    ~qLibReSpect();

protected:
    virtual void test();
//    virtual void resizeEvent(QResizeEvent *);

private:
    QImage* image;
};

#endif // QLIBRESPECT_H
