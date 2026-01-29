#include "mainwindow.h"
#include <QApplication>
#include <QCoreApplication>
#include <QPluginLoader>
#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUi()
{
    // Determine the appropriate plugin extension based on the platform
    QString pluginExtension;
    #if defined(Q_OS_WIN)
        pluginExtension = ".dll";
    #elif defined(Q_OS_MAC)
        pluginExtension = ".dylib";
    #else // Linux and other Unix-like systems
        pluginExtension = ".so";
    #endif

    QString pluginPath = QCoreApplication::applicationDirPath() + "/../waku_ui" + pluginExtension;
    QPluginLoader loader(pluginPath);

    QWidget* wakuWidget = nullptr;

    if (loader.load()) {
        QObject* plugin = loader.instance();
        if (plugin) {
            // Try to create the waku widget using the plugin's createWidget method
            QMetaObject::invokeMethod(plugin, "createWidget",
                                    Qt::DirectConnection,
                                    Q_RETURN_ARG(QWidget*, wakuWidget));
        }
    }

    if (wakuWidget) {
        setCentralWidget(wakuWidget);
    } else {
        qWarning() << "================================================";
        qWarning() << "Failed to load waku UI plugin from:" << pluginPath;
        qWarning() << "Error:" << loader.errorString();
        qWarning() << "================================================";
        
        // Fallback: show a message when plugin is not found
        QWidget* fallbackWidget = new QWidget(this);
        QVBoxLayout* layout = new QVBoxLayout(fallbackWidget);

        QLabel* messageLabel = new QLabel("Waku UI module not loaded", fallbackWidget);
        QFont font = messageLabel->font();
        font.setPointSize(14);
        messageLabel->setFont(font);
        messageLabel->setAlignment(Qt::AlignCenter);

        layout->addWidget(messageLabel);
        setCentralWidget(fallbackWidget);
    }

    // Set window title and size
    setWindowTitle("Logos Waku UI App");
    resize(800, 600);
}
