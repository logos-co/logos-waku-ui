#include "WakuPlugin.h"
#include "WakuBackend.h"
#include <QQuickWidget>
#include <QQmlContext>
#include <QQmlEngine>
#include <QDebug>
#include <QFileInfo>
#include <QFile>

QWidget* WakuPlugin::createWidget(LogosAPI* logosAPI) {
    qDebug() << "WakuPlugin::createWidget called";

    QQuickWidget* quickWidget = new QQuickWidget();
    quickWidget->setResizeMode(QQuickWidget::SizeRootObjectToView);

    qmlRegisterType<WakuBackend>("WakuBackend", 1, 0, "WakuBackend");

    WakuBackend* backend = new WakuBackend(logosAPI, quickWidget);
    
    quickWidget->rootContext()->setContextProperty("backend", backend);

    QString qmlPath = "qrc:/WakuView.qml";
    QString envPath = qgetenv("WAKU_UI_QML_PATH");
    if (!envPath.isEmpty() && QFile::exists(envPath)) {
        qmlPath = QUrl::fromLocalFile(QFileInfo(envPath).absoluteFilePath()).toString();
        qDebug() << "Loading QML from file system:" << qmlPath;
    }
    
    quickWidget->setSource(QUrl(qmlPath));
    
    if (quickWidget->status() == QQuickWidget::Error) {
        qWarning() << "WakuPlugin: Failed to load QML:" << quickWidget->errors();
    }

    return quickWidget;
}

void WakuPlugin::destroyWidget(QWidget* widget) {
    delete widget;
}
