#include "LyricEditorView.h"

#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QStandardItemModel>
#include <QWheelEvent>
#include <QStyleOptionGraphicsItem>

#include <NeoLrcEditorApp/LyricDocument.h>

static LyricEditorView *m_view = nullptr;

class LyricLineItem : public QGraphicsItem {
public:
    explicit LyricLineItem(const QPersistentModelIndex &index, QGraphicsItem *parent = nullptr) : QGraphicsItem(parent), index(index) {
        setX(m_view->getItemX(index.model()->data(index).toInt()));
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsScenePositionChanges);
        setAcceptHoverEvents(true);
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
        qDebug() << change << value;
        if (change == ItemPositionChange) {
            auto newPos = value.toPointF();
            newPos.setX(qMax(newPos.x(), 0.0));
            newPos.setY(0);
            return newPos;
        }
        return QGraphicsItem::itemChange(change, value);
    }

    QRectF boundingRect() const override {
        return {0, 0, 100, 160};
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
        auto lineColor = QColor(0x42, 0x63, 0xeb);
        auto textColor = QColor(Qt::black);

        auto lineBrush = QBrush(lineColor);
        painter->setBrush(lineBrush);
        painter->setPen(Qt::NoPen);
        painter->drawRect(0, 0, 2, 160);
    }

    QPersistentModelIndex index;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        QGraphicsItem::mousePressEvent(event);
        update();
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        QGraphicsItem::mouseReleaseEvent(event);
        update();
    }
};

LyricEditorView::LyricEditorView(QWidget *parent) : QGraphicsView(parent) {
    m_view = this;
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setFixedHeight(160);
    auto model = LyricDocument::instance()->model();
    m_scene->addItem(new QGraphicsRectItem);
    connect(model, &QStandardItemModel::rowsInserted, this, [=](const QModelIndex &, int first, int last) {
        for (int row = first; row <= last; row++) {
            auto index = QPersistentModelIndex(model->index(row, 0));
            auto item = new LyricLineItem(index);
            m_scene->addItem(item);
        }
    });
    connect(model, &QStandardItemModel::rowsAboutToBeRemoved, this, [=](const QModelIndex &, int first, int last) {
        qDebug() << first << last;
    });
}

LyricEditorView::~LyricEditorView() {
    m_view = nullptr;
}

double LyricEditorView::getItemX(int timeValue) const {
    return timeValue * std::pow(2, m_scaleRate);
}

void LyricEditorView::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        if (event->angleDelta().y() > 0) {
            m_scaleRate += 0.25;
        } else {
            m_scaleRate -= 0.25;
        }
        updateItemPositionAfterScaling();
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void LyricEditorView::updateItemPositionAfterScaling() {
    for (auto item : m_itemDict.values()) {
        item->setX(getItemX(item->index.model()->data(item->index).toInt()));
    }
}
