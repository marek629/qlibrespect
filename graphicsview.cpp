#include "graphicsview.h"
#include <QWheelEvent>
#include <QScrollBar>
#include <QGraphicsItem>
#include <QPlainTextEdit>

#include <QDebug>

const QString helpString = QObject::tr("Usage:\n\n"
                                       "--- marker operations ---\n"
                                       " double left click to add new marker\n"
                                       " right click to remove focused marker\n"
                                       " left click to (un)select focused marker\n\n"
                                       "--- view manipulations ---\n"
                                       " left click and move to move view\n"
                                       " mouse scroll to zooming time\n"
                                       " mouse scroll when right mouse button pressed to zooming frequency\n"
                                       " mouse scroll when Ctrl key pressed to zooming time and frequncy");

GraphicsView::GraphicsView(QWidget* parent) : QGraphicsView(parent)
{
    freqZoomFactor = 1.0;
    timeZoomFactor = 1.0;
    mouseRightPressed = false;
    mouseLeftPressed = false;
    keyCtrlPressed = false;
    graphicsPixmapItem = 0;
    selectedMarker = 0;
}

void GraphicsView::setPixmap(QPixmap pxm)
{
    pixmap = pxm;

    freqZoomFactor = (double) viewport()->height() / pixmap.height();
    timeZoomFactor = (double) viewport()->width() / pixmap.width();

    minFreqZoomFactor = freqZoomFactor;
    minTimeZoomFactor = timeZoomFactor;

    scalePixmap();
    emit selectedItemChanged(helpString);
}

void GraphicsView::scalePixmap()
{
    this->scene()->clear();
    int h = pixmap.height() * freqZoomFactor;
    int w = pixmap.width() * timeZoomFactor;
    graphicsPixmapItem = this->scene()->addPixmap(
                pixmap.scaled(w,h,Qt::IgnoreAspectRatio,Qt::SmoothTransformation) );
    this->scene()->setSceneRect(0,0,w,h);

    QMap<QGraphicsItem*,MarkerPoint> tempMap (markerPointMap);
    markerPointMap.clear();
    MarkerPoint::setMaxX( sceneRect().width() );
    MarkerPoint::setMaxY( sceneRect().height() );
    MarkerPoint marker;
    foreach (marker,tempMap)
    {
        bool selected = ( tempMap.key(marker) == selectedMarker );
        marker.refresh();
        if (selected)
            selectedMarker = addItem(marker, markerPointSelectedRadius);
        else
            addItem(marker, markerPointRadius);
    }

    this->update();
    emit pixmapChanged();
}

void GraphicsView::wheelEvent(QWheelEvent *event)
{
    double i = event->delta()/1200.;

    if (keyCtrlPressed)
    {
        timeZoomFactor += i;
        freqZoomFactor += i;
    }
    else if (mouseRightPressed)
        freqZoomFactor += i;
    else
        timeZoomFactor += i;

    if (minFreqZoomFactor==freqZoomFactor && minTimeZoomFactor==timeZoomFactor)
        return;

    if (minFreqZoomFactor > freqZoomFactor)
        freqZoomFactor = minFreqZoomFactor;
    else if (minTimeZoomFactor > timeZoomFactor)
        timeZoomFactor = minTimeZoomFactor;

    scalePixmap();

    QPoint cursorPosition = mapFromGlobal(QCursor::pos());
    QPointF positionOnScene = mapToScene(cursorPosition);
    centerOn( positionOnScene.x() + i * cursorPosition.x(),
              positionOnScene.y() + i * cursorPosition.y() );
}

void GraphicsView::mouseMoveEvent(QMouseEvent *event)
{
    if (mouseLeftPressed)
    {
        verticalScrollBar()->setValue( verticalScrollBar()->value() +
                                       (mousePosition.y() - event->globalY()) );
        horizontalScrollBar()->setValue( horizontalScrollBar()->value() -
                                         (mousePosition.x() - event->globalX()) );
        mousePosition = event->globalPos();
    }
}

void GraphicsView::mousePressEvent(QMouseEvent *event)
{
    QGraphicsItem* itemPtr = scene()->itemAt( mapToScene( event->pos() ) );
    if (event->button() == Qt::RightButton)
    {   // remove marker
        mouseRightPressed = true;

        if (itemPtr != (QGraphicsItem*)graphicsPixmapItem)
            removeItem(itemPtr);
    }
    else if (event->button() == Qt::LeftButton)
    {
        mouseLeftPressed = true;
        mousePosition = event->globalPos();

        if (itemPtr != (QGraphicsItem*)graphicsPixmapItem)
        {
            if (itemPtr == selectedMarker)
            {   // unselect marker
                MarkerPoint marker = removeItem(itemPtr);
                addItem(marker, markerPointRadius);
                selectedMarker = 0;
                emit selectedItemChanged( helpString );
            }
            else
            {   // select marker
                if (selectedMarker != 0)
                {   // unselect other marker
                    MarkerPoint mrk = removeItem(selectedMarker);
                    addItem(mrk, markerPointRadius);
                }
                QMap<QGraphicsItem*,MarkerPoint>::iterator i = markerPointMap.find(itemPtr);
                MarkerPoint marker = i.value();
                scene()->removeItem(itemPtr);
                markerPointMap.erase(i);
                itemPtr = addItem(marker, markerPointSelectedRadius);
                selectedMarker = itemPtr;
                emit selectedItemChanged( marker.string() );
            }
        }
    }
}

void GraphicsView::mouseReleaseEvent(QMouseEvent *)
{
    mouseRightPressed = false;
    mouseLeftPressed = false;
}

void GraphicsView::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPointF scenePoint = mapToScene( event->pos() );
    QGraphicsItem* itemPtr = scene()->itemAt(scenePoint);
    if ( itemPtr == (QGraphicsItem*)graphicsPixmapItem )
    {
        double t = fileTime * scenePoint.x() / sceneRect().width();
        int f = fileFreq * ( sceneRect().height() - scenePoint.y() ) / sceneRect().height();
        MarkerPoint marker(t,f,this);
        addItem(marker, markerPointRadius);
    }
}

void GraphicsView::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Control)
        keyCtrlPressed = true;
}

void GraphicsView::keyReleaseEvent(QKeyEvent *)
{
    keyCtrlPressed = false;
}

void GraphicsView::resizeEvent(QResizeEvent *event)
{
    bool freqMinimalZoom = (minFreqZoomFactor==freqZoomFactor);
    bool timeMinimalZoom = (minTimeZoomFactor==timeZoomFactor);

    minFreqZoomFactor = (double) event->size().height() / pixmap.height();
    minTimeZoomFactor = (double) event->size().width() / pixmap.width();

    bool scaleFreq = (minFreqZoomFactor > freqZoomFactor  ||  freqMinimalZoom);
    bool scaleTime = (minTimeZoomFactor > timeZoomFactor  ||  timeMinimalZoom);

    if (scaleFreq || scaleTime)
    {
        if (scaleFreq)
            freqZoomFactor = minFreqZoomFactor;
        if (scaleTime)
            timeZoomFactor = minTimeZoomFactor;

        scalePixmap();

        if (scaleFreq ^ scaleTime)
            QGraphicsView::resizeEvent(event);
    }
    else
        QGraphicsView::resizeEvent(event);
}

void GraphicsView::setMaxTime(double v)
{
    fileTime = v;
}

void GraphicsView::setMaxFreq(int v)
{
    fileFreq = v;
}

MarkerPoint GraphicsView::removeItem(QGraphicsItem *itemPtr)
{
    MarkerPoint marker = markerPointMap.value(itemPtr);
    scene()->removeItem(itemPtr);
    markerPointMap.erase( markerPointMap.find(itemPtr) );
    return marker;
}

QGraphicsItem * GraphicsView::addItem(const MarkerPoint &marker, uchar radius)
{
    uchar diameter = 2 * radius;
    QGraphicsItem *itemPtr = (QGraphicsItem*)
            scene()->addEllipse( marker.x()-radius, marker.y()-radius,
                                 diameter, diameter, QPen(), QBrush(Qt::red) );
    markerPointMap.insert(itemPtr,marker);
    QString tip = tr("Time: ") + QString::number(marker.time(),'g',3) + " s\n" +
                  tr("Frequency: ") + QString::number(marker.freq()) + " Hz";
    itemPtr->setToolTip(tip);
    return itemPtr;
}

void GraphicsView::selectedTextChanged()
{
    static QString lastText;

    QPlainTextEdit *textEdit = (QPlainTextEdit*) sender();
    QString txt = textEdit->toPlainText();
    if (txt != lastText)
    {
        if (txt != helpString)
        {
            MarkerPoint marker = removeItem(selectedMarker);
            marker.setString(txt);
            selectedMarker = addItem(marker, markerPointSelectedRadius);
            textEdit->setReadOnly(false);
        }
        else
            textEdit->setReadOnly(true);
        lastText = txt;
    }
}
