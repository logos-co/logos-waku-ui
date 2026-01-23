#include "WakuBackend.h"
#include <QDebug>
#include <QDateTime>
#include <QLocale>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

WakuBackend::WakuBackend(LogosAPI* logosAPI, QObject* parent)
    : QObject(parent),
      m_status(NotStarted),
      m_logosAPI(nullptr),
      m_logos(nullptr),
      m_metricsTimer(nullptr),
      m_peersLastUpdated(""),
      m_metricsLastUpdated("")
{
    qDebug() << "Initializing WakuBackend...";
    
    if (logosAPI) {
        m_logosAPI = logosAPI;
    } else {
        m_logosAPI = new LogosAPI("core", this);
    }
    
    m_logos = new LogosModules(m_logosAPI);
    
    // Create metrics timer for auto-refresh
    m_metricsTimer = new QTimer(this);
    connect(m_metricsTimer, &QTimer::timeout, this, &WakuBackend::refreshMetrics);
}

WakuBackend::~WakuBackend()
{
    stopWaku();
    if (m_metricsTimer) {
        m_metricsTimer->stop();
    }
}

void WakuBackend::setStatus(WakuStatus newStatus)
{
    if (m_status != newStatus) {
        m_status = newStatus;
        emit statusChanged();
        qDebug() << "WakuBackend: Status changed to" << m_status;
        
        // Reset last updated times when stopped or not started
        if (newStatus == NotStarted || newStatus == Stopped) {
            setPeersLastUpdated("");
            setMetricsLastUpdated("");
        }
    }
}

void WakuBackend::startWaku()
{
    setStatus(Starting);

    auto& wakuModule = m_logos->waku_module;

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
        setStatus(Error);
        return;
    }

    if (!wakuModule.setEventCallback()) {
        setStatus(Error);
        return;
    }

    // Subscribe to connectedPeersResponse events
    if (!wakuModule.on("connectedPeersResponse", [this](const QVariantList& data) {
            if (data.size() < 1) {
                qWarning() << "WakuBackend: connectedPeersResponse payload missing fields";
                return;
            }
            onConnectedPeersResponse(data);
        })) {
        qWarning() << "WakuBackend: failed to subscribe to connectedPeersResponse events";
    }

    // Subscribe to metricsResponse events
    if (!wakuModule.on("metricsResponse", [this](const QVariantList& data) {
            if (data.size() < 1) {
                qWarning() << "WakuBackend: metricsResponse payload missing fields";
                return;
            }
            onMetricsResponse(data);
        })) {
        qWarning() << "WakuBackend: failed to subscribe to metricsResponse events";
    }

    if (!wakuModule.startWaku()) {
        setStatus(Error);
        return;
    }

    setStatus(Running);

    // Refresh peers and metrics after a delay to allow Waku to fully start
    QTimer::singleShot(1000, this, [this]() {
        refreshPeers();
        refreshMetrics();
    });
}

void WakuBackend::stopWaku()
{
    if (m_status != Running && m_status != Starting) {
        return;
    }

    setStatus(Stopping);

    auto& wakuModule = m_logos->waku_module;

    if (!wakuModule.stopWaku()) {
        qWarning() << "WakuBackend::stopWaku: stopWaku() returned false";
        setStatus(Error);
        return;
    }

    setStatus(Stopped);

    // Clear displays
    updatePeersList(QStringList());
    updateMetrics("");
    setPeersLastUpdated("");
    setMetricsLastUpdated("");
}

void WakuBackend::refreshPeers()
{
    if (m_status != Running) {
        return;
    }

    auto& wakuModule = m_logos->waku_module;

    if (!wakuModule.getConnectedPeers()) {
        qWarning() << "WakuBackend: Failed to get connected peers";
        return;
    }

    // The peers will be received via the onConnectedPeersResponse event handler
}

void WakuBackend::refreshMetrics()
{
    if (m_status != Running) {
        return;
    }

    auto& wakuModule = m_logos->waku_module;

    if (!wakuModule.getMetrics()) {
        qWarning() << "WakuBackend: Failed to get metrics";
        return;
    }

    // The metrics will be received via the onMetricsResponse event handler
}

void WakuBackend::updatePeersList(const QStringList& peers)
{
    if (m_peersList != peers) {
        m_peersList = peers;
        emit peersListChanged();
        qDebug() << "WakuBackend: Updated peers list:" << peers.size() << "peers";
    }
}

void WakuBackend::updateMetrics(const QString& metrics)
{
    if (m_metrics != metrics) {
        m_metrics = metrics;
        emit metricsChanged();
        qDebug() << "WakuBackend: Updated metrics";
    }
}

void WakuBackend::setPeersLastUpdated(const QString& timestamp)
{
    if (m_peersLastUpdated != timestamp) {
        m_peersLastUpdated = timestamp;
        emit peersLastUpdatedChanged();
    }
}

void WakuBackend::setMetricsLastUpdated(const QString& timestamp)
{
    if (m_metricsLastUpdated != timestamp) {
        m_metricsLastUpdated = timestamp;
        emit metricsLastUpdatedChanged();
    }
}

QString WakuBackend::formatTimestamp(const QString& isoTimestamp)
{
    if (isoTimestamp.isEmpty()) {
        return "";
    }
    
    // Parse ISO 8601 format: "2026-01-23T19:53:46" or "2026-01-23T19:53:46.123Z"
    QDateTime dateTime = QDateTime::fromString(isoTimestamp, Qt::ISODate);
    if (!dateTime.isValid()) {
        // Try alternative format
        dateTime = QDateTime::fromString(isoTimestamp, "yyyy-MM-ddTHH:mm:ss");
    }
    
    if (!dateTime.isValid()) {
        // If parsing fails, return original
        return isoTimestamp;
    }
    
    // Format as "Jan 23, 2026 19:53:46"
    return dateTime.toString("MMM d, yyyy HH:mm:ss");
}

void WakuBackend::onConnectedPeersResponse(const QVariantList& data)
{
    if (data.isEmpty()) {
        qWarning() << "WakuBackend::onConnectedPeersResponse: Empty data";
        return;
    }

    qDebug() << "WakuBackend::onConnectedPeersResponse: Data size:" << data.size();
    qDebug() << "WakuBackend::onConnectedPeersResponse: Data:" << data;

    // The first element is a comma-separated string of peer IDs
    QString peersString = data[0].toString();

    // Parse comma-separated peer IDs
    QStringList peers = peersString.split(',', Qt::SkipEmptyParts);
    for (QString& peer : peers) {
        peer = peer.trimmed();
    }

    // Update the peers list
    updatePeersList(peers);
    
    // Update timestamp if available
    if (data.size() >= 2) {
        QString isoTimestamp = data[1].toString();
        QString formattedTimestamp = formatTimestamp(isoTimestamp);
        setPeersLastUpdated(formattedTimestamp);
    }
}

void WakuBackend::onMetricsResponse(const QVariantList& data)
{
    if (data.isEmpty()) {
        qWarning() << "WakuBackend::onMetricsResponse: Empty data";
        return;
    }

    qDebug() << "WakuBackend::onMetricsResponse: Data size:" << data.size();
    qDebug() << "WakuBackend::onMetricsResponse: Data:" << data;

    // The first element is the metrics JSON string
    QString metricsJson = data[0].toString();

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

    // Update metrics
    updateMetrics(metricsText);
    
    // Update timestamp if available
    if (data.size() >= 2) {
        QString isoTimestamp = data[1].toString();
        QString formattedTimestamp = formatTimestamp(isoTimestamp);
        setMetricsLastUpdated(formattedTimestamp);
    }
}
