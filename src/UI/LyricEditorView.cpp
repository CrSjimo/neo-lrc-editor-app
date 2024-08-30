#include "LyricEditorView.h"

#include <limits>
#include <cmath>

#include <QGraphicsScene>
#include <QGraphicsLineItem>
#include <QStandardItemModel>
#include <QWheelEvent>
#include <QStyleOptionGraphicsItem>
#include <QSortFilterProxyModel>
#include <QTreeView>
#include <QDialog>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QToolTip>

#include <TalcsGui/WaveformPainter.h>

#include <NeoLrcEditorApp/LyricDocument.h>
#include <NeoLrcEditorApp/MainWindow.h>
#include <NeoLrcEditorApp/PlaybackController.h>
#include <NeoLrcEditorApp/TimeValidator.h>

static LyricEditorView *m_view = nullptr;

class EditDialog : public QDialog {
public:
    explicit EditDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowFlags(Qt::Popup);
    }
    bool event(QEvent *event) override {
        switch (event->type()) {
            case QEvent::KeyPress:
            case QEvent::ShortcutOverride: {
                auto e = static_cast<QKeyEvent *>(event);
                int key = e->key();
                switch (key) {
                    case Qt::Key_Enter:
                    case Qt::Key_Return:
                        accept();
                        break;
                    case Qt::Key_Escape:
                        reject();
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
        return QDialog::event(event);
    }
};

class LyricLineItem : public QGraphicsItem {
public:

    QPersistentModelIndex index;
    QRectF m_boundingRect;

    explicit LyricLineItem(const QPersistentModelIndex &index, QGraphicsItem *parent = nullptr) : QGraphicsItem(parent), index(index) {
        setX(m_view->getItemXFromTime(time()));
        setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIsSelectable | QGraphicsItem::ItemSendsScenePositionChanges);
        setAcceptHoverEvents(true);
        setCursor(Qt::SizeHorCursor);
        updateBoundingRectBeforeRepaint();
    }

    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override {
        if (change == ItemPositionChange) {
            auto newPos = value.toPointF();
            newPos.setX(qMax(newPos.x(), 0.0));
            newPos.setY(0);
            LyricDocument::instance()->model()->setData(index, m_view->getTimeFromItemX(x()));
            auto affectedItem = previousItem();
            if (affectedItem)
                affectedItem->update();
            affectedItem = nextItem();
            if (affectedItem)
                affectedItem->update();
            return newPos;
        }
        return QGraphicsItem::itemChange(change, value);
    }

    QRectF boundingRect() const override {
        return m_boundingRect;
    }

    void updateBoundingRectBeforeRepaint() {
        auto textWidth = QFontMetrics(QFont()).horizontalAdvance(lyric());
        auto maximumWidth = spacingWidth();
        m_boundingRect |= {0, 0, qMin(maximumWidth, textWidth + 6.0), m_view->visibleRect().height()};
    }

    void updateBoundingRectAfterRepaint() {
        auto textWidth = QFontMetrics(QFont()).horizontalAdvance(lyric());
        auto maximumWidth = spacingWidth();
        m_boundingRect = {0, 0, qMin(maximumWidth, textWidth + 6.0), m_view->visibleRect().height()};
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
        auto lineColor = QColor(0x42, 0x63, 0xeb);
        auto textColor = QColor(Qt::black);

        painter->setRenderHint(QPainter::Antialiasing);

        auto lineBrush = QBrush(lineColor);
        painter->setBrush(lineBrush);
        painter->setPen(Qt::NoPen);
        painter->drawRect(QRectF(0, 0, 2, m_view->visibleRect().height()));

        auto textPen = QPen(textColor);
        painter->setPen(textPen);

        QString text;
        auto textWidth = painter->fontMetrics().horizontalAdvance(lyric());
        if (spacingWidth() - 6.0 < textWidth) {
            text = painter->fontMetrics().elidedText(lyric(), Qt::ElideRight, static_cast<int>(std::round(spacingWidth())) - 2);
        } else {
            text = lyric();
        }

        painter->drawText(QPointF(4, m_view->visibleRect().height() / 2), text);

        updateBoundingRectAfterRepaint();
    }

    int time() const {
        return index.model()->data(QModelIndex(index).siblingAtColumn(0)).toInt();
    }

    QString lyric() const {
        return index.model()->data(QModelIndex(index).siblingAtColumn(1)).toString();
    }

    LyricLineItem *nextItem() const {
        auto proxyModel = LyricDocument::instance()->proxyModel();
        auto proxyIndex = proxyModel->mapFromSource(index);
        auto proxyNextIndex = proxyIndex.siblingAtRow(proxyIndex.row() + 1);
        auto nextIndex = proxyModel->mapToSource(proxyNextIndex);
        return m_view->m_itemDict.value(nextIndex);
    }

    LyricLineItem *previousItem() const {
        auto proxyModel = LyricDocument::instance()->proxyModel();
        auto proxyIndex = proxyModel->mapFromSource(index);
        auto proxyNextIndex = proxyIndex.siblingAtRow(proxyIndex.row() - 1);
        auto nextIndex = proxyModel->mapToSource(proxyNextIndex);
        return m_view->m_itemDict.value(nextIndex);
    }

    double spacingWidth() const {
        auto item = nextItem();
        if (!item) {
            return std::numeric_limits<double>::max();
        } else {
            return item->x() - x();
        }
    }

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *event) override {
        QGraphicsItem::mousePressEvent(event);
        m_timeBeforeDragging = time();
        MainWindow::instance()->treeView()->setCurrentIndex(LyricDocument::instance()->proxyModel()->mapFromSource(index));
        update();
    }

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override {
        QGraphicsItem::mouseReleaseEvent(event);
        MainWindow::instance()->treeView()->setCurrentIndex(LyricDocument::instance()->proxyModel()->mapFromSource(index));
        auto newTime = m_view->getTimeFromItemX(x());
        if (newTime != m_timeBeforeDragging) {
            LyricDocument::instance()->beginTransaction("Edit Time");
            LyricDocument::instance()->pushEditCommand(index, newTime, m_timeBeforeDragging);
            LyricDocument::instance()->commitTransaction();
        }
        update();
    }
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override {
        QGraphicsItem::mouseDoubleClickEvent(event);
        EditDialog dlg;
        auto lineEdit = new QLineEdit;
        lineEdit->setText(lyric());

        QFontMetrics fm(lineEdit->font());
        auto offset = fm.horizontalAdvance(" ") * 2;
        auto adjust = [&](const QString &text) {
            dlg.resize(fm.horizontalAdvance(text) + offset * 4, lineEdit->height());
        };
        QObject::connect(lineEdit, &QLineEdit::textChanged, &dlg, adjust);

        auto layout = new QHBoxLayout();
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(lineEdit);

        dlg.setLayout(layout);
        dlg.adjustSize();
        dlg.move(m_view->mapToGlobal(m_view->mapFromScene(QPointF(x() + 4.0, boundingRect().height() / 2.0 - dlg.height() / 2.0).toPoint())));

        lineEdit->setFocus();
        adjust(lineEdit->text());
        if (dlg.exec() == QDialog::Accepted) {
            LyricDocument::instance()->beginTransaction("Edit Lyric");
            LyricDocument::instance()->pushEditCommand(QModelIndex(index).siblingAtColumn(1), lineEdit->text());
            LyricDocument::instance()->commitTransaction();
        }
    }

private:
    int m_timeBeforeDragging = 0;
};

class WaveformItem : public QGraphicsItem {
public:
    QRectF m_boundingRect;

    explicit WaveformItem(QGraphicsItem *parent = nullptr) : QGraphicsItem(parent) {
    }

    ~WaveformItem() override = default;

    QRectF boundingRect() const override {
        return m_boundingRect;
    }

    void updateBoundingRectBeforeRepaint() {
        m_boundingRect |= {0, 0, m_view->getItemXFromTime(PlaybackController::instance()->audioLengthTime()), m_view->visibleRect().height()};
    }

    void updateBoundingRectAfterRepaint() {
        m_boundingRect = {0, 0, m_view->getItemXFromTime(PlaybackController::instance()->audioLengthTime()), m_view->visibleRect().height()};
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
        auto rect = boundingRect().intersected(m_view->visibleRect());
        painter->setBrush(QColor(0xcc, 0xcc, 0xcc));
        painter->setPen(Qt::NoPen);
        auto startSecond = m_view->getSecondFromItemX(rect.left());
        auto lengthSecond = m_view->getSecondFromItemX(rect.width());
        PlaybackController::instance()->waveformPainter()->paint(painter, rect.toRect(), startSecond, lengthSecond);
        updateBoundingRectAfterRepaint();
    }
};

class PlayheadItem : public QGraphicsItem {
public:
    explicit PlayheadItem(QGraphicsItem *parent = nullptr) : QGraphicsItem(parent) {
    }

    ~PlayheadItem() override = default;

    QRectF boundingRect() const override {
        return {0, 0, 1, m_view->visibleRect().height()};
    }

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override {
        painter->setRenderHint(QPainter::Antialiasing);
        painter->setBrush(Qt::red);
        painter->setPen(Qt::NoPen);
        painter->drawRect(boundingRect());
    }
};

LyricEditorView::LyricEditorView(QWidget *parent) : QGraphicsView(parent) {
    m_view = this;
    m_scene = new QGraphicsScene(this);
    setScene(m_scene);
    setAlignment(Qt::AlignLeft | Qt::AlignTop);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    verticalScrollBar()->setEnabled(false);
    setMouseTracking(true);

    m_waveformItem = new WaveformItem;
    m_scene->addItem(m_waveformItem);

    m_playheadItem = new PlayheadItem;
    m_playheadItem->setZValue(1);
    m_scene->addItem(m_playheadItem);

    connect(m_scene, &QGraphicsScene::changed, this, [=] {
        QRectF rect = m_scene->itemsBoundingRect();
        m_scene->setSceneRect(rect);
    });


    auto model = LyricDocument::instance()->model();

    connect(model, &QStandardItemModel::rowsInserted, this, [=](const QModelIndex &, int first, int last) {
        for (int row = first; row <= last; row++) {
            auto index = model->index(row, 0);
            auto item = new LyricLineItem(index);
            m_itemDict.insert(index, item);
            m_scene->addItem(item);

            auto affectedItem = item->previousItem();
            if (affectedItem)
                affectedItem->update();
        }
    });
    connect(model, &QStandardItemModel::dataChanged, this, [=](const QModelIndex &topLeft, const QModelIndex &bottomRight) {
        for (int row = topLeft.row(); row <= bottomRight.row(); row++) {
            auto index = model->index(row, 0);
            auto item = m_itemDict.value(index);
            item->setX(getItemXFromTime(item->time()));
            item->updateBoundingRectBeforeRepaint();
            item->update();
            auto affectedItem = item->previousItem();
            if (affectedItem)
                affectedItem->update();
        }
    });
    connect(model, &QStandardItemModel::rowsAboutToBeRemoved, this, [=](const QModelIndex &, int first, int last) {
        for (int row = first; row <= last; row++) {
            auto index = QPersistentModelIndex(model->index(row, 0));
            auto item = m_itemDict.value(index);
            m_scene->removeItem(item);
            m_itemDict.remove(index);
            auto affectedItem = item->previousItem();
            if (affectedItem)
                affectedItem->update();
            delete item;
        }
    });
    connect(model, &QStandardItemModel::modelAboutToBeReset, this, [=] {
        for (auto item : m_itemDict.values()) {
            m_scene->removeItem(item);
        }
        m_itemDict.clear();
    });

    connect(PlaybackController::instance()->waveformPainter(), &talcs::WaveformPainter::loadFinished, this, [=] {
        m_waveformItem->updateBoundingRectBeforeRepaint();
        m_waveformItem->update();
    });

    connect(PlaybackController::instance(), &PlaybackController::positionTimeChanged, this, [=](int time) {
        m_playheadItem->setX(getItemXFromTime(time));
        auto rect = visibleRect();
        if (rect.right() - m_playheadItem->x() <= 50) {
            centerOn(m_playheadItem->x() + rect.width() / 2 - 50, rect.center().y());
        } else if (m_playheadItem->x() - rect.left() < 50) {
            centerOn(m_playheadItem->x() - rect.width() / 2 + 50, rect.center().y());
        }
    });
}

LyricEditorView::~LyricEditorView() {
    m_view = nullptr;
}

double LyricEditorView::getItemXFromTime(int timeValue) const {
    return timeValue * std::pow(2, m_scaleRate);
}

int LyricEditorView::getTimeFromItemX(double x) const {
    return static_cast<int>(std::round(x / std::pow(2, m_scaleRate)));
}

double LyricEditorView::getSecondFromItemX(double x) const {
    return x / std::pow(2, m_scaleRate) / 100.0;
}

double LyricEditorView::getItemXFromSecond(double second) const {
    return second * 100 * std::pow(2, m_scaleRate);
}

QRectF LyricEditorView::visibleRect() const  {
    QPointF A = mapToScene(QPoint(0, 0) );
    QPointF B = mapToScene(QPoint(viewport()->width(), viewport()->height()));
    return {A, B};
}

void LyricEditorView::wheelEvent(QWheelEvent *event) {
    if (event->modifiers() & Qt::ControlModifier) {
        double itemXBeforeScaling = mapToScene(event->position().toPoint()).x();
        double secondPointingTo = getSecondFromItemX(itemXBeforeScaling);
        if (event->angleDelta().y() > 0) {
            m_scaleRate += 0.25;
        } else {
            m_scaleRate -= 0.25;
        }
        updateItemPositionAfterScaling();
        double itemXAfterScaling = getItemXFromSecond(secondPointingTo);
        qDebug() << itemXBeforeScaling << secondPointingTo << itemXAfterScaling;
        auto center = visibleRect().center();
        centerOn(center.x() + itemXAfterScaling - itemXBeforeScaling, center.y());
        event->accept();
    } else {
        QGraphicsView::wheelEvent(event);
    }
}

void LyricEditorView::mouseMoveEvent(QMouseEvent *event) {
    auto time = getTimeFromItemX(mapToScene(event->pos()).x());
    QToolTip::showText(event->globalPosition().toPoint(), {}, viewport());
    QToolTip::showText(event->globalPosition().toPoint(), TimeValidator::timeToString(time), viewport());
    QGraphicsView::mouseMoveEvent(event);
}

void LyricEditorView::updateItemPositionAfterScaling() {
    for (auto item : m_itemDict.values()) {
        item->setX(getItemXFromTime(item->time()));
    }
    m_waveformItem->updateBoundingRectBeforeRepaint();
    m_waveformItem->update();
    m_playheadItem->setX(getItemXFromTime(PlaybackController::instance()->positionTime()));
}
