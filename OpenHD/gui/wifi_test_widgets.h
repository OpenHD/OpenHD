#pragma once
#include <QColor>
#include <QComboBox>
#include <QGridLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QResizeEvent>
#include <QWidget>

#include "wifi_test.h"

namespace openhd::ui {
class TestDropdown : public QComboBox {
 protected:
  void paintEvent(QPaintEvent* e) override {
    QComboBox::paintEvent(e);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QPen(isEnabled() ? QColor("#a8c2d5") : QColor("#536a7a"), 1.5));
    const int x = width() - 13, y = height() / 2;
    p.drawLine(x - 3, y - 2, x, y + 1);
    p.drawLine(x, y + 1, x + 3, y - 2);
  }
};
inline QColor segment_color(int segment) {
  static const QColor colors[] = {QColor("#2388ee"), QColor("#16a875"),
                                  QColor("#7058eb"), QColor("#ee7948"),
                                  QColor("#1ab8cb")};
  return colors[(segment > 0 ? segment - 1 : 0) % 5];
}
inline QString elapsed_text(qint64 us) {
  const auto seconds = us / 1000000;
  return QString("%1:%2")
      .arg(seconds / 60, 2, 10, QChar('0'))
      .arg(seconds % 60, 2, 10, QChar('0'));
}
class TestTimeline : public QWidget {
 public:
  wifi_test::Json events = wifi_test::Json::array();
  qint64 elapsed_us = 0;
  TestTimeline() { setMinimumHeight(104); }

 protected:
  void paintEvent(QPaintEvent*) override {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    const QRectF bar(12, 14, width() - 24, 26);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor("#142431"));
    p.drawRoundedRect(bar, 4, 4);
    const qint64 scale = std::max<qint64>(60000000, elapsed_us);
    auto x = [&](qint64 time) {
      return bar.left() + bar.width() * double(time) / double(scale);
    };
    for (size_t i = 0; i < events.size(); ++i) {
      const auto& e = events[i];
      if (e.value("type", "") != "applied") continue;
      qint64 end = elapsed_us;
      for (size_t n = i + 1; n < events.size(); ++n) {
        const auto type = events[n].value("type", "");
        if (type == "segment_end" || type == "stop" || type == "error") {
          end = events[n].value("elapsed_us", int64_t(0));
          break;
        }
      }
      const auto start = e.value("elapsed_us", int64_t(0));
      p.setBrush(segment_color(e.value("segment", 0)));
      p.drawRect(QRectF(x(start), bar.top(), std::max(1.0, x(end) - x(start)),
                        bar.height()));
    }
    p.setPen(QColor("#3b5364"));
    p.drawLine(QPointF(bar.left(), 57), QPointF(bar.right(), 57));
    for (int i = 0; i <= 5; ++i) {
      const qint64 t = scale * i / 5;
      const auto pos = x(t);
      p.drawLine(QPointF(pos, 57), QPointF(pos, 64));
      p.setPen(QColor("#a8bbc8"));
      p.drawText(QRectF(pos - 25, 68, 50, 24), Qt::AlignCenter,
                 elapsed_text(t));
      p.setPen(QColor("#3b5364"));
    }
    p.setPen(QPen(QColor("#e4f2fa"), 2));
    p.drawLine(QPointF(x(elapsed_us), 7), QPointF(x(elapsed_us), 47));
  }
};
// Wrap the settings panels on small Pi touchscreens while retaining the
// reference's single row on desktop displays.
class TestControlPanels : public QWidget {
 public:
  QGridLayout* grid = new QGridLayout(this);
  QList<QWidget*> panels;
  TestControlPanels() {
    grid->setSizeConstraint(QLayout::SetNoConstraint);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  }
  QSize minimumSizeHint() const override { return QSize(0, 0); }
  void arrange(int available) {
    setFixedWidth(available);
    reflow();
  }
  void add(QWidget* panel) {
    panels.push_back(panel);
    reflow();
  }

 protected:
  void resizeEvent(QResizeEvent* event) override {
    QWidget::resizeEvent(event);
    reflow();
  }

 private:
  int previous = -1;
  void reflow() {
    const int columns = width() >= 1080 ? 7 : width() >= 700 ? 4 : 2;
    if (columns == previous && grid->count() == panels.size()) return;
    previous = columns;
    while (auto* item = grid->takeAt(0)) delete item;
    for (int i = 0; i < panels.size(); ++i)
      grid->addWidget(panels[i], i / columns, i % columns);
    setFixedHeight(grid->minimumSize().height() + 8);
  }
};
}  // namespace openhd::ui
