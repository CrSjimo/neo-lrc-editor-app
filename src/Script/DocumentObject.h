#ifndef NEOLRCEDITORAPP_DOCUMENTOBJECT_H
#define NEOLRCEDITORAPP_DOCUMENTOBJECT_H

#include <QObject>
#include <QJSValue>

class ItemObject;

class DocumentObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(int itemCount READ itemCount)
    Q_PROPERTY(int playbackPosition READ playbackPosition WRITE setPlaybackPosition)
    Q_PROPERTY(bool isPlaying READ isPlaying WRITE setPlaying)
public:
    explicit DocumentObject(QObject *parent = nullptr);
    ~DocumentObject() override;

public slots:
    QObject *insert(int time, const QString &lyric);

    QJSValue item(int index) const;
    QJSValue items() const;

    QJSValue selectedItems() const;
    void clearSelection();

    QObject *findItemByTime(int time) const;

private:
    int itemCount() const;

    void setPlaybackPosition(int time);
    int playbackPosition() const;

    void setPlaying(bool isPlaying);
    bool isPlaying() const;
};


#endif //NEOLRCEDITORAPP_DOCUMENTOBJECT_H
