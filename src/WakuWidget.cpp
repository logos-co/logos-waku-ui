#include "WakuWidget.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaType>
#include "logos_api_client.h"

WakuWidget::WakuWidget(QWidget* parent) 
    : QWidget(parent), 
      isWakuRunning(false),
      m_logosAPI(nullptr),
      metricsTimer(nullptr) {
    
    qDebug() << "Initializing WakuWidget...";
    m_logosAPI = new LogosAPI("core", this);
    logos = new LogosModules(m_logosAPI);
    
    // Main vertical layout
    mainLayout = new QVBoxLayout(this);
    
    // Create status label
    statusLabel = new QLabel("Status: Not started", this);
    statusLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    statusLabel->setLineWidth(1);
    statusLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    statusLabel->setMinimumHeight(30);
    
    // Create button layout
    buttonLayout = new QHBoxLayout();
    startButton = new QPushButton("Start Waku", this);
    stopButton = new QPushButton("Stop Waku", this);
    refreshPeersButton = new QPushButton("Refresh Peers", this);
    refreshMetricsButton = new QPushButton("Refresh Metrics", this);
    
    stopButton->setEnabled(false);
    refreshPeersButton->setEnabled(false);
    refreshMetricsButton->setEnabled(false);
    
    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(stopButton);
    buttonLayout->addWidget(refreshPeersButton);
    buttonLayout->addWidget(refreshMetricsButton);
    buttonLayout->addStretch();
    
    // Create peers section
    peersLabel = new QLabel("Connected Peers:", this);
    peersList = new QListWidget(this);
    peersList->setMinimumHeight(150);
    peersList->setMaximumHeight(200);
    
    // Create metrics section
    metricsLabel = new QLabel("Metrics:", this);
    metricsDisplay = new QTextEdit(this);
    metricsDisplay->setReadOnly(true);
    metricsDisplay->setMinimumHeight(200);
    metricsDisplay->setFont(QFont("Courier", 9));
    
    // Add all components to main layout
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addWidget(peersLabel);
    mainLayout->addWidget(peersList);
    mainLayout->addWidget(metricsLabel);
    mainLayout->addWidget(metricsDisplay);
    
    // Set spacing and margins
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Connect signals to slots
    connect(startButton, &QPushButton::clicked, this, &WakuWidget::onStartButtonClicked);
    connect(stopButton, &QPushButton::clicked, this, &WakuWidget::onStopButtonClicked);
    connect(refreshPeersButton, &QPushButton::clicked, this, &WakuWidget::onRefreshPeersClicked);
    connect(refreshMetricsButton, &QPushButton::clicked, this, &WakuWidget::onRefreshMetricsClicked);
    
    // Create metrics timer for auto-refresh
    metricsTimer = new QTimer(this);
    connect(metricsTimer, &QTimer::timeout, this, &WakuWidget::onMetricsTimer);
}

WakuWidget::~WakuWidget() {
    stopWaku();
    if (metricsTimer) {
        metricsTimer->stop();
    }
}

void WakuWidget::startWaku() {
    updateStatus("Status: Starting Waku...");

    auto& wakuModule = logos->waku_module;

    // TODO: Add connections to various signals from waku module here
    
    // TODO: call start method on waku module
    // bool _start = wakuModule->start();
    //if (!success) {
    //    updateStatus("Error: Failed to start waku module");
    //    return;
    //}

    isWakuRunning = true;
    updateStatus("Status: Waku started");

    // Enable UI components
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    refreshPeersButton->setEnabled(true);
    refreshMetricsButton->setEnabled(true);
}

void WakuWidget::stopWaku() {
    if (!isWakuRunning) {
        return;
    }
    
    updateStatus("Status: Stopping Waku...");
    
    // TODO: Try to stop waku
    // bool stop = wakuModule->stop();
    //if (!success) {
    //    updateStatus("Error: Failed to stop waku module");
    //    return;
    //}
    
    isWakuRunning = false;
    updateStatus("Status: Waku stopped");
    
    // Disable UI components
    startButton->setEnabled(true);
    stopButton->setEnabled(false);
    refreshPeersButton->setEnabled(false);
    refreshMetricsButton->setEnabled(false);
    
    // Clear displays
    peersList->clear();
    metricsDisplay->clear();
}

void WakuWidget::refreshPeers() {
    if (!isWakuRunning) {
        return;
    }
    
    // Try to get peers - method name may vary: getPeers, peers, listPeers
    QVariant result;
    // if (logos && logos->waku_module.getPeers) {
    //     result = logos->waku_module.getPeers();
    // } else if (logos && logos->waku_module.peers) {
    //     result = logos->waku_module.peers();
    // } else if (logos && logos->waku_module.listPeers) {
    //     result = logos->waku_module.listPeers();
    // } else {
        // Try generic invoke
        result = m_logosAPI->getClient("waku_module")->invokeRemoteMethod("waku_module", "getPeers");
    // }
    
    QStringList peers;
    if (result.typeId() == QMetaType::QStringList) {
        peers = result.toStringList();
    } else if (result.typeId() == QMetaType::QVariantList) {
        QVariantList list = result.toList();
        for (const QVariant& item : list) {
            peers.append(item.toString());
        }
    } else if (result.typeId() == QMetaType::QString) {
        // Try to parse as JSON
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(result.toString().toUtf8(), &error);
        if (error.error == QJsonParseError::NoError && doc.isArray()) {
            QJsonArray array = doc.array();
            for (const QJsonValue& value : array) {
                peers.append(value.toString());
            }
        }
    }
    
    updatePeersList(peers);
}

void WakuWidget::refreshMetrics() {
    if (!isWakuRunning) {
        return;
    }
    
    // Try to get metrics - method name may vary: getMetrics, metrics, getStats
    QVariant result;
    // if (logos && logos->waku_module.getMetrics) {
    //     result = logos->waku_module.getMetrics();
    // } else if (logos && logos->waku_module.metrics) {
    //     result = logos->waku_module.metrics();
    // } else if (logos && logos->waku_module.getStats) {
    //     result = logos->waku_module.getStats();
    // } else {
        // Try generic invoke
        result = m_logosAPI->getClient("waku_module")->invokeRemoteMethod("waku_module", "getMetrics");
    // }
    
    QString metricsText;
    if (result.typeId() == QMetaType::QString) {
        metricsText = result.toString();
    } else if (result.typeId() == QMetaType::QVariantMap) {
        QVariantMap map = result.toMap();
        QJsonObject obj = QJsonObject::fromVariantMap(map);
        QJsonDocument doc(obj);
        metricsText = doc.toJson(QJsonDocument::Indented);
    } else {
        metricsText = result.toString();
    }
    
    updateMetrics(metricsText);
}

void WakuWidget::onStartButtonClicked() {
    startWaku();
}

void WakuWidget::onStopButtonClicked() {
    stopWaku();
}

void WakuWidget::onRefreshPeersClicked() {
    refreshPeers();
}

void WakuWidget::onRefreshMetricsClicked() {
    refreshMetrics();
}

void WakuWidget::onMetricsTimer() {
    refreshMetrics();
}

void WakuWidget::updateStatus(const QString& message) {
    statusLabel->setText(message);
    qDebug() << message;
}

void WakuWidget::updatePeersList(const QStringList& peers) {
    peersList->clear();
    if (peers.isEmpty()) {
        peersList->addItem("No peers connected");
    } else {
        for (const QString& peer : peers) {
            peersList->addItem(peer);
        }
    }
    qDebug() << "Updated peers list:" << peers.size() << "peers";
}

void WakuWidget::updateMetrics(const QString& metrics) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    metricsDisplay->setPlainText(QString("Last updated: %1\n\n%2").arg(timestamp, metrics));
    qDebug() << "Updated metrics";
}
