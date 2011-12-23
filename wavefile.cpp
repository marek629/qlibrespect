#include "wavefile.h"
#include <QByteArray>
#include <QDebug>

WaveFile::WaveFile(QObject *parent) : QFile(parent)
{
}

WaveFile::WaveFile(const QString &name) : QFile(name)
{
}

WaveFile::WaveFile(const QString &name, QObject *parent) : QFile(name, parent)
{
}

qint64 WaveFile::readHeader() {
    qint64 result = 0;

    if (!isSequential()) // true only if device is random access
        result = seek(0); //true when index position of reading file is 0
    if (result != 0)
    {
        result = read(reinterpret_cast<char *>(&header), headerLength); //reading metadata into header
        if (result == headerLength) //true when bytes readed are HeaderLength long;
        {
            if ((memcmp(&header.riff.descriptor.id, "RIFF", 4) == 0
                || memcmp(&header.riff.descriptor.id, "RIFX", 4) == 0)
                && memcmp(&header.riff.type, "WAVE", 4) == 0
                && memcmp(&header.wave.descriptor.id, "fmt ", 4) == 0
                && header.wave.audioFormat == 1 ) // true when is PCM
            {
                while (memcmp(&header.data.descriptor.id, "data", 4) != 0)
                {
                    result += read(header.data.descriptor.size).size(); //changing offset
                    result += read( reinterpret_cast<char *>(&header.data), sizeof(Chunk) ); //overwrite header.data
                }
                qDebug() << "readHeader:file name" << fileName()
                         << ": size =" << size()
                         << ": data.descriptor :" << header.data.descriptor.id << header.data.descriptor.size;
                dataOffset = result;
            }
            else
            {
                qDebug() << "error- readHeader:: unsupported format or file demaged";
                result = -2; // unsuportet format so return false
            }
        }
    }
    else //current position is not the start of the header
        qDebug() << "error- readHeader:: position to read is not the start of the hearder";


    if (header.wave.bitsPerSample <= 0) //correction of division by zero in many places
        result = -1;
    return result;
}

qint64 WaveFile::readData(double *buffer, int bufferSize, int channelId) {
    qint64 result = 0;

    if (channelId > header.wave.numChannels)
    {
        qDebug() <<"readData: warning! There is no such channel like:" << channelId
                 << " - changed to channel 0" ;
        channelId = 0;
    }

    bool ok; //helping bool for converting string to int
    QString Sample, tmp;

    if (header.wave.bitsPerSample > 0)
    {
        for (int i=0; i<bufferSize && pos()<posDataEnd(); i++)
        {
            if (isFirstSample)
            {
                isFirstSample = false;
                result += read( (header.wave.bitsPerSample/8) * (channelId) ).size();
            }
            Sample = read( (header.wave.bitsPerSample/8) * header.wave.numChannels ).toHex();
            result += Sample.size() / 2;
            tmp = Sample.right(2);
            Sample.remove(Sample.length()-2,2);//remove last 2 chars
            tmp.append(Sample.right(2));
            double oneSample = tmp.toInt(&ok,16);
            if (oneSample > 32767)
                oneSample -= 65535;
            if (oneSample == 0)
                buffer[i] = 0;
            else
                buffer[i] = oneSample/32767;
    /*/____________________________FOR TESTS ONLY______________________
            qDebug() << i << "  one sample: " << tmp << "=" << oneSample << QString::number(tab[i]);

    /*/
        }
    /*/____________________________FOR TESTS ONLY______________________
        //qDebug() return strange results when its getting more than 2 fields from header
        qDebug() << "readData:bitsPerSample:"<< header.wave.bitsPerSample<<
                    ", numbers of channels:" << header.wave.numChannels<<
                    ", sample rate:" << header.wave.sampleRate<<
                    ", byte rate:" << header.wave.byteRate<<
                    ", header length:" << HeaderLength;
    /*/
    }
    else {
        qDebug() << "error - readData:bitsPerSample: " << header.wave.bitsPerSample ;
        result = -1;
    }
    return result;
}

qint64 WaveFile::readCue() {
    cue.numCuePoints = 0;
    cue.descriptor.size = 0;
    if (atEnd())
        return -1;
    qint64 result = read(reinterpret_cast<char *>(&cue), chunkLength);
    if ( result == chunkLength ) {
        while (memcmp(cue.descriptor.id,"cue ",4) != 0) {
            result += read(cue.descriptor.size).size();
            result += read(reinterpret_cast<char *>(&cue), chunkLength);
        }
        result += read(reinterpret_cast<char *>(&cue.numCuePoints), sizeof(int));
        cue.list.resize(cue.numCuePoints);
        for (int i=0; i<cue.numCuePoints; i++) {
            result += read(reinterpret_cast<char *>(&cue.list[i]), cuePointLength);
        }
        qDebug() << "Cue Point count:" << cue.numCuePoints;
        // reading external (not supported yet) chunks
        while (!atEnd()) {
            Chunk descriptor;
            result += read(reinterpret_cast<char *>(&descriptor), chunkLength);
            QByteArray array(descriptor.id, 4);
            qDebug() << array;
            result += read(descriptor.size).size();
        }
    }
    else
        return -2;
    return result;
}

bool WaveFile::seek(qint64 offset) {
    return QFile::seek(offset);
}

bool WaveFile::open(OpenMode flags) {
    bool result = QFile::open(flags);
    if (result)
        readHeader();
    return result;
}
