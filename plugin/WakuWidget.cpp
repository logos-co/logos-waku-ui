#include "WakuWidget.h"
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QMetaType>
#include <QBrush>
#include <QColor>
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
    buttonLayout->addWidget(statusLabel);
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

    QString relayTopic = "/waku/2/rs/16/32";
    // Create appropriate Waku config
    QString configStr = R"({
        "host": "0.0.0.0",
        "tcpPort": 60010,
        "key": null,
        "clusterId": 16,
        "relay": true,
        "relayTopics": [")" + relayTopic + R"("],
        "shards": [1,32,64,128,256],
        "maxMessageSize": "1024KiB",
        "dnsDiscovery": true,
        "dnsDiscoveryUrl": "enrtree://AMOJVZX4V6EXP7NTJPMAYJYST2QP6AJXYW76IU6VGJS7UVSNDYZG4@boot.prod.status.nodes.status.im",
        "discv5Discovery": false,
        "numShardsInNetwork": 257,
        "discv5EnrAutoUpdate": false,
        "logLevel": "INFO",
        "keepAlive": true
    })";

    if (!wakuModule.initWaku(configStr)) {
        updateStatus("Error: Failed to initialize Waku module");
    }    

    // TODO: Add connections to various signals from waku module here

    if (!wakuModule.setEventCallback()) {
        updateStatus("Error: Failed to register Waku event callback");
    }

    // Subscribe to connectedPeersResponse events using the same pattern as ChatWidget
    if (!wakuModule.on("connectedPeersResponse", [this](const QVariantList& data) {
            if (data.size() < 1) {
                qWarning() << "WakuWidget: connectedPeersResponse payload missing fields";
                return;
            }
            onConnectedPeersResponse(data);
        })) {
        qWarning() << "WakuWidget: failed to subscribe to connectedPeersResponse events";
    }

    // Subscribe to metricsResponse events
    if (!wakuModule.on("metricsResponse", [this](const QVariantList& data) {
            if (data.size() < 1) {
                qWarning() << "WakuWidget: metricsResponse payload missing fields";
                return;
            }
            onMetricsResponse(data);
        })) {
        qWarning() << "WakuWidget: failed to subscribe to metricsResponse events";
    }

    if (!wakuModule.startWaku()) {
        updateStatus("Error: Failed to start Waku module");
    }

    isWakuRunning = true;
    updateStatus("Status: Waku started");

    // Enable UI components
    startButton->setEnabled(false);
    stopButton->setEnabled(true);
    refreshPeersButton->setEnabled(true);
    refreshMetricsButton->setEnabled(true);
    
    // Refresh peers and metrics after a delay to allow Waku to fully start
    QTimer::singleShot(1000, this, [this]() {
        onRefreshPeersClicked();
        onRefreshMetricsClicked();
    });
}

void WakuWidget::stopWaku() {
    if (!isWakuRunning) {
        return;
    }
    
    updateStatus("Status: Stopping Waku...");
    
    // Use the same pattern as startWaku() - access via logos->waku_module
    // This should work the same way as startWaku() does
    auto& wakuModule = logos->waku_module;
        
    if (!wakuModule.stopWaku()) {
        qWarning() << "WakuWidget::stopWaku: stopWaku() returned false";
        updateStatus("Error: Failed to stop Waku module");
        return;
    }
        
    isWakuRunning = false;
    updateStatus("Status: Waku stopped");
    
    // Disable UI components
    startButton->setEnabled(true);
    stopButton->setEnabled(false);
    refreshPeersButton->setEnabled(false);
    refreshMetricsButton->setEnabled(false);
    
    // Clear displays
    updatePeersList(QStringList());
    metricsDisplay->clear();
}

void WakuWidget::refreshPeers() {
    if (!isWakuRunning) {
        return;
    }
    
    // Use the new getConnectedPeers method
    auto& wakuModule = logos->waku_module;
    
    if (!wakuModule.getConnectedPeers()) {
        updateStatus("Error: Failed to get connected peers");
        return;
    }
    
    // The peers will be received via the onConnectedPeersResponse event handler
}

void WakuWidget::refreshMetrics() {
    if (!isWakuRunning) {
        return;
    }
    
    // Use the new getMetrics method
    auto& wakuModule = logos->waku_module;
    
    if (!wakuModule.getMetrics()) {
        updateStatus("Error: Failed to get metrics");
        return;
    }
    
    // The metrics will be received via the onMetricsResponse event handler
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

void WakuWidget::updatePeersList(const QStringList& peers, const QString& timestamp) {
    peersList->clear();
    
    // Add timestamp at the top if provided
    if (!timestamp.isEmpty()) {
        QListWidgetItem* timestampItem = new QListWidgetItem(QString("Last updated: %1").arg(timestamp));
        QFont timestampFont = peersList->font();
        timestampFont.setItalic(true);
        timestampItem->setFont(timestampFont);
        timestampItem->setFlags(timestampItem->flags() & ~Qt::ItemIsEnabled); // Make it non-selectable
        peersList->addItem(timestampItem);
        
        // Add a separator line
        QListWidgetItem* separatorItem = new QListWidgetItem("─────────────────────────");
        separatorItem->setFlags(separatorItem->flags() & ~Qt::ItemIsEnabled);
        separatorItem->setForeground(QBrush(QColor(Qt::gray)));
        peersList->addItem(separatorItem);
    }
    
    if (peers.isEmpty()) {
        peersList->addItem("No peers connected");
        peersLabel->setText("Connected Peers: 0");
    } else {
        for (const QString& peer : peers) {
            QListWidgetItem* item = new QListWidgetItem(QString("Peer ID:   %1").arg(peer));
            peersList->addItem(item);
        }
        peersLabel->setText(QString("Connected Peers: %1").arg(peers.size()));
    }
    qDebug() << "Updated peers list:" << peers.size() << "peers";
}

void WakuWidget::updateMetrics(const QString& metrics) {
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    metricsDisplay->setPlainText(QString("Last updated: %1\n\n%2").arg(timestamp, metrics));
    qDebug() << "Updated metrics";
}

void WakuWidget::onConnectedPeersResponse(const QVariantList& data) {
    if (data.isEmpty()) {
        qWarning() << "WakuWidget::onConnectedPeersResponse: Empty data";
        return;
    }
    
    qDebug() << "WakuWidget::onConnectedPeersResponse: Data size:" << data.size();
    qDebug() << "WakuWidget::onConnectedPeersResponse: Data:" << data;
    
    // The first element is a comma-separated string of peer IDs
    QString peersString = data[0].toString();
    
    // The second element is the timestamp
    QString timestamp = data.size() > 1 ? data[1].toString() : QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Parse comma-separated peer IDs
    QStringList peers = peersString.split(',', Qt::SkipEmptyParts);
    for (QString& peer : peers) {
        peer = peer.trimmed();
    }
    
    // Update the peers list with timestamp
    updatePeersList(peers, timestamp);
}

void WakuWidget::onMetricsResponse(const QVariantList& data) {
    if (data.isEmpty()) {
        qWarning() << "WakuWidget::onMetricsResponse: Empty data";
        return;
    }
    
    qDebug() << "WakuWidget::onMetricsResponse: Data size:" << data.size();
    qDebug() << "WakuWidget::onMetricsResponse: Data:" << data;
    
    // The first element is the metrics JSON string
    QString metricsJson = data[0].toString();
    
    // The second element is the timestamp (optional)
    QString timestamp = data.size() > 1 ? data[1].toString() : QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Try to parse and format the JSON if it's valid JSON
    QString metricsText;
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(metricsJson.toUtf8(), &error);
    
    if (error.error == QJsonParseError::NoError && doc.isObject()) {
        // Format as indented JSON
        metricsText = doc.toJson(QJsonDocument::Indented);
    } else {
        // If not valid JSON, use as-is
        metricsText = metricsJson;
    }
    
    // Update metrics display with timestamp from event
    QString formattedMetrics = QString("Last updated: %1\n\n%2").arg(timestamp, metricsText);
    metricsDisplay->setPlainText(formattedMetrics);
    qDebug() << "Updated metrics from event";
}
