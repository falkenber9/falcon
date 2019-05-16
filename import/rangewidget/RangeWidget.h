#ifndef RANGEWIDGET_H
#define RANGEWIDGET_H

#include <QWidget>
#include <QPainter>
#include <QStyle>
#include <QMouseEvent>

class RangeWidget : public QWidget
{
    Q_OBJECT
private:
    Q_DISABLE_COPY(RangeWidget)

    Qt::Orientation _orientation;

    int _handleWidth;
    int _handleHeight;

    int _minimum;
    int _maximum;

    int _firstValue;
    int _secondValue;

    bool _firstHandlePressed;
    bool _secondHandlePressed;

    bool _firstHandleHovered;
    bool _secondHandleHovered;

    QColor _firstHandleColor;
    QColor _secondHandleColor;

protected:
    void paintEvent(QPaintEvent* event);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);

    QRectF firstHandleRect() const;
    QRectF secondHandleRect() const;
    QRectF handleRect(int value) const;
    qreal span() const;

public:
    RangeWidget(Qt::Orientation orientation = Qt::Vertical, QWidget *parent = nullptr);

    QSize minimumSizeHint() const;

    inline int firstValue() const { return _firstValue; }
    inline int secondValue() const { return _secondValue; }
    inline int minimum() const { return _minimum; }
    inline int maximum() const { return _maximum; }
    inline Qt::Orientation orientation() const { return _orientation; }
    inline int interval() const { return secondValue()-firstValue(); }
    inline unsigned int absInterval() const { return qAbs(interval()); }

signals:
    void firstValueChanged(int firstValue);
    void secondValueChanged(int secondValue);
    void rangeChanged(int min, int max);
    void sliderPressed();
    void sliderReleased();

public slots:
    void setFirstValue(int firstValue);
    void setSecondValue(int secondValue);
    void setMinimum(int min);
    void setMaximum(int max);
    void setRange(int min, int max);
    void setOrientation(Qt::Orientation orientation);

};

#endif // RANGEWIDGET_H
