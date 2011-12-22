#ifndef WAVEFILE_H
#define WAVEFILE_H

#include <QFile>

class WaveFile : public QFile
{
    Q_OBJECT

public:
    WaveFile(QObject *parent = 0);
    WaveFile(const QString& name);
    WaveFile(const QString &name, QObject *parent);
    qint64 readHeader();
    qint64 readData(double *buffer, int bufferSize, int channelId = 0);
    int numChannels() const { return header.wave.numChannels; }
    int numSamples() const { return  header.data.descriptor.size /
                (header.wave.bitsPerSample/8) / header.wave.numChannels; }
    int maxFrequency() const { return header.wave.sampleRate * 0.5; }
    double realTime() const { return (double) header.data.descriptor.size / header.wave.byteRate; }
    // virtual methods interided from QFile
    virtual bool atEnd() const { return QFile::atEnd(); }
    virtual void close() { QFile::close(); }
    virtual bool isSequential() const { return QFile::isSequential(); }
    virtual bool open(OpenMode flags);
    virtual qint64 pos() const { return QFile::pos(); }
    virtual bool seek(qint64 offset);
    virtual qint64 size() const { return QFile::size(); }

private:
    struct chunk
    {
        char        id[4];
        quint32     size;
    };
    struct RIFFHeader
    {
        chunk       descriptor;     // "RIFF"
        char        type[4];        // "WAVE"
    };
    struct WAVEHeader
    {
        chunk       descriptor;
        quint16     audioFormat;
        quint16     numChannels;
        quint32     sampleRate;
        quint32     byteRate;
        quint16     blockAlign;
        quint16     bitsPerSample;
    };
    struct DATAHeader
    {
        chunk       descriptor;
    };
    struct CombinedHeader
    {
        RIFFHeader  riff;
        WAVEHeader  wave;
        DATAHeader  data;
    };
    CombinedHeader header;
    static const int HeaderLength = sizeof(CombinedHeader);
    bool isFirstSample;
};

#endif // WAVEFILE_H
