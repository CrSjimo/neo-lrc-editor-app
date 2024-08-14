#ifndef NEOLRCEDITORAPP_LYRICEDITORVIEW_H
#define NEOLRCEDITORAPP_LYRICEDITORVIEW_H

#include <QGraphicsView>

class LyricEditorView : public QGraphicsView {
    Q_OBJECT
public:
    explicit LyricEditorView(QWidget *parent = nullptr);
    ~LyricEditorView() override;
};


#endif //NEOLRCEDITORAPP_LYRICEDITORVIEW_H
