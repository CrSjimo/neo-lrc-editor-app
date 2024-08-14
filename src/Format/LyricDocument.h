#ifndef NEOLRCEDITORAPP_LYRICDOCUMENT_H
#define NEOLRCEDITORAPP_LYRICDOCUMENT_H

#include <QObject>

class QStandardItemModel;
class QUndoStack;

class LyricLine;

class LyricDocument : public QObject {
    Q_OBJECT
public:
    explicit LyricDocument(QObject *parent = nullptr);
    ~LyricDocument() override;

    static LyricDocument *instance();

    void newFile();
    bool openFile(const QString &fileName);
    bool saveFile();
    bool saveFileAs(const QString &fileName);

    bool isDirty() const;

    QStandardItemModel *model() const;
    QUndoStack *undoStack() const;
    void beginTransaction(const QString &name);
    void pushEditCommand(const QModelIndex &index, const QVariant &value);
    void commitTransaction();

signals:
    void fileNameChanged(const QString &fileName);
    void dirtyChanged(bool isDirty);

private:
    void buildModelFromLyricLines(const QList<LyricLine> &lyricLines);
    QList<LyricLine> getLyricLinesFromModel() const;

    void setFileName(const QString &fileName);
    void setDirty(bool isDirty);

    QStandardItemModel *m_lyricModel;
    QUndoStack *m_undoStack;
    QString m_fileName;
    bool m_isDirty = false;
};


#endif //NEOLRCEDITORAPP_LYRICDOCUMENT_H
