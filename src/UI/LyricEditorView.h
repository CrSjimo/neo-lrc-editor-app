#ifndef NEOLRCEDITORAPP_LYRICEDITORVIEW_H
#define NEOLRCEDITORAPP_LYRICEDITORVIEW_H

#include <QGraphicsView>

class LyricLineItem;

class LyricEditorView : public QGraphicsView {
    Q_OBJECT
public:
    explicit LyricEditorView(QWidget *parent = nullptr);
    ~LyricEditorView() override;

    double getItemX(int timeValue) const;

protected:
    void wheelEvent(QWheelEvent *event) override;

private:
    QGraphicsScene *m_scene;
    QHash<QPersistentModelIndex, LyricLineItem *> m_itemDict;

    double m_scaleRate = 0;

    void updateItemPositionAfterScaling();
};


#endif //NEOLRCEDITORAPP_LYRICEDITORVIEW_H
