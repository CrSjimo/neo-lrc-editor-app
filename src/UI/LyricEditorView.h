#ifndef NEOLRCEDITORAPP_LYRICEDITORVIEW_H
#define NEOLRCEDITORAPP_LYRICEDITORVIEW_H

#include <QGraphicsView>

class LyricLineItem;
class WaveformItem;

class LyricEditorView : public QGraphicsView {
    Q_OBJECT
    friend class LyricLineItem;
public:
    explicit LyricEditorView(QWidget *parent = nullptr);
    ~LyricEditorView() override;

    double getItemXFromTime(int timeValue) const;
    int getTimeFromItemX(double x) const;
    double getSecondFromItemX(double x) const;
    double getItemXFromSecond(double second) const;

    QRectF visibleRect() const;

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QGraphicsScene *m_scene;
    QHash<QPersistentModelIndex, LyricLineItem *> m_itemDict;
    QGraphicsItem *m_playheadItem;
    WaveformItem *m_waveformItem;

    double m_scaleRate = 0;

    void updateItemPositionAfterScaling();
};


#endif //NEOLRCEDITORAPP_LYRICEDITORVIEW_H
