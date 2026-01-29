#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QTimer>
#include "logos_api.h"
#include "logos_api_client.h"
#include "logos_sdk.h"

class WakuBackend : public QObject {
    Q_OBJECT

public:
    enum WakuStatus {
        NotStarted = 0,
        Starting,
        Running,
        Stopping,
        Stopped,
        Error
    };
    Q_ENUM(WakuStatus)

    Q_PROPERTY(WakuStatus status READ status NOTIFY statusChanged)
    Q_PROPERTY(QStringList peersList READ peersList NOTIFY peersListChanged)
    Q_PROPERTY(QString metrics READ metrics NOTIFY metricsChanged)
    Q_PROPERTY(QString peersLastUpdated READ peersLastUpdated NOTIFY peersLastUpdatedChanged)
    Q_PROPERTY(QString metricsLastUpdated READ metricsLastUpdated NOTIFY metricsLastUpdatedChanged)

    explicit WakuBackend(LogosAPI* logosAPI = nullptr, QObject* parent = nullptr);
    ~WakuBackend();

    WakuStatus status() const { return m_status; }
    QStringList peersList() const { return m_peersList; }
    QString metrics() const { return m_metrics; }
    QString peersLastUpdated() const { return m_peersLastUpdated; }
    QString metricsLastUpdated() const { return m_metricsLastUpdated; }

public slots:
    Q_INVOKABLE void startWaku();
    Q_INVOKABLE void stopWaku();
    Q_INVOKABLE void refreshPeers();
    Q_INVOKABLE void refreshMetrics();

signals:
    void statusChanged();
    void peersListChanged();
    void metricsChanged();
    void peersLastUpdatedChanged();
    void metricsLastUpdatedChanged();

private slots:
    void onConnectedPeersResponse(const QVariantList& data);
    void onMetricsResponse(const QVariantList& data);

private:
    void setStatus(WakuStatus newStatus);
    void updatePeersList(const QStringList& peers);
    void updateMetrics(const QString& metrics);
    void setPeersLastUpdated(const QString& timestamp);
    void setMetricsLastUpdated(const QString& timestamp);
    QString formatTimestamp(const QString& isoTimestamp);

    WakuStatus m_status;
    QStringList m_peersList;
    QString m_metrics;
    QString m_peersLastUpdated;
    QString m_metricsLastUpdated;
    
    LogosAPI* m_logosAPI;
    LogosModules* m_logos;
    QTimer* m_metricsTimer;
};
