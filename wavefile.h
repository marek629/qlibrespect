#ifndef WAVEFILE_H
#define WAVEFILE_H

#include <QFile>
#include <QVector>

class WaveFile : public QFile
{
    Q_OBJECT

public:
    WaveFile(QObject *parent = 0);
    WaveFile(const QString& name);
    WaveFile(const QString &name, QObject *parent);
    qint64 readHeader();
    qint64 readData(double *buffer, int bufferSize, int channelId = 0);
    qint64 readCue();
    int bytesPerSample() const { return header.wave.bitsPerSample / 8; }
    int posDataEnd() const { return dataOffset + header.data.descriptor.size; }
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
    bool isFirstSample;
    struct Chunk
    {
        char        id[4];
        quint32     size;
    };
    struct RIFFHeader
    {
        Chunk       descriptor;     // "RIFF"
        char        type[4];        // "WAVE"
    };
    struct WAVEHeader
    {
        Chunk       descriptor;
        quint16     audioFormat;
        quint16     numChannels;
        quint32     sampleRate;
        quint32     byteRate;
        quint16     blockAlign;
        quint16     bitsPerSample;
    };
    struct DATAHeader
    {
        Chunk       descriptor;
    };
    struct CombinedHeader
    {
        RIFFHeader  riff;
        WAVEHeader  wave;
        DATAHeader  data;
    };
    CombinedHeader header;
    static const int headerLength = sizeof(CombinedHeader);
    static const int chunkLength = sizeof(Chunk);
    int dataOffset;
    struct CuePoint
    {
        quint32     id;
        quint32     position;
        char        dataID[4];
        quint32     chunkStart;
        quint32     blockStart;
        quint32     sampleOffset;
    };
    struct CueChunk
    {
        Chunk       descriptor;
        quint32     numCuePoints;
        QVector<CuePoint>   list;
    };
    CueChunk cue;
    static const int cuePointLength = sizeof(CuePoint);
    struct LabelChunk
    {
        Chunk       descriptor;
        quint32     cuePointID;
        QString     text;
    };
    struct LabeledTextChunk
    {
        LabelChunk  label;
        quint32     sampleLength;
        quint32     purposeID;
        quint16     country;
        quint16     lang;
        quint16     dialect;
        quint16     codePage;
    };
    struct ListChunk
    {
        Chunk       descriptor;
        char        typeID[4];
        QList<LabeledTextChunk> list;
    };
    ListChunk list;
    static const int labelHeaderLength = chunkLength + sizeof(quint32);
    static const int labeledTextBodyLength = sizeof(LabeledTextChunk) - sizeof(LabelChunk);
};

#endif // WAVEFILE_H
