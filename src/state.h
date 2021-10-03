#pragma once

// On Qt 6, you can use <QtQml/qqmlregistration.h> instead.
#include <QtQml/qqml.h>

namespace state {

class AppState : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(QString qt_version READ qt_version CONSTANT)
public:
    // AppState()
    using QObject::QObject;

    QString qt_version() const;
};

}
