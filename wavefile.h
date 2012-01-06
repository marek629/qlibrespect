#ifndef WAVEFILE_H
#define WAVEFILE_H

#include <QFile>
#include <QVector>

// Wave file access class (read only yet)
class WaveFile : public QFile
{
    Q_OBJECT

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
 //cue chunk section
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
    struct DataListChunk // "list"
    {
        Chunk       descriptor;
        char        typeID[4];
    };
public:
    struct LabeledTextChunk // "ltxt"
    {
       // Chunk       descriptor;
        quint32     cuePointID;
        quint32     sampleLength;
        quint32     purposeID;
        quint16     country;
        quint16     lang;
        quint16     dialect;
        quint16     codePage;
        QString     text;
    };
    struct LabelChunk // "label"
    {
       // Chunk       descriptor;
        quint32     cuePointID;
        QString     text;
    };
    struct NoteChunk : LabelChunk {}; // "note"
private:
    //mark's description lists
    QVector<LabelChunk> listLabels_;
    QVector<NoteChunk>  listNotes_;
    QVector<LabeledTextChunk> listLtxt_;

public:
    // Constructors
    WaveFile(QObject *parent = 0);
    WaveFile(const QString& name);
    WaveFile(const QString &name, QObject *parent);
    // Read header up to data chunk including header fo data chunk
    // After use this function file is seeked to first byte of sound data
    qint64 readHeader();
    // Read sound data maximum bufferSize bytes from channel channelId into buffer
    qint64 readData(double *buffer, int bufferSize, int channelId = 0);
    // Read cue chunk. It should call after reading last byte in data chunk
    qint64 readCue();
    // Return number of bytes per sample in the file
    int bytesPerSample() const { return header.wave.bitsPerSample / 8; }
    // Return end of data chunk position in the file
    int posDataEnd() const { return dataOffset + header.data.descriptor.size; }
    // Return number of channels in the file
    int numChannels() const { return header.wave.numChannels; }
    // Return number of samples in the file
    int numSamples() const { return  header.data.descriptor.size /
                (header.wave.bitsPerSample/8) / header.wave.numChannels; }
    // Return maximum frequency value in Hz witch it can be in the file
    int maxFrequency() const { return header.wave.sampleRate * 0.5; }
    // Return time value in seconds of the file
    double realTime() const { return (double) header.data.descriptor.size / header.wave.byteRate; }

    // Return pointers to list of markers data
    QVector<WaveFile::LabelChunk> *listLabels() { return &listLabels_; }
    QVector<WaveFile::NoteChunk>  *listNotes() { return &listNotes_; }
    QVector<WaveFile::LabeledTextChunk> *listLtxt() { return &listLtxt_; }

    // virtual methods interided from QFile
    virtual bool atEnd() const { return QFile::atEnd(); }
    virtual void close() { QFile::close(); }
    virtual bool isSequential() const { return QFile::isSequential(); }
    virtual bool open(OpenMode flags);
    virtual qint64 pos() const { return QFile::pos(); }
    virtual bool seek(qint64 offset);
    virtual qint64 size() const { return QFile::size(); }
};

#endif // WAVEFILE_H
