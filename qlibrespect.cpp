#include "qlibrespect.h"
#include <librespect.h>

#include <QImage>

// Including for test
#include <sndfile.h>
#include <QDebug>

qLibReSpect::qLibReSpect(QWidget *parent)
    : UserInterface(parent)
{
    test();
}

qLibReSpect::~qLibReSpect()
{
    delete image;
}

void qLibReSpect::test()
{
    SF_INFO* SF_info = new SF_INFO;
    SF_info->format = 0;
    SNDFILE* snd_file = sf_open("/home/marek/krogul.wav",SFM_READ,SF_info);
    if (snd_file==NULL)
    {
        qDebug("open sound file failed");
        return;
    }
    quint16 bufferSize = 512;
    quint16 halfBufferSize = bufferSize * 0.5;
    double* arrayPreviousRead = new double [halfBufferSize];
    double* arrayRead = new double [halfBufferSize];
    double* arrayFFT = new double [bufferSize];
    LibReSpect spect;
    quint32 framesBuffer = SF_info->frames / halfBufferSize + 1;
    image = new QImage(framesBuffer,halfBufferSize,QImage::Format_ARGB32);

    // First samples counting
    sf_read_double(snd_file,arrayFFT,bufferSize);
    for (quint16 i=0, j=halfBufferSize; i<halfBufferSize; i++, j++)
        arrayPreviousRead[i] = arrayFFT[j];
    spect.makeWindow(arrayFFT);
    spect.countFFT(arrayFFT);
    for (quint16 j=0, k=halfBufferSize-1; j<halfBufferSize; j++, k--)
    {
        int m;
        if (arrayFFT[j]>1.0)
        {
            m = 255;
        }
        else
            m = arrayFFT[j] * 254;
        image->setPixel(0,k,QColor(m,m,m).rgb());
    }

    // Next samples counting
    for (quint16 i=1; i<framesBuffer; i++)
    {
        int count = sf_read_double(snd_file,arrayRead,halfBufferSize);
        for (quint16 j=0, k=halfBufferSize; j<halfBufferSize; j++,k++)
        {
            arrayFFT[j] = arrayPreviousRead[j];
            arrayFFT[k] = arrayRead[j];
            arrayPreviousRead[j] = arrayRead[j];
        }
        spect.makeWindow(arrayFFT);
        spect.countFFT(arrayFFT);
        for (quint16 j=0, k=halfBufferSize-1; j<halfBufferSize; j++, k--)
        {
            int m;
            if (arrayFFT[j]>1.0)
            {
                m = 255;
            }
            else
                m = arrayFFT[j] * 222 + 20;
            image->setPixel(i,k,QColor(m,m,m).rgb());
        }
    }

    this->resize(990,270);

    delete [] arrayPreviousRead;
    delete [] arrayRead;
    delete [] arrayFFT;
    delete SF_info;
}

//void qLibReSpect::resizeEvent(QResizeEvent *)
//{

//}
