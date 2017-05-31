#pragma once

#include <QtTest/QtTest>
class CodeBlockLoader : public QObject {
    Q_OBJECT
  private slots:
    void load_basic_data();
};
