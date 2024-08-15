#include "LyricEditorView.h"

#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QStandardItemModel>

#include <NeoLrcEditorApp/LyricDocument.h>

class LineItem : public QGraphicsLineItem {
public:
    explicit LineItem(const QPersistentModelIndex &index, QGraphicsItem *parent = nullptr) : QGraphicsLineItem(parent), index(index) {
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemSendsGeometryChanges);
    }

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
        if (change == QGraphicsItem::ItemPositionChange) {
            QPointF newPos = value.toPointF();
            LyricDocument::instance()->model()->setData(index, newPos.x());
        }
        return QGraphicsItem::itemChange(change, value);
    }

private:
    QPersistentModelIndex index;
};

LyricEditorView::LyricEditorView(QWidget *parent) : QGraphicsView(parent) {
    m_scene = new QGraphicsScene(this);
}

LyricEditorView::~LyricEditorView() {

}

void LyricEditorView::resizeEvent(QResizeEvent *event) {
    QGraphicsView::resizeEvent(event);
}

void LyricEditorView::wheelEvent(QWheelEvent *event) {
    QGraphicsView::wheelEvent(event);
}
