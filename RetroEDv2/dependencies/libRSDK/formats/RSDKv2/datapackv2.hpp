#pragma once

namespace RSDKv2
{

class Datapack
{
public:
    class DirInfo
    {
    public:
        DirInfo() {}
        DirInfo(Reader &reader) { read(reader); }

        void read(Reader &reader);
        void write(Writer &writer);

        QString directory = "dir/";
        int startOffset   = 0;
    };

    class FileInfo
    {
    public:
        FileInfo() {}
        FileInfo(Reader &reader) { read(reader); }

        void read(Reader &reader);
        void write(Writer &writer);

        QString fileName     = "File.ext";
        QString fullFileName = "Folder/File.ext";
        uint fileSize        = 0;
        QByteArray fileData;
        ushort dirID = 0;
    };

    Datapack() {}
    Datapack(QString filename) { read(filename); }
    Datapack(Reader &reader) { read(reader); }

    inline void read(QString filename)
    {
        Reader reader(filename);
        read(reader);
    }
    void read(Reader &reader);

    inline void write(QString filename)
    {
        if (filename == "")
            filename = filePath;
        if (filename == "")
            return;
        Writer writer(filename);
        write(writer);
    }
    void write(Writer &writer);

    QList<DirInfo> directories;
    QList<FileInfo> files;

    QString filePath = "";
};

} // namespace RSDKv2


