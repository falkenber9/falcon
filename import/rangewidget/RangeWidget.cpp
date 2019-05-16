#include "RangeWidget.h"

#include <QtDebug>

RangeWidget::RangeWidget(Qt::Orientation orientation, QWidget *parent)
    : QWidget(parent),
      _orientation(orientation),
      _handleWidth(8),
      _handleHeight(20),
      _minimum(0),
      _maximum(100),
      _firstValue(10),
      _secondValue(90),
      _firstHandlePressed(false),
      _secondHandlePressed(false),
      _firstHandleColor(style()->standardPalette().highlight().color()),
      _secondHandleColor(style()->standardPalette().highlight().color())
{
    setMouseTracking(true);
}

void RangeWidget::paintEvent(QPaintEvent *event)
{
    QPainter p(this);

    // First value handle rect
    QRectF rv1 = firstHandleRect();
    QColor c1(_firstHandleColor);
    if(_firstHandleHovered)
        c1 = c1.darker();

    // Second value handle rect
    QRectF rv2 = secondHandleRect();
    QColor c2(_secondHandleColor);
    if(_secondHandleHovered)
        c2 = c2.darker();

    // Background
    QRect r;
    if(_orientation == Qt::Horizontal)
        r = QRect(0, (height()-_handleWidth)/2, width()-1, _handleWidth);
    else
        r = QRect((width()-_handleWidth)/2, 0, _handleWidth, height()-1);
    p.drawRect(r);

    // Handles
    QRectF rf(r);
    if(_orientation == Qt::Horizontal)
    {
        rf.setLeft(rv1.right());
        rf.setRight(rv2.left());
        rf.setBottom(rf.bottom()+1);
    }
    else
    {
        rf.setTop(rv1.bottom());
        rf.setBottom(rv2.top());
        rf.setRight(rf.right()+1);
    }
    p.fillRect(rf, QColor(Qt::green).darker(150));
    p.fillRect(rv1, c1);
    p.fillRect(rv2, c2);
}

qreal RangeWidget::span() const
{
    int interval = qAbs(_maximum-_minimum);

    if(_orientation == Qt::Horizontal)
        return qreal(width()-_handleWidth)/qreal(interval);
    else
        return qreal(height()-_handleWidth)/qreal(interval);
}

QRectF RangeWidget::firstHandleRect() const
{
    return handleRect(_firstValue);
}

QRectF RangeWidget::secondHandleRect() const
{
    return handleRect(_secondValue);
}

QRectF RangeWidget::handleRect(int value) const
{
    qreal s = span();

    QRectF r;
    if(_orientation == Qt::Horizontal)
    {
        r = QRectF(0, (height()-_handleHeight)/2, _handleWidth, _handleHeight);
        r.moveLeft(s*(value-_minimum));
    }
    else
    {
        r = QRectF((width()-_handleHeight)/2, 0, _handleHeight, _handleWidth);
        r.moveTop(s*(value-_minimum));
    }
    return r;
}

void RangeWidget::mousePressEvent(QMouseEvent* event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        _secondHandlePressed = secondHandleRect().contains(event->pos());
        _firstHandlePressed = !_secondHandlePressed && firstHandleRect().contains(event->pos());
        emit sliderPressed();
    }
}

void RangeWidget::mouseMoveEvent(QMouseEvent* event)
{
    if(event->buttons() & Qt::LeftButton)
    {
        int interval = qAbs(_maximum-_minimum);

        if(_secondHandlePressed)
        {
            if(_orientation == Qt::Horizontal)
                setSecondValue(event->pos().x()*interval/(width()-_handleWidth));
            else
                setSecondValue(event->pos().y()*interval/(height()-_handleWidth));
        }
        else if(_firstHandlePressed)
        {
            if(_orientation == Qt::Horizontal)
                setFirstValue(event->pos().x()*interval/(width()-_handleWidth));
            else
                setFirstValue(event->pos().y()*interval/(height()-_handleWidth));
        }
    }

    QRectF rv2 = secondHandleRect();
    QRectF rv1 = firstHandleRect();
    _secondHandleHovered = _secondHandlePressed || (!_firstHandlePressed && rv2.contains(event->pos()));
    _firstHandleHovered = _firstHandlePressed || (!_secondHandleHovered && rv1.contains(event->pos()));
    update(rv2.toRect());
    update(rv1.toRect());
}

void RangeWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if(_firstHandlePressed || _secondHandlePressed)
        emit sliderReleased();

    _firstHandlePressed = false;
    _secondHandlePressed = false;
}

QSize RangeWidget::minimumSizeHint() const
{
    return QSize(_handleHeight, _handleHeight);
}

void RangeWidget::setSecondValue(int secondValue)
{
    if(secondValue > _maximum)
        secondValue = _maximum;

    if(secondValue < _minimum)
        secondValue = _minimum;

    _secondValue = secondValue;
    emit secondValueChanged(_secondValue);

    update();
}

void RangeWidget::setFirstValue(int firstValue)
{
    if(firstValue > _maximum)
        firstValue = _maximum;

    if(firstValue < _minimum)
        firstValue = _minimum;

    _firstValue = firstValue;
    emit firstValueChanged(_firstValue);

    update();
}

void RangeWidget::setMaximum(int max)
{
    if(max >= minimum())
        _maximum = max;
    else
    {
        int oldMin = minimum();
        _maximum = oldMin;
        _minimum = max;
    }

    update();

    if(firstValue() > maximum())
        setFirstValue(maximum());

    if(secondValue() > maximum())
        setSecondValue(maximum());

    emit rangeChanged(minimum(), maximum());
}

void RangeWidget::setRange(int min, int max)
{
    setMinimum(min);
    setMaximum(max);
}

void RangeWidget::setMinimum(int min)
{
    if(min <= maximum())
        _minimum = min;
    else
    {
        int oldMax = maximum();
        _minimum = oldMax;
        _maximum = min;
    }

    update();

    if(firstValue() < minimum())
        setFirstValue(minimum());

    if(secondValue() < minimum())
        setSecondValue(minimum());

    emit rangeChanged(minimum(), maximum());
}

void RangeWidget::setOrientation(Qt::Orientation orientation)
{
    if(_orientation == orientation)
        return;

    _orientation = orientation;
    update();
}
