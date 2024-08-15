#ifndef NEOLRCEDITORAPP_LYRICEDITORVIEW_H
#define NEOLRCEDITORAPP_LYRICEDITORVIEW_H

#include <QGraphicsView>

class LyricEditorView : public QGraphicsView {
    Q_OBJECT
public:
    explicit LyricEditorView(QWidget *parent = nullptr);
    ~LyricEditorView() override;

protected:
    void resizeEvent(QResizeEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    QGraphicsScene *m_scene;
};


#endif //NEOLRCEDITORAPP_LYRICEDITORVIEW_H
