#ifndef NEOLRCEDITORAPP_LYRICDOCUMENT_H
#define NEOLRCEDITORAPP_LYRICDOCUMENT_H

#include <QObject>

class QStandardItemModel;
class QUndoStack;
class QSortFilterProxyModel;

class LyricLine;

class LyricSortFilterProxyModel;

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

    QString fileName() const;

    void setDirty(bool isDirty);
    bool isDirty() const;

    QStandardItemModel *model() const;
    QSortFilterProxyModel *proxyModel() const;
    QUndoStack *undoStack() const;
    void beginTransaction(const QString &name);
    void pushEditCommand(const QModelIndex &index, const QVariant &value);
    void pushEditCommand(const QModelIndex &index, const QVariant &value, const QVariant &previousValue);
    void pushMoveRowCommand(int sourceRow, int destinationRow);
    void pushInsertRowCommand(int row, int time, const QString &lyric);
    void pushDeleteRowCommand(int row);
    void commitTransaction();
    void abortTransaction();

    int findRowByTime(int time) const;

signals:
    void fileNameChanged(const QString &fileName);
    void dirtyChanged(bool isDirty);

private:
    void buildModelFromLyricLines(const QList<LyricLine> &lyricLines);
    QList<LyricLine> getLyricLinesFromModel() const;

    void setFileName(const QString &fileName);

    QStandardItemModel *m_lyricModel;
    LyricSortFilterProxyModel *m_proxyModel;
    QUndoStack *m_undoStack;
    QString m_fileName;
    bool m_isDirty = false;
};


#endif //NEOLRCEDITORAPP_LYRICDOCUMENT_H
