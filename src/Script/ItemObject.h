#ifndef NEOLRCEDITORAPP_ITEMOBJECT_H
#define NEOLRCEDITORAPP_ITEMOBJECT_H

#include <QObject>
#include <QPersistentModelIndex>

class ItemObject : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString lyric READ lyric WRITE setLyric)
    Q_PROPERTY(int time READ time WRITE setTime)
    Q_PROPERTY(int index READ index)
public:
    explicit ItemObject(const QPersistentModelIndex &index, QObject *parent = nullptr);
    ~ItemObject() override;

public slots:
    void remove();

private:
    void setLyric(const QString &lyric);
    QString lyric() const;

    void setTime(int time);
    int time() const;

    void setSelected(bool selected);
    bool isSelected() const;

    int index() const;

    QPersistentModelIndex m_index;
};


#endif //NEOLRCEDITORAPP_ITEMOBJECT_H
