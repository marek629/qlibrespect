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
    //reading main part of cue chunk
    qint64 result = read(reinterpret_cast<char *>(&cue), chunkLength);
    if ( result == chunkLength ) {
        while (memcmp(cue.descriptor.id,"cue ",4) != 0) {
            result += read(cue.descriptor.size).size();
            result += read(reinterpret_cast<char *>(&cue), chunkLength);
        }
        result += read(reinterpret_cast<char *>(&cue.numCuePoints), sizeof(cue.numCuePoints));
        cue.list.resize(cue.numCuePoints);
        for (quint32 i=0; i<cue.numCuePoints; i++) {
            result += read(reinterpret_cast<char *>(&cue.list[i]), cuePointLength);
        }
        qDebug() << "Cue Point count:" << cue.numCuePoints;
        if (atEnd())
            return result;

        // reading list chunk and details
        DataListChunk list;     //main LIST chunk
        while (true)
        {
            result += read(reinterpret_cast<char *>(&list), chunkLength); // searching for "LIST" chunk
            if (memcmp(list.descriptor.id,"LIST",4) == 0)
            {
                //LIST was found reading rest of it
                result += read(reinterpret_cast<char *>(&list.typeID) ,4);
                if (memcmp(list.typeID,"adtl",4) == 0)
                    break;
                else
                    return result; // chunk List was found, but was containing incorrect data
            }
            else if(atEnd()) // chunk List not found, this mean it didn't found descryption of marks
                return result;
        }
        //chunk List was readed wholly proper, looking for rest of cue chank's
        Chunk temp; //temporary chunk
        //data list's
        listLabels.clear();
        listNotes.clear();
        listLtxt.clear();
        while(!atEnd())
        {
            //looking for chunk begining
            //reading 1 byte loop until first byte of id is not "/0"
            do
                result += read(reinterpret_cast<char *>(&temp),1);
            while(temp.id[0] == 0);
            // reading rest of chunk
            seek(pos()-1); // withdrawing read positnion by 1 byte
            result += read(reinterpret_cast<char *>(&temp),sizeof(temp));
            if (memcmp(temp.id,"ltxt",4) == 0)
            {
                LabeledTextChunk ltxt;
                result += read(reinterpret_cast<char *>(&ltxt),sizeof(ltxt)-sizeof(ltxt.text));
                ltxt.text.clear();
                quint32 sizeOfText = temp.size - (quint32)sizeof(ltxt) + (quint32)sizeof(ltxt.text); // size of text stored in chunk
                if (sizeOfText != 0)
                {
                    ltxt.text.reserve(sizeOfText); // reserving nessesery size for text
                    char *tempChar = new char[sizeOfText]; //templorary char table for text
                    result += read(tempChar,sizeOfText); // reading text
                    ltxt.text = tempChar;
                    delete tempChar;
                    qDebug() << "ltxt text: " << ltxt.text;
                }
                listLtxt.append(ltxt); // adding readed chunk to list
            }
            else if (memcmp(temp.id,"labl",4) == 0)
            {
                LabelChunk label;
                result += read(reinterpret_cast<char *>(&label.cuePointID),sizeof(label.cuePointID));
                label.text.clear();
                quint32 sizeOfText = temp.size - (quint32)sizeof(label) + (quint32)sizeof(label.text); // size of text stored in chunk
                if (sizeOfText != 0)
                {
                    label.text.reserve(sizeOfText);
                    char *tempChar = new char[sizeOfText];
                    result += read(tempChar,sizeOfText);
                    label.text = tempChar;
                    delete tempChar;
                    qDebug() << "label text: " << label.text;
                }
                listLabels.append(label);
            }
            else if (memcmp(temp.id,"note",4) == 0)
            {
                NoteChunk note;
                result += read(reinterpret_cast<char *>(&note.cuePointID),sizeof(note.cuePointID));
                note.text.clear();
                quint32 sizeOfText = temp.size - (quint32)sizeof(note) + (quint32)sizeof(note.text); // size of text stored in chunk
                if (sizeOfText != 0)
                {
                    note.text.reserve(sizeOfText);
                    char *tempChar = new char[sizeOfText];
                    result += read(tempChar,sizeOfText);
                    note.text = tempChar;
                    delete tempChar;
                    qDebug() << "note text: " << note.text;
                }
                listNotes.append(note);
            }
        }
        return result; //and of file, returning
    }
    else //at EOF
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
